// 第二轮深挖探针：验证第一版计划书未覆盖的缺陷
// 用法：probe_extra [overlap|zeroalign|rest|peak|all]
// 构建：需要 -DALLOCATOR_VALIDATE（rest 用例依赖内部不变量校验）
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <vector>
#include <cstring>

#include "check_allocator.h"
#include "FreeListAllocator.h"
#include "PoolAllocator.h"
#include "CAllocator.h"
#include "LinearAllocator.h"

static int g_fail = 0;

static void verdict(const char* id, bool ok, const char* what) {
    std::printf("%s %-8s %s\n", ok ? "[ OK ]" : "[FAIL]", id, what);
    if (!ok) ++g_fail;
}

// D-24：Free() 把节点放在 headerAddress，并把 padding 计入 blockSize，
//       导致空闲块向后多覆盖 alignmentPadding 字节，侵占相邻块的 AllocationHeader
static void CaseOverlap() {
    std::printf("--- D-24 释放块向后越界，侵占相邻块头部 ---\n");
    FreeListAllocator a(4096, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    a.Init();
    Checker c;
    void* A = c.Allocate(&a, 64, 32);
    void* B = c.Allocate(&a, 64, 32);
    std::printf("     A=%p B=%p 间距=%zu\n", A, B, (std::size_t)B - (std::size_t)A);

    std::size_t headerBefore = 0;
    std::memcpy(&headerBefore, (void*)((std::size_t)B - 16), sizeof(headerBefore));
    std::printf("     B 的 AllocationHeader.blockSize 释放前 = %zu\n", headerBefore);

    c.Free(&a, A);
    void* D = c.Allocate(&a, 96, 8);
    std::printf("     free(A) 后 Allocate(96,8) -> %p，数据区 [%p, +96)\n", D, D);

    std::size_t headerAfter = 0;
    std::memcpy(&headerAfter, (void*)((std::size_t)B - 16), sizeof(headerAfter));
    std::printf("     B 的 AllocationHeader.blockSize 分配后 = %zu\n", headerAfter);

    verdict("D-24", headerBefore == headerAfter, "相邻块块头未被侵占");

    // 交叉校验：B 的数据仍完好，且内部不变量成立
    const unsigned char* bb = static_cast<const unsigned char*>(B);
    bool intact = true;
    for (std::size_t i = 0; i < 64; ++i) if (bb[i] != 2) { intact = false; break; }
    verdict("D-24b", intact, "相邻块 B 的数据未被覆盖");

    std::vector<void*> live;
    live.push_back(B);
    live.push_back(D);
    std::string err;
    const bool okInv = a.Validate(live, &err);
    std::printf("     不变量校验：%s\n", okInv ? "通过" : err.c_str());
    verdict("D-24c", okInv, "空闲链表不变量成立");
    std::fflush(stdout);
}

// D-25：Allocate 默认 alignment = 0，CalculatePaddingWithHeader 整数除零
static void CaseZeroAlign() {
    std::printf("--- D-25 默认 alignment=0 导致整数除零 ---\n");
    FreeListAllocator a(4096, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    a.Init();
    std::printf("     调用 Allocate(64)（使用默认 alignment=0）\n");
    std::fflush(stdout);
    void* p = a.Allocate(64);
    std::printf("     返回 %p\n", p);
    verdict("D-25", p != nullptr, "默认 alignment=0 不再除零，正常返回");
    std::fflush(stdout);
}

// D-26：rest 落在 (0, sizeof(Node)) 时，插入空闲节点会越界写入
static void CaseRest() {
    std::printf("--- D-26 剩余空间 0<rest<sizeof(Node) 时越界写入 ---\n");
    const std::size_t total = 512;
    FreeListAllocator a(total, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    a.Init();
    Checker c;
    // alignment 8 时 padding = 16（块头），故 required = size + 16
    // 取 size = 488，使 rest = 512 - 504 = 8 < sizeof(Node) = 16
    void* p = c.Allocate(&a, 488, 8);
    std::printf("     total=%zu Allocate(488,8)=%p\n", total, p);
    verdict("D-26a", p != nullptr, "分配成功");

    c.Free(&a, p);
    const std::size_t blocks = a.FreeBlockCount();
    const std::size_t largest = a.LargestFreeBlock();
    std::printf("     全部释放后：空闲块=%zu 最大块=%zu（期望 1 / %zu）\n", blocks, largest, total);
    verdict("D-26b", blocks == 1 && largest == total,
            "余量已并入块大小，未留下永久空洞");
    std::fflush(stdout);
}

// D-16 / D-27：CAllocator 不统计用量，空间对比失真
static void CasePeak() {
    std::printf("--- D-16 CAllocator 未统计 m_used/m_peak ---\n");
    struct ProbeC : CAllocator { std::size_t peak() const { return m_peak; } };
    ProbeC a;
    a.Init();
    std::vector<void*> ps;
    for (int i = 0; i < 10; ++i) ps.push_back(a.Allocate(1024, 8));

    // 口径说明：Used()/Peak() 统计的是"从系统实际占用的字节"，
    // CAllocator 每次分配额外带 16 字节块头 + 至多 8 字节对齐余量。
    // 10 x (1024 + 16 + 8) = 10480。
    const std::size_t peak = a.peak();
    std::printf("     分配 10 x 1024 字节后 m_peak=%zu（期望 10480 = 10 x 1048）\n", peak);
    verdict("D-16a", peak == 10480, "峰值统计含块头与对齐余量，口径与其它分配器一致");

    for (std::size_t i = 0; i < ps.size(); ++i) a.Free(ps[i]);
    std::printf("     全部释放后 m_used=%zu（期望 0）\n", a.Used());
    verdict("D-16b", a.Used() == 0, "释放后用量归零，未下溢");
    std::fflush(stdout);
}

int main(int argc, char** argv) {
    const char* mode = (argc > 1) ? argv[1] : "all";
    if (!std::strcmp(mode, "overlap")   || !std::strcmp(mode, "all")) CaseOverlap();
    if (!std::strcmp(mode, "zeroalign") || !std::strcmp(mode, "all")) CaseZeroAlign();
    if (!std::strcmp(mode, "rest")      || !std::strcmp(mode, "all")) CaseRest();
    if (!std::strcmp(mode, "peak")      || !std::strcmp(mode, "all")) CasePeak();
    std::printf("\n失败项: %d\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
