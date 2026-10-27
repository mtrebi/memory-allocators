// 第四轮探针：实施后复查发现的残留问题（D-42 / D-43 / D-44 及度量口径）
// 用法：probe_residual [stack_order|stack_double|stack_pop|used_semantics|
//                       pool_strict|align_nonpow2|fl_overflow|small_overhead|all]
// 构建：需要 -DNDEBUG（验证 Release 构建下的行为）与 -DALLOCATOR_VALIDATE
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <limits>

#include "StackAllocator.h"
#include "PoolAllocator.h"
#include "FreeListAllocator.h"
#include "LinearAllocator.h"
#include "CAllocator.h"

static int g_fail = 0;

static void verdict(const char* id, bool ok, const char* what) {
    std::printf("%s %-14s %s\n", ok ? "[ OK ]" : "[FAIL]", id, what);
    if (!ok) ++g_fail;
}

// D-42：栈分配器乱序释放。修复前 NDEBUG 下会把更早的块"释放"掉，
//       导致仍活动的块被重新派发（实测 Allocate 返回了 b 的地址）。
static void CaseStackOrder() {
    std::printf("--- D-42 栈分配器乱序释放 ---\n");
    StackAllocator s(4096);
    s.Init();
    char* a = (char*) s.Allocate(64, 8);
    char* b = (char*) s.Allocate(64, 8);
    std::memset(b, 0xBB, 64);
    std::printf("     a=%p b=%p\n", (void*)a, (void*)b);

    s.Free(a);   // a 不是栈顶，应被拒绝
    std::printf("     Free(a) 后：Depth=%zu RejectedFrees=%zu Used=%zu\n",
                s.Depth(), s.RejectedFrees(), s.Used());

    char* c = (char*) s.Allocate(64, 8);
    std::printf("     再 Allocate(64) -> %p\n", (void*)c);

    verdict("D-42a", c != b, "未把仍活动的 b 重新派发");
    verdict("D-42b", s.RejectedFrees() == 1, "乱序释放被计数并拒绝");
    verdict("D-42c", b[0] == (char)0xBB, "b 的数据未被破坏");
    // 注意：Free(a) 被拒意味着 a 仍然活动在栈中，收尾必须严格按 c -> b -> a
    s.Free(c);
    s.Free(b);
    s.Free(a);
    std::printf("     LIFO 收尾后 Depth=%zu Used=%zu Rejected=%zu\n",
                s.Depth(), s.Used(), s.RejectedFrees());
    verdict("D-42d", s.Depth() == 0 && s.Used() == 0, "按 LIFO 收尾后状态归零");
    std::fflush(stdout);
}

// D-42：重复释放同一个块
static void CaseStackDouble() {
    std::printf("--- D-42 栈分配器重复释放 ---\n");
    StackAllocator s(4096);
    s.Init();
    void* a = s.Allocate(64, 8);
    const bool first  = s.FreeChecked(a);
    const bool second = s.FreeChecked(a);
    std::printf("     第一次 FreeChecked=%d 第二次=%d Used=%zu\n",
                (int)first, (int)second, s.Used());
    verdict("D-42e", first && !second, "第二次释放被拒绝");
    verdict("D-42f", s.Used() == 0, "用量未下溢");
    std::fflush(stdout);
}

// Pop(marker) 必须同步收缩标记数组，否则后续 Free 会读到过期下界
static void CaseStackPop() {
    std::printf("--- D-42 栈分配器 Pop 与标记同步 ---\n");
    StackAllocator s(4096);
    s.Init();
    const std::size_t m = s.Push();
    void* a = s.Allocate(64, 8);
    void* b = s.Allocate(64, 8);
    (void)a;
    (void)b;
    std::printf("     分配两块后 Depth=%zu，Pop 到 marker=%zu\n", s.Depth(), m);
    s.Pop(m);
    std::printf("     Pop 后 Depth=%zu Used=%zu\n", s.Depth(), s.Used());
    verdict("D-42g", s.Depth() == 0 && s.Used() == 0, "Pop 丢弃了其后的分配标记");

    void* c = s.Allocate(64, 8);
    std::printf("     重新 Allocate -> %p（与 b 相同属正常，b 已被 Pop 丢弃）\n", c);
    verdict("D-42h", s.FreeChecked(c), "Pop 之后新分配可正常释放");
    std::fflush(stdout);
}

// 度量口径：Used() 一律是"从系统/池里实际占用的字节"，必须 >= 用户请求字节
static void CaseUsedSemantics() {
    std::printf("--- 度量口径一致性 ---\n");
    const std::size_t req = 100;

    LinearAllocator lin(4096); lin.Init();
    lin.Allocate(req, 8);
    const std::size_t uLin = lin.Used();

    StackAllocator stk(4096); stk.Init();
    stk.Allocate(req, 8);
    const std::size_t uStk = stk.Used();

    PoolAllocator pool(4096, 128); pool.Init();
    pool.Allocate(128, 8);
    const std::size_t uPool = pool.Used();

    FreeListAllocator fl(4096, FreeListAllocator::PlacementPolicy::FIND_FIRST); fl.Init();
    fl.Allocate(req, 8);
    const std::size_t uFl = fl.Used();

    CAllocator ca; ca.Init();
    void* p = ca.Allocate(req, 8);
    const std::size_t uCa = ca.Used();
    ca.Free(p);

    std::printf("     用户请求 %zu 字节 -> Used: Linear=%zu Stack=%zu Pool=%zu FreeList=%zu C=%zu\n",
                req, uLin, uStk, uPool, uFl, uCa);
    std::printf("     说明：Pool 的请求量是 chunk(128)，其余为 %zu\n", req);

    const bool allAtLeast = uLin >= req && uStk >= req && uPool >= 128 && uFl >= req && uCa >= req;
    verdict("METRIC-a", allAtLeast, "所有分配器的 Used 均不小于实际占用");
    // CAllocator 若只统计用户字节会等于 100，说明口径不一致
    verdict("METRIC-b", uCa > req, "CAllocator 计入自身块头与对齐余量（口径统一）");
    std::fflush(stdout);
}

// D-43：Pool 尺寸不符时必须失败，而不是静默给一个更大的块
static void CasePoolStrict() {
    std::printf("--- D-43 池分配器尺寸契约 ---\n");
    PoolAllocator p(4096, 64);
    p.Init();
    void* wrong = p.Allocate(16, 8);
    void* right = p.Allocate(p.ChunkSize(), 8);
    std::printf("     Allocate(16)（chunk=64）-> %p（期望 nullptr）\n", wrong);
    std::printf("     Allocate(64)            -> %p\n", right);
    verdict("D-43a", wrong == nullptr, "尺寸不符返回 nullptr");
    verdict("D-43b", right != nullptr, "尺寸相符正常返回");
    std::fflush(stdout);
}

// D-36 修订：FreeList 的对齐计算用取模，任意对齐值都应可用
static void CaseAlignNonPow2() {
    std::printf("--- 非 2 的幂对齐 ---\n");
    const std::size_t aligns[] = {8, 16, 24, 48, 96};
    int bad = 0;
    for (std::size_t i = 0; i < sizeof(aligns) / sizeof(aligns[0]); ++i) {
        FreeListAllocator f(4096, FreeListAllocator::PlacementPolicy::FIND_FIRST);
        f.Init();
        void* p = f.Allocate(64, aligns[i]);
        const std::size_t a = (std::size_t) p;
        const bool ok = (p != nullptr) && (a % aligns[i] == 0);
        std::printf("     alignment=%-3zu -> %p  mod=%zu  %s\n",
                    aligns[i], p, p ? a % aligns[i] : 0, ok ? "OK" : "违约");
        if (!ok) ++bad;
    }
    verdict("ALIGN", bad == 0, "任意对齐值均被正确满足");
    std::fflush(stdout);
}

// D-44：Find 内部 size + padding 回绕，使装不下的块被误判为命中
static void CaseFlOverflow() {
    std::printf("--- D-44 Find 内部加法回绕 ---\n");
    FreeListAllocator f(4096, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    f.Init();
    const std::size_t huge = std::numeric_limits<std::size_t>::max();
    void* a = f.Allocate(huge, 8);
    void* b = f.Allocate(huge - 16, 8);
    void* c = f.Allocate(4096, huge);      // 对齐值大于池容量
    std::printf("     Allocate(SIZE_MAX)      -> %p\n", a);
    std::printf("     Allocate(SIZE_MAX-16)   -> %p\n", b);
    std::printf("     Allocate(4096, SIZE_MAX)-> %p\n", c);
    verdict("D-44a", a == nullptr, "SIZE_MAX 请求被拒");
    verdict("D-44b", b == nullptr, "SIZE_MAX-16 请求被拒");
    verdict("D-44c", c == nullptr, "对齐值超池容量被拒");
    std::fflush(stdout);
}

// 小块开销测量（记录用，不作为通过条件）
static void CaseSmallOverhead() {
    std::printf("--- 小块分配开销（测量）---\n");
    FreeListAllocator f(1 << 20, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    f.Init();
    const int n = 1000;
    for (int i = 0; i < n; ++i) f.Allocate(16, 8);
    const std::size_t used = f.Used();
    const double overhead = (used - 16.0 * n) / (16.0 * n) * 100.0;
    std::printf("     %d 次 Allocate(16) 占用 %zu 字节，用户 %d 字节，开销 %.0f%%\n",
                n, used, 16 * n, overhead);
    std::printf("     （块头 16 字节 + 对齐填充到 32，故每块实际 32 字节）\n");
    verdict("SMALL-a", used == (std::size_t)32 * n, "每块恰好 32 字节，符合预期几何");
    std::fflush(stdout);
}

// D-46：Pool 重复释放。原先用 "m_used < m_chunkSize" 兜底，
//       只在池里仅剩一个活动块时成立；有第二个活动块时两次释放都通过，
//       该 chunk 被压入空闲链表两次，随后两次 Allocate 返回同一地址。
static void CasePoolDouble() {
    std::printf("--- D-46 Pool 重复释放 ---\n");
    PoolAllocator p(4096, 64);
    p.Init();
    void* a = p.Allocate(64, 8);
    void* b = p.Allocate(64, 8);
    std::printf("     a=%p b=%p Used=%zu\n", a, b, p.Used());

    p.Free(a);
    p.Free(a);   // 重复释放：b 仍活动，m_used=64 不低于 chunkSize，旧检查放行
    std::printf("     Free(a) 两次后 Used=%zu Rejected=%zu\n", p.Used(), p.RejectedFrees());

    void* c = p.Allocate(64, 8);
    void* d = p.Allocate(64, 8);
    std::printf("     再分配: c=%p d=%p\n", c, d);

    verdict("D-46a", p.RejectedFrees() == 1, "重复释放被计数并拒绝");
    verdict("D-46b", d != c, "同一地址未被派发两次");
    verdict("D-46c", p.LiveCount() == 3, "位图统计与实际活动块数一致（b,c,d）");
    std::fflush(stdout);
}

// D-45：FreeList 重复释放。原实现第二次 Free 会把空闲节点数据当成 padding
//       反推块起点，算出池外地址后写入 —— 实测 SIGSEGV。
static void CaseFreeListDouble() {
    std::printf("--- D-45 FreeList 重复释放 ---\n");
    FreeListAllocator f(4096, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    f.Init();
    void* a = f.Allocate(64, 8);
    void* b = f.Allocate(64, 8);
    std::printf("     a=%p b=%p\n", a, b);

    f.Free(a);
    f.Free(a);   // 修复前这里段错误
    std::printf("     Free(a) 两次后 Rejected=%zu（未崩溃）\n", f.RejectedFrees());

    void* c = f.Allocate(64, 8);
    void* d = f.Allocate(64, 8);
    std::printf("     再分配: c=%p d=%p\n", c, d);

    verdict("D-45a", f.RejectedFrees() == 1, "重复释放被计数并拒绝，未崩溃");
    verdict("D-45b", d != c, "同一地址未被派发两次");

    // 释放后块头 magic 必须已失效，否则下次还能被当成合法块
    f.Free(b);
    f.Free(c);
    f.Free(d);
    verdict("D-45c", f.Used() == 0, "全部释放后用量归零");
    std::fflush(stdout);
}

// 越界指针：不属于本分配器的地址必须被拒绝，而不是拿去反推块头
static void CaseForeignPtr() {
    std::printf("--- 外部指针 ---\n");
    int stackVar = 0;
    void* foreign = &stackVar;

    FreeListAllocator f(4096, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    f.Init();
    void* a = f.Allocate(64, 8);
    // 注意：Used() 按统一口径包含块头，64 字节请求实际记账 64 + 16 = 80。
    // 这里不能硬编码 80（几何一变就误报），要验证的是"外部指针不改变账目"。
    const std::size_t usedBefore = f.Used();
    f.Free(foreign);                       // 栈地址
    f.Free((void*)((char*)a + 4096));      // 池尾之外
    std::printf("     传入 2 个外部指针后 Rejected=%zu Used=%zu（此前 %zu）\n",
                f.RejectedFrees(), f.Used(), usedBefore);
    verdict("FOREIGN-a", f.RejectedFrees() == 2, "外部指针被拒绝");
    verdict("FOREIGN-b", f.Used() == usedBefore && usedBefore >= 64,
            "活动块用量未被影响");
    f.Free(a);

    PoolAllocator p(4096, 64);
    p.Init();
    void* pa = p.Allocate(64, 8);
    p.Free(foreign);
    p.Free((void*)((char*)pa + 8));        // 池内但未对齐到 chunk
    std::printf("     Pool 收到 2 个非法指针后 Rejected=%zu\n", p.RejectedFrees());
    verdict("FOREIGN-c", p.RejectedFrees() == 2, "Pool 拒绝外部与未对齐指针");
    std::fflush(stdout);
}

// D-47：零字节分配。修复前 Allocate(0) 会压入标记但 m_offset 不前进，
//       返回的地址落在空区间 [lower, upper) 上，任何指针都判越界，
//       这块"分配"永远无法释放 —— Depth() 卡住不归零。
static void CaseZeroSize() {
    std::printf("--- D-47 零字节分配 ---\n");
    StackAllocator s(1024);
    s.Init();
    void* z = s.Allocate(0, 0);
    std::printf("     Allocate(0) -> %p Depth=%zu\n", z, s.Depth());
    verdict("D-47a", z == nullptr, "零字节请求返回 nullptr");
    verdict("D-47b", s.Depth() == 0, "未残留无法释放的标记");

    // 正常分配-释放一轮后必须干净归零
    void* a = s.Allocate(64, 8);
    void* b = s.Allocate(64, 8);
    s.Allocate(0, 0);                       // 中途插入的零字节请求不得影响状态
    const bool fb = s.FreeChecked(b);
    const bool fa = s.FreeChecked(a);
    std::printf("     64/64/0 三轮后 Free(b)=%d Free(a)=%d Depth=%zu Used=%zu\n",
                (int)fb, (int)fa, s.Depth(), s.Used());
    verdict("D-47c", fb && fa && s.Depth() == 0 && s.Used() == 0,
            "混入零字节请求后 LIFO 收尾仍归零");

    // 其它分配器对零字节的处理（记录，不作为失败条件）
    FreeListAllocator f(1024, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    f.Init();
    void* fz = f.Allocate(0, 8);
    f.Free(fz);
    std::printf("     FreeList Allocate(0) -> %p，释放后 Used=%zu Rejected=%zu\n",
                fz, f.Used(), f.RejectedFrees());
    verdict("D-47d", fz != nullptr && f.Used() == 0 && f.RejectedFrees() == 0,
            "FreeList 的零字节块有块头，可正常释放");
    std::fflush(stdout);
}

int main(int argc, char** argv) {
    const char* mode = (argc > 1) ? argv[1] : "all";
    if (!std::strcmp(mode, "stack_order")    || !std::strcmp(mode, "all")) CaseStackOrder();
    if (!std::strcmp(mode, "stack_double")   || !std::strcmp(mode, "all")) CaseStackDouble();
    if (!std::strcmp(mode, "stack_pop")      || !std::strcmp(mode, "all")) CaseStackPop();
    if (!std::strcmp(mode, "used_semantics") || !std::strcmp(mode, "all")) CaseUsedSemantics();
    if (!std::strcmp(mode, "pool_strict")    || !std::strcmp(mode, "all")) CasePoolStrict();
    if (!std::strcmp(mode, "align_nonpow2")  || !std::strcmp(mode, "all")) CaseAlignNonPow2();
    if (!std::strcmp(mode, "fl_overflow")    || !std::strcmp(mode, "all")) CaseFlOverflow();
    if (!std::strcmp(mode, "small_overhead") || !std::strcmp(mode, "all")) CaseSmallOverhead();
    if (!std::strcmp(mode, "pool_double")    || !std::strcmp(mode, "all")) CasePoolDouble();
    if (!std::strcmp(mode, "fl_double")      || !std::strcmp(mode, "all")) CaseFreeListDouble();
    if (!std::strcmp(mode, "foreign_ptr")    || !std::strcmp(mode, "all")) CaseForeignPtr();
    if (!std::strcmp(mode, "zero_size")      || !std::strcmp(mode, "all")) CaseZeroSize();
    std::printf("\n失败项: %d\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
