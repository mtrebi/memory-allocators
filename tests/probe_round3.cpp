// 第三轮探针：算术溢出、类型契约、拷贝语义
// 用法：probe_round3 [overflow|overflow_fl|poolalign|doublefree|copyassign|all]
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>
#include <type_traits>
#include <cstring>

#include "LinearAllocator.h"
#include "StackAllocator.h"
#include "FreeListAllocator.h"
#include "PoolAllocator.h"
#include "CAllocator.h"

static int g_fail = 0;

static void verdict(const char* id, bool ok, const char* what) {
    std::printf("%s %-12s %s\n", ok ? "[ OK ]" : "[FAIL]", id, what);
    if (!ok) ++g_fail;
}

// D-32：size + padding 溢出，边界检查失效
static void CaseOverflow() {
    std::printf("--- D-32 整数溢出使边界检查失效 ---\n");
    LinearAllocator lin(1024);
    lin.Init();
    const std::size_t huge = std::numeric_limits<std::size_t>::max();
    void* p1 = lin.Allocate(huge, 8);
    std::printf("     LinearAllocator(1024).Allocate(SIZE_MAX, 8) = %p（期望 nullptr）\n", p1);
    verdict("D-32a", p1 == nullptr, "LinearAllocator 拒绝超大请求");

    StackAllocator st(1024);
    st.Init();
    void* p2 = st.Allocate(huge, 8);
    std::printf("     StackAllocator(1024).Allocate(SIZE_MAX, 8)  = %p（期望 nullptr）\n", p2);
    verdict("D-32b", p2 == nullptr, "StackAllocator 拒绝超大请求");
    std::fflush(stdout);
}

// D-32c：FreeListAllocator 中 size + padding 溢出
static void CaseOverflowFreeList() {
    std::printf("--- D-32c FreeListAllocator 溢出 ---\n");
    FreeListAllocator fl(4096, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    fl.Init();
    const std::size_t huge = std::numeric_limits<std::size_t>::max() - 8;
    std::printf("     调用 Allocate(SIZE_MAX-8, 8)\n");
    std::fflush(stdout);
    void* p = fl.Allocate(huge, 8);
    std::printf("     返回 %p（期望 nullptr）\n", p);
    verdict("D-32c", p == nullptr, "FreeListAllocator 拒绝超大请求");
    std::fflush(stdout);
}

// D-33：chunkSize 非 16 的倍数时，部分 chunk 仅 8 字节对齐
static void CasePoolAlign() {
    std::printf("--- D-33 PoolAllocator chunk 对齐 ---\n");
    const std::size_t chunk = 24;            // >= 16，但非 16 的倍数
    PoolAllocator pool(4080, chunk);
    pool.Init();
    int bad16 = 0, bad8 = 0;
    // 注意：chunkSize 会被向上对齐到 alignof(max_align_t)，故按实际 chunk 请求
    const std::size_t eff = pool.ChunkSize();
    std::printf("     请求 chunkSize=%zu -> 实际 %zu\n", chunk, eff);
    for (int i = 0; i < 8; ++i) {
        void* p = pool.Allocate(eff, 8);
        const std::size_t a = (std::size_t)p;
        std::printf("     chunk[%d]=%p mod8=%zu mod16=%zu\n", i, p, a % 8, a % 16);
        if (a % 8) ++bad8;
        if (a % 16) ++bad16;
    }
    verdict("D-33", bad16 == 0, "每个 chunk 均满足 16 字节对齐");
    std::fflush(stdout);
}

// D-37：重复释放导致 m_used 下溢
static void CaseDoubleFree() {
    std::printf("--- D-37 重复释放导致用量统计下溢 ---\n");
    PoolAllocator pool(1024, 64);
    pool.Init();
    void* p = pool.Allocate(pool.ChunkSize(), 8);
    std::printf("     分配后 m_used=%zu\n", pool.Used());
    pool.Free(p);
    std::printf("     释放一次 m_used=%zu\n", pool.Used());
    pool.Free(p);   // 重复释放：修复后应被识别并拒绝
    std::printf("     释放两次 m_used=%zu\n", pool.Used());
    verdict("D-37", pool.Used() == 0, "重复释放被拒绝，用量未下溢");
    std::fflush(stdout);
}

// D-34：赋值运算符未禁用，浅拷贝导致双重释放
static void CaseCopyAssign() {
    std::printf("--- D-34 拷贝赋值未禁用 ---\n");
    const bool lin  = std::is_copy_assignable<LinearAllocator>::value;
    const bool pool = std::is_copy_assignable<PoolAllocator>::value;
    const bool fl   = std::is_copy_assignable<FreeListAllocator>::value;
    const bool st   = std::is_copy_assignable<StackAllocator>::value;
    const bool c    = std::is_copy_assignable<CAllocator>::value;
    std::printf("     LinearAllocator   : %s\n", lin  ? "可拷贝" : "已禁用");
    std::printf("     PoolAllocator     : %s\n", pool ? "可拷贝" : "已禁用");
    std::printf("     FreeListAllocator : %s\n", fl   ? "可拷贝" : "已禁用");
    std::printf("     StackAllocator    : %s\n", st   ? "可拷贝" : "已禁用");
    std::printf("     CAllocator        : %s\n", c    ? "可拷贝" : "已禁用");
    verdict("D-34", !lin && !pool && !fl && !st && !c, "五个分配器均不可拷贝赋值");
}

int main(int argc, char** argv) {
    const char* mode = (argc > 1) ? argv[1] : "all";
    if (!std::strcmp(mode, "overflow")    || !std::strcmp(mode, "all")) CaseOverflow();
    if (!std::strcmp(mode, "overflow_fl") || !std::strcmp(mode, "all")) CaseOverflowFreeList();
    if (!std::strcmp(mode, "poolalign")   || !std::strcmp(mode, "all")) CasePoolAlign();
    if (!std::strcmp(mode, "doublefree")  || !std::strcmp(mode, "all")) CaseDoubleFree();
    if (!std::strcmp(mode, "copyassign")  || !std::strcmp(mode, "all")) CaseCopyAssign();
    std::printf("\n失败项: %d\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
