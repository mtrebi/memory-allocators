#include "LinearAllocator.h"
#include "Utils.h"  /* CalculatePadding */
#include <stdlib.h>     /* malloc, free */
#include <cassert>   /*assert		*/
#include <algorithm>    // max
// D-35: _DEBUG 只由 MSVC 定义，GCC/Clang 下恒不成立，调试输出永远不出现。
// 但同时也不能直接绑到 NDEBUG —— 那会把"保留断言"和"打印每次分配"绑死，
// 一开断言就刷屏。改为独立开关 ALLOCATOR_TRACE。
#ifdef ALLOCATOR_TRACE
#include <iostream>
#endif

LinearAllocator::LinearAllocator(const std::size_t totalSize)
: Allocator(totalSize) {
}

void LinearAllocator::Init() {
    if (m_start_ptr != nullptr) {
        free(m_start_ptr);
        m_start_ptr = nullptr;
    }
    m_start_ptr = malloc(m_totalSize);
    m_offset = 0;
    // D-16: 原实现不重置 m_used/m_peak，重复 Init() 后峰值统计混入上一轮数据。
    m_used = 0;
    m_peak = 0;
}

LinearAllocator::~LinearAllocator() {
    free(m_start_ptr);
    m_start_ptr = nullptr;
}

void* LinearAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
    std::size_t padding = 0;
    const std::size_t currentAddress = (std::size_t)m_start_ptr + m_offset;

    if (m_start_ptr == nullptr) return nullptr;

    if (alignment != 0) {
        // D-05: 原实现用 "m_offset % alignment" 判定是否需要对齐。
        // m_offset 是相对池首的偏移，而 malloc 返回的池首地址本身不保证
        // 是 alignment 的整数倍，因此偏移对齐不等于地址对齐 ——
        // 只要池首地址不是 alignment 倍数，返回的地址就永远不可能对齐。
        // 判定必须基于绝对地址。
        padding = Utils::CalculatePadding(currentAddress, alignment);
    }

    // D-32: 原写法 "m_offset + padding + size > m_totalSize" 中，
    // padding 与 size 均可来自调用方，二者相加可能整数回绕，
    // 回绕后的极小值能绕过检查。改用减法形式，两侧都不会溢出。
    if (padding > m_totalSize - m_offset ||
        size > m_totalSize - m_offset - padding) {
        return nullptr;
    }

    m_offset += padding;
    const std::size_t nextAddress = currentAddress + padding;
    m_offset += size;

#ifdef ALLOCATOR_TRACE
    std::cout << "A" << "\t@C " << (void*) currentAddress << "\t@R " << (void*) nextAddress << "\tO " << m_offset << "\tP " << padding << std::endl;
#endif

    m_used = m_offset;
    m_peak = std::max(m_peak, m_used);

    return (void*) nextAddress;
}

void LinearAllocator::Free(void* ptr) {
    (void)ptr;
    // 线性分配器不支持单独释放（D-31 的 IsFreeable() 返回 false），
    // 保留断言提示误用；Release 构建下退化为无操作。
    assert(false && "Use Reset() method");
}

void LinearAllocator::Reset() {
    m_offset = 0;
    m_used = 0;
    m_peak = 0;
}
