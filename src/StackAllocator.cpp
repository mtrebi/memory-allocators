#include "StackAllocator.h"
#include "Utils.h"  /* CalculatePadding */
#include <stdlib.h>     /* malloc, free */
#include <algorithm>    /* max */
#include <cassert>
// D-35: _DEBUG 只由 MSVC 定义，GCC/Clang 下恒不成立，调试输出永远不出现。
// 但同时也不能直接绑到 NDEBUG —— 那会把"保留断言"和"打印每次分配"绑死，
// 一开断言就刷屏。改为独立开关 ALLOCATOR_TRACE。
#ifdef ALLOCATOR_TRACE
#include <iostream>
#endif

// 标记数组的默认预留容量。避免头几十次分配就触发 vector 扩容。
static const std::size_t kDefaultMarkerReserve = 256;

StackAllocator::StackAllocator(const std::size_t totalSize)
: Allocator(totalSize) {
    m_markers.reserve(kDefaultMarkerReserve);
    m_checkpoints.reserve(16);
}

void StackAllocator::Init() {
    if (m_start_ptr != nullptr) {
        free(m_start_ptr);
        m_start_ptr = nullptr;
    }
    m_start_ptr = malloc(m_totalSize);
    if (m_start_ptr == nullptr) {
        m_totalSize = 0;
        return;
    }
    Reset();
}

StackAllocator::~StackAllocator() {
    free(m_start_ptr);
    m_start_ptr = nullptr;
}

void StackAllocator::Reserve(const std::size_t n) {
    m_markers.reserve(n);
}

void* StackAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
    if (m_start_ptr == nullptr) return nullptr;

    // D-47: 零字节请求必须拒绝，不能返回一个"看着成功"的指针。
    // 原实现在 size == 0 时 m_offset 不前进，但标记仍被压栈，返回的地址
    // 恰好等于栈顶偏移。FreeChecked 的合法区间是 [lower, upper)，此时
    // lower == upper，区间为空 —— 任何指针都判越界，这块"分配"永远无法
    // 释放。实测 Depth() 卡在 1 不再归零，后续所有 Free 的下界都错位。
    // 零字节没有可交付的地址，按 D-43 的同一原则直接失败。
    if (size == 0) return nullptr;

    const std::size_t currentAddress = (std::size_t)m_start_ptr + m_offset;

    std::size_t padding = 0;
    if (alignment != 0) {
        padding = Utils::CalculatePadding(currentAddress, alignment);
    }

    // D-32: 原写法 "m_offset + padding + size > m_totalSize" 可能整数回绕，
    // 回绕后的极小值绕过检查。改用减法形式。
    if (padding > m_totalSize - m_offset ||
        size > m_totalSize - m_offset - padding) {
        return nullptr;
    }

    m_markers.push_back(m_offset);
    const std::size_t nextAddress = currentAddress + padding;
    m_offset += padding + size;

#ifdef ALLOCATOR_TRACE
    std::cout << "A" << "\t@C " << (void*) currentAddress << "\t@R " << (void*) nextAddress << "\tO " << m_offset << "\tP " << padding << std::endl;
#endif
    m_used = m_offset;
    m_peak = std::max(m_peak, m_used);

    return (void*) nextAddress;
}

bool StackAllocator::FreeChecked(void* ptr) {
    if (ptr == nullptr || m_start_ptr == nullptr) {
        ++m_rejectedFrees;
        return false;
    }
    if (m_markers.empty()) {
        // 栈已空，没有任何可回退的分配
        ++m_rejectedFrees;
        return false;
    }

    // D-42: 栈分配器只允许按 LIFO 顺序释放。合法指针必须落在
    // 栈顶那次分配所占的地址区间 [起始偏移, 当前偏移) 内。
    //   - 释放更早的块：ptr < start + markers.back()，被拒
    //   - 重复释放同一块：第一次已把 m_offset 回退，ptr >= start + m_offset，被拒
    //   - 释放外部指针：通常同时违反上下界，被拒
    const std::size_t p     = (std::size_t) ptr;
    const std::size_t lower = (std::size_t) m_start_ptr + m_markers.back();
    const std::size_t upper = (std::size_t) m_start_ptr + m_offset;

    if (p < lower || p >= upper) {
        ++m_rejectedFrees;
        return false;
    }

    m_offset = m_markers.back();
    m_markers.pop_back();
    m_used = m_offset;

#ifdef ALLOCATOR_TRACE
    std::cout << "F" << "\t@F " << ptr << "\tO " << m_offset << std::endl;
#endif
    return true;
}

void StackAllocator::Free(void* ptr) {
    // 返回值被丢弃是刻意的：接口是 void，误用时状态保持不变（安全失败），
    // 需要感知失败请用 FreeChecked() 或查 RejectedFrees()。
    (void) FreeChecked(ptr);
}

void StackAllocator::Reset() {
    m_offset = 0;
    m_used = 0;
    m_peak = 0;
    m_markers.clear();
    m_checkpoints.clear();
    m_rejectedFrees = 0;
}

std::size_t StackAllocator::Push() {
    m_checkpoints.push_back(m_offset);
    return m_offset;
}

void StackAllocator::Pop(const std::size_t marker) {
    assert(marker <= m_offset && "StackAllocator::Pop marker must be <= current offset");
    if (marker > m_offset) return;   // Release 下同样拒绝非法 marker

    while (!m_checkpoints.empty() && m_checkpoints.back() >= marker) {
        m_checkpoints.pop_back();
    }

    // 回退到 marker 意味着丢弃其后的所有分配，标记数组必须同步收缩，
    // 否则这些标记会残留在数组里，导致后续 Free 校验读到过期的下界。
    while (!m_markers.empty() && m_markers.back() >= marker) {
        m_markers.pop_back();
    }

    m_offset = marker;
    m_used = m_offset;
}
