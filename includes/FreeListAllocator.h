#ifndef FREELISTALLOCATOR_H
#define FREELISTALLOCATOR_H

#include "Allocator.h"
#include "SinglyLinkedList.h"
#include <cstdint>   /* std::uint32_t */

#ifdef ALLOCATOR_VALIDATE
#include <string>
#include <vector>
#endif

class FreeListAllocator : public Allocator {
public:
    enum PlacementPolicy {
        FIND_FIRST,
        FIND_BEST
    };

private:
    struct FreeHeader {
        std::size_t blockSize;
    };

    // D-45: 块头从 {size_t, size_t} 改为 {size_t, uint32, uint32}，
    // sizeof 仍是 16，内存几何不变，但多出一个 magic 字段用于识别重复释放。
    //
    // 为什么必须加：原实现在 Free 里直接读 headerAddress 上的 padding 反推
    // 块起点。第一次 Free 之后，这块内存已被空闲节点的 {blockSize, next}
    // 覆盖，第二次 Free 读到的是垃圾 padding，算出的块起点落在池外，
    // 随后写入直接段错误（实测 SIGSEGV）。
    //
    // 为什么 padding 能缩成 uint32：它只是对齐填充，上界是 alignment，
    // 而 Allocate 已拒绝 align > m_totalSize 的请求；超过 4 GiB 的对齐
    // 在实践中不存在，真遇到时 Allocate 会返回 nullptr（见实现）。
    struct AllocationHeader {
        std::size_t blockSize;
        std::uint32_t padding;
        std::uint32_t magic;     // 见 kAllocatedMagic
    };

    static const std::uint32_t kAllocatedMagic = 0xA110CA7Eu;

    typedef SinglyLinkedList<FreeHeader>::Node Node;

    
    void* m_start_ptr = nullptr;
    PlacementPolicy m_pPolicy;
    SinglyLinkedList<FreeHeader> m_freeList;

    // D-51: 空闲块大小的**安全上界**（永远 >= 真实最大值，绝不低估）。
    //
    // 用途：Allocate 在进入 Find 之前先做一次 O(1) 判断 —— 若
    // "size + 最小块头" 都超过这个上界，那么任何块都装不下，直接失败。
    // 修复前这种情况要扫完整条空闲链表才能确认，池满后的每一次 Allocate
    // 都是 O(n)。基准里分配阶段一旦触底，剩下的全是这种注定失败的调用。
    //
    // 为什么是"上界"而不是精确最大值：精确最大值在消耗掉最大块时会下降，
    // 维护它需要在每次分配后重算。这里取保守策略 —— 只保证不低估，
    // 低估会导致错误拒绝，那是正确性问题；高估只是少一次快速失败，无害。
    // 真正会失效的时机（消耗的正好是当前上界对应的块）才重扫一次。
    std::size_t m_maxFreeBlock = 0;

    void RecomputeMaxFreeBlock();

    // padding 至少等于块头大小（CalculatePaddingWithHeader 的返回值下界），
    // 所以 requiredSize >= size + sizeof(AllocationHeader)。用减法比较避免回绕。
    bool FastReject(const std::size_t size) const {
        if (size > m_maxFreeBlock) return true;
        return sizeof(AllocationHeader) > m_maxFreeBlock - size;
    }

public:
    FreeListAllocator(const std::size_t totalSize, const PlacementPolicy pPolicy);

    virtual ~FreeListAllocator();

    virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;

    virtual void Free(void* ptr) override;

    virtual void Init() override;

    virtual void Reset();

    // 被拒绝的 Free 次数（重复释放、越界指针、块头校验失败）。
    // 接口是 void，无法返回错误，用这个观察误用。
    std::size_t RejectedFrees() const { return m_rejectedFrees; }

private:
    // D-34: 用 "= delete" 取代"私有声明但不定义"，语义明确且能阻止隐式拷贝
    FreeListAllocator(FreeListAllocator &freeListAllocator) = delete;
    FreeListAllocator& operator=(FreeListAllocator &freeListAllocator) = delete;

    // 返回合并后仍留在链表中的那个节点（可能已被后一个节点吞并）。
    Node* Coalescence(Node* prevBlock, Node * freeBlock);

    void Find(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode);
    void FindBest(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode);
    void FindFirst(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node*& previousNode, Node*& foundNode);

    std::size_t m_rejectedFrees = 0;

#ifdef ALLOCATOR_VALIDATE
public:
    // 仅调试构建启用：校验空闲链表地址序、区间不重叠、相邻块已合并、
    // m_used 与活动块总和一致。供 tests/ 下的不变量校验使用。
    bool Validate(const std::vector<void*>& livePtrs, std::string* err) const;
    std::size_t FreeBlockCount() const;
    std::size_t LargestFreeBlock() const;

    // 仅测试用：把最大空闲块上界抬到 SIZE_MAX，等效于关闭 D-51 的快速失败。
    // 用途：tests/measure_findbest.cpp 在同一分配器上做差分 —— 同一请求序列
    // 先开快速失败跑一遍、再关掉跑一遍，两次结果必须逐请求一致。这是验证
    // "快速失败绝不误拒"最干净的办法：跨分配器比较是错的，两个池的起始
    // 地址不同，对齐填充就不同，成败本来就可能不一样。
    void DisableFastRejectForTest() { m_maxFreeBlock = (std::size_t)-1; }
#endif
};

#endif /* FREELISTALLOCATOR_H */
