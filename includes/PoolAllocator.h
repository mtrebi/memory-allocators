#ifndef POOLALLOCATOR_H
#define POOLALLOCATOR_H

#include "Allocator.h"
#include "StackLinkedList.h"
#include <vector>

// D-20: 原文件没有 include guard，被重复包含时 FreeHeader 重定义报错。

class PoolAllocator : public Allocator {
private:
    struct  FreeHeader{
    };
    using Node = StackLinkedList<FreeHeader>::Node;
    StackLinkedList<FreeHeader> m_freeList;

    void * m_start_ptr = nullptr;
    std::size_t m_chunkSize;
    std::size_t m_offset = 0;

    // D-46: 每个 chunk 一 bit 的占用位图，用于精确识别重复释放。
    //
    // 为什么必须加：原先用 "m_used < m_chunkSize" 判断重复释放，只在池中
    // 仅剩一个活动块时成立。实测分配 a、b 后 Free(a) 两次，两次都能通过
    // 检查，a 被压入空闲链表两次，随后的两次 Allocate 都返回 a —— 同一
    // 地址被派发两次，且调用方毫不知情。
    //
    // 为什么用位图而不是"遍历空闲链表查找"：池分配器的价值就在 O(1)，
    // 遍历会把 Free 变成 O(空闲块数)。位图代价是 chunkCount/8 字节，
    // 对 16 MiB / 4096 字节的池是 512 字节。
    std::vector<unsigned char> m_allocBitmap;
    std::size_t m_chunkCount = 0;

    bool BitIsSet(std::size_t idx) const {
        return (m_allocBitmap[idx >> 3] >> (idx & 7u)) & 1u;
    }
    void BitSet(std::size_t idx) {
        m_allocBitmap[idx >> 3] = (unsigned char)(m_allocBitmap[idx >> 3] | (1u << (idx & 7u)));
    }
    void BitClear(std::size_t idx) {
        m_allocBitmap[idx >> 3] = (unsigned char)(m_allocBitmap[idx >> 3] & ~(1u << (idx & 7u)));
    }

public:
    PoolAllocator(const std::size_t totalSize, const std::size_t chunkSize);

    virtual ~PoolAllocator();

    virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;

    virtual void Free(void* ptr) override;

    virtual void Init() override;

    virtual void Reset();

    // 实际生效的 chunk 大小。构造时会向上对齐到 alignof(std::max_align_t)，
    // 与调用方传入的值可能不同，故需要显式查询。
    std::size_t ChunkSize() const { return m_chunkSize; }
    std::size_t ChunkCount() const { return m_chunkSize ? m_totalSize / m_chunkSize : 0; }

    // 被拒绝的 Free 次数（重复释放、越界指针、未对齐指针）。
    std::size_t RejectedFrees() const { return m_rejectedFrees; }

    // 内部状态不一致导致的"拒发"次数（位图说该 chunk 已占用，却仍从空闲
    // 链表弹出）。正常运行时恒为 0；一旦非 0 说明链表或位图被外部踩坏。
    // 单独计数而不并进 RejectedFrees，是因为它发生在 Allocate 而非 Free，
    // 混在一起会让"误用统计"和"状态损坏"两件事无法区分。
    std::size_t InconsistentPops() const { return m_inconsistentPops; }

    // 当前活动 chunk 数（由位图统计，可用于交叉验证 Used()）
    std::size_t LiveCount() const;

private:
    // D-34: 用 "= delete" 取代"私有声明但不定义"
    PoolAllocator(PoolAllocator &poolAllocator) = delete;
    PoolAllocator& operator=(PoolAllocator &poolAllocator) = delete;

    std::size_t m_rejectedFrees = 0;
    std::size_t m_inconsistentPops = 0;

    void BuildBitmap();
};

#endif /* POOLALLOCATOR_H */
