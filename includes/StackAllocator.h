#ifndef STACKALLOCATOR_H
#define STACKALLOCATOR_H

#include "Allocator.h"
#include <vector>

class StackAllocator : public Allocator {
protected:
    void* m_start_ptr = nullptr;
    std::size_t m_offset = 0;
    // 每次 Allocate 记录分配前的偏移，Free 时回退到这里。
    // 注意：这是分配器自身的堆外存储 —— 见 Reserve() 的说明。
    std::vector<std::size_t> m_markers;
    std::vector<std::size_t> m_checkpoints;
public:
    StackAllocator(const std::size_t totalSize);

    virtual ~StackAllocator();

    virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;

    // 说明：size == 0 返回 nullptr（D-47）。零字节没有可交付的地址，
    // 若返回指针则它落在空区间上，永远无法通过 Free 的区间校验。

    // D-42: 乱序 / 重复 / 越界释放会被拒绝，状态保持不变。
    // 原实现把校验放在 assert 里，NDEBUG 构建下整段消失，实测会出现
    // 同一地址被派发两次。现改为无条件校验，Release 下同样生效。
    virtual void Free(void* ptr) override;

    // 带反馈的版本：返回 false 表示该指针不是当前栈顶的分配，未做任何改动。
    // 需要区分"释放成功"与"被拒绝"时使用它，而不是 Free()。
    //
    // 语义说明：栈分配器没有逐块元数据，无法把指针精确比对到某次分配。
    // Free 的实际动作是"回退到栈顶那次分配之前"，ptr 只用于校验它是否落在
    // 栈顶块所占区间内。因此传入栈顶块内部的地址（如 a + 8）会被接受，
    // 效果等同于释放 a。这是 LIFO 语义的必然结果，不是缺陷 —— 要精确校验
    // 就必须为每块存元数据，那会让栈分配器退化成 free list。
    bool FreeChecked(void* ptr);

    virtual void Init() override;

    virtual void Reset();
    std::size_t Push();
    void Pop(const std::size_t marker);

    // 当前栈深度（尚未释放的分配次数）
    std::size_t Depth() const { return m_markers.size(); }

    // 被拒绝的 Free 次数。用于在不改变 void 接口的前提下观察误用。
    std::size_t RejectedFrees() const { return m_rejectedFrees; }

    // 预分配标记数组的容量。
    // 说明：本分配器用 std::vector 记录每次分配的偏移，push_back 在容量
    // 不足时会调用 malloc —— 这与"自定义分配器避免运行时 malloc"的初衷
    // 相悖。预留足够容量可以把这部分开销从热路径上移走。
    void Reserve(const std::size_t n);
private:
    // D-34: 用 "= delete" 取代"私有声明但不定义"
    StackAllocator(StackAllocator &stackAllocator) = delete;
    StackAllocator& operator=(StackAllocator &stackAllocator) = delete;

    std::size_t m_rejectedFrees = 0;
};

#endif /* STACKALLOCATOR_H */
