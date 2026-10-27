#include "PoolAllocator.h"
#include <assert.h>
#include <stdint.h>
#include <stddef.h>     /* max_align_t */
#include <stdlib.h>     /* malloc, free */
#include <algorithm>    //max
// D-35: _DEBUG 只由 MSVC 定义，GCC/Clang 下恒不成立，调试输出永远不出现。
// 但同时也不能直接绑到 NDEBUG —— 那会把"保留断言"和"打印每次分配"绑死，
// 一开断言就刷屏。改为独立开关 ALLOCATOR_TRACE。
#ifdef ALLOCATOR_TRACE
#include <iostream>
#endif

PoolAllocator::PoolAllocator(const std::size_t totalSize, const std::size_t chunkSize)
: Allocator(totalSize) {
    // D-33: 原实现只要求 chunkSize >= 8。池内第 k 个 chunk 的地址是
    // start + k * chunkSize，若 chunkSize 不是 max_align_t(16) 的整数倍，
    // 每隔一个 chunk 就落在 8 字节边界上，无法承载 double/long double 等
    // 需要 16 字节对齐的类型。向上取整到 max_align_t 的整数倍。
    const std::size_t align = alignof(std::max_align_t);
    assert(chunkSize >= align && "Chunk size must be at least alignof(max_align_t)");
    this->m_chunkSize = ((chunkSize + align - 1) / align) * align;
    // 说明：chunkSize 被向上取整后，totalSize 不再是 chunkSize 的整数倍时，
    // 尾部不足一个 chunk 的空间不参与分配（由 m_totalSize / m_chunkSize 决定块数）。
    // 因此原先 "totalSize % chunkSize == 0" 的断言改为不阻断的约束。
}

void PoolAllocator::Init() {
    // D-17: 重复调用 Init() 时原实现直接覆盖 m_start_ptr，泄漏上一块内存。
    if (m_start_ptr != nullptr) {
        free(m_start_ptr);
        m_start_ptr = nullptr;
    }
    m_start_ptr = malloc(m_totalSize);
    if (m_start_ptr == nullptr) {
        m_totalSize = 0;
        m_chunkCount = 0;
        m_allocBitmap.clear();
        return;
    }
    this->Reset();
}

void PoolAllocator::BuildBitmap() {
    // D-46: 位图按 chunk 数量分配，每 chunk 1 bit
    m_chunkCount = m_chunkSize ? (m_totalSize / m_chunkSize) : 0;
    m_allocBitmap.assign((m_chunkCount + 7) / 8, 0);
}

std::size_t PoolAllocator::LiveCount() const {
    std::size_t n = 0;
    for (std::size_t i = 0; i < m_chunkCount; ++i) {
        if (BitIsSet(i)) ++n;
    }
    return n;
}

PoolAllocator::~PoolAllocator() {
    free(m_start_ptr);
    m_start_ptr = nullptr;
}

void *PoolAllocator::Allocate(const std::size_t allocationSize, const std::size_t alignment) {
    (void)alignment;   // 池分配器的对齐能力由 chunkSize 决定，不支持逐次指定

    // D-43: 原实现只靠 assert 校验尺寸，NDEBUG 下整段消失。
    // 实测请求 16 字节而 chunk 为 64 时，会静默返回一个 64 字节的块 ——
    // 调用方以为自己只占了 16 字节，实际占 64，且没有任何反馈。
    // 池的契约是"每次分配恰好一个 chunk"，不满足就该失败而不是凑合。
    if (allocationSize != this->m_chunkSize) return nullptr;

    if (m_start_ptr == nullptr) return nullptr;
    Node * freePosition = m_freeList.pop();

    // 区分两种来源：空闲链表回收的 chunk，还是从高水位游标新切的 chunk。
    // 这个区别决定了下面校验失败时该怎么收场 —— 从游标切出来的那一格
    // 还没有被任何结构记录，一旦跳过就永久丢失；从空闲链表弹出的节点
    // 则已经脱离链表，只能丢弃（推回去等于重新引入重复节点）。
    bool fromOffset = false;
    if (freePosition == nullptr) {
        // 与 D-04 同类问题：原实现 assert 后继续计算地址，Release 构建下会越出池尾。
        if (m_offset >= m_chunkCount) return nullptr;
        std::size_t address = (std::size_t) m_start_ptr + m_offset * m_chunkSize;
        freePosition = (Node *) address;
        fromOffset = true;
    }

    // D-46: 置位并校验。位图里该 chunk 若已是"占用"，说明内部状态已不一致
    // （同一 chunk 进了空闲链表两次）。此时不能再发出去，否则就是重叠派发。
    const std::size_t idx =
        ((std::size_t) freePosition - (std::size_t) m_start_ptr) / m_chunkSize;
    if (idx >= m_chunkCount || BitIsSet(idx)) {
        // 注意：这里刻意不递增 m_offset。原实现先 ++ 再校验，校验失败时
        // 游标已经越过那一格，该 chunk 既不在空闲链表也不在游标可达范围，
        // 等于被永久吞掉。现在从游标分支失败时不推进，容量不受损。
        ++m_inconsistentPops;
        return nullptr;
    }
    if (fromOffset) ++m_offset;
    BitSet(idx);

    m_used += m_chunkSize;
    m_peak = std::max(m_peak, m_used);
#ifdef ALLOCATOR_TRACE
    std::cout << "A" << "\t@S " << m_start_ptr << "\t@R " << (void*) freePosition << "\tM " << m_used << std::endl;
#endif

    return (void*) freePosition;
}

void PoolAllocator::Free(void * ptr) {
    // D-37: 原实现无条件把任意指针压入空闲链表并递减 m_used。
    // 传入越界指针或重复释放都会让链表指向池外，下次 Allocate 返回野地址。
    if (ptr == nullptr || m_start_ptr == nullptr) { ++m_rejectedFrees; return; }

    const std::size_t p     = (std::size_t) ptr;
    const std::size_t start = (std::size_t) m_start_ptr;
    if (p < start || p >= start + m_totalSize) { ++m_rejectedFrees; return; }
    if ((p - start) % m_chunkSize != 0)        { ++m_rejectedFrees; return; }

    const std::size_t idx = (p - start) / m_chunkSize;
    if (idx >= m_chunkCount) { ++m_rejectedFrees; return; }

    // D-46: 精确的重复释放检测。
    // 原先用 "m_used < m_chunkSize" 兜底，只在池里仅剩一个活动块时成立：
    // 分配 a、b 后 Free(a) 两次，两次都能通过，a 被压入空闲链表两次，
    // 随后两次 Allocate 都返回 a —— 同一地址派发两次。
    if (!BitIsSet(idx)) { ++m_rejectedFrees; return; }
    BitClear(idx);

    m_used = (m_used >= m_chunkSize) ? (m_used - m_chunkSize) : 0;

    m_freeList.push((Node *) ptr);

#ifdef ALLOCATOR_TRACE
    std::cout << "F" << "\t@S " << m_start_ptr << "\t@F " << ptr << "\tM " << m_used << std::endl;
#endif
}

void PoolAllocator::Reset() {
    m_used = 0;
    m_peak = 0;
    m_offset = 0;
    m_rejectedFrees = 0;
    m_inconsistentPops = 0;
    // D-02: 原实现未清空空闲链表。链表节点本身存放在池内存里，
    // Reset 后 m_offset 归零，下一次 Allocate 会从池首重新派发，
    // 而旧的 head 仍指向同一批地址 —— 同一 chunk 被派发两次。
    m_freeList.head = nullptr;
    // D-46: 位图同步归零
    BuildBitmap();
}
