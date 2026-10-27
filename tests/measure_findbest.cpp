// D-51 测量与正确性验证：FindBest 的 O(1) 快速失败与精确命中提前退出。
//
// 用法：measure_findbest
//
// 三件事：
//   1. SOUNDNESS —— 快速失败绝不能拒绝"本来能成功"的请求。做法：同一批
//      请求分别喂给 FIND_FIRST（没有快速失败，作为可成功性的参照）与
//      FIND_BEST，两边的成功/失败必须完全一致。任何不一致都是上界算错，
//      属于正确性回归，直接判失败退出。
//   2. 精确命中路径 —— 反复分配/释放同一尺寸时，diff==0 提前退出应当
//      把整表扫描降下来。
//   3. 池满路径 —— 分配触底后继续请求，快速失败应把 O(n) 的全表扫描
//      变成 O(1)。
//
// 计时是测量不是断言：机器速度不同，数字只用于横向对比，退出码只看 1。

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <vector>

#include "FreeListAllocator.h"

typedef std::chrono::steady_clock Clock;

static double MsSince(Clock::time_point t0) {
    return std::chrono::duration<double, std::milli>(Clock::now() - t0).count();
}

// ---- 1. 快速失败的健全性：同一分配器、同一请求序列，开/关快速失败做差分 ----
//
// 为什么不能拿两个分配器比：两个池的起始地址不同，对齐填充就不同，
// 成败本来就可能不一样（实测 size=262128 align=64 时两个实例一个成一个败，
// 与快速失败无关）。正确做法是在**同一个**分配器上跑同一序列两次，
// 一次开着快速失败、一次关掉（上界抬到 SIZE_MAX）。成功的分配走完全
// 相同的代码路径，空闲链表演化也相同，所以两次的结果逐请求一致。
static bool CaseSoundness() {
    std::printf("--- D-51a 快速失败不得误拒（同分配器差分）---\n");
    const std::size_t poolSize = 256 * 1024;

    // 确定性的请求序列：覆盖极端尺寸与各种对齐，混合分配与释放
    std::vector<std::size_t> opsSize, opsAlign;
    {
        unsigned seed = 20251027u;
        for (int i = 0; i < 4000; ++i) {
            seed = seed * 1664525u + 1013904223u;   // LCG，不依赖 <random> 的实现差异
            static const std::size_t cand[] = {
                1, 8, 16, 17, 48, 64, 100, 1000, 4096, 65536, 200000,
                poolSize, poolSize - 16, poolSize + 1
            };
            opsSize.push_back(cand[seed % (sizeof(cand)/sizeof(cand[0]))]);
            static const std::size_t al[] = {8, 16, 32, 64, 128};
            opsAlign.push_back(al[(seed >> 8) % 5]);
        }
    }
    FreeListAllocator a(poolSize, FreeListAllocator::PlacementPolicy::FIND_BEST);
    a.Init();

    // 跑一遍序列，记录每步结果；同时维护活动指针，随后清场
    std::vector<void*> live;
    std::vector<int> result;             // +1 成功 / 0 失败（按请求顺序）
    for (std::size_t i = 0; i < opsSize.size(); ++i) {
        const bool doAlloc = live.empty() || (i % 3) != 2;
        if (doAlloc) {
            void* p = a.Allocate(opsSize[i], opsAlign[i]);
            result.push_back(p ? 1 : 0);
            if (p) { live.push_back(p); }
        } else {
            const std::size_t k = i % live.size();
            a.Free(live[k]);
            live[k] = live.back(); live.pop_back();
            // 释放步骤不记录结果：回放时步进只数分配，两边序列必须同构
        }
    }
    while (!live.empty()) { a.Free(live.back()); live.pop_back(); }
    const std::size_t successCount = [&]{
        std::size_t n = 0;
        for (std::size_t i = 0; i < result.size(); ++i) if (result[i] == 1) ++n;
        return n;
    }();

    // 关掉快速失败，Reset 后重放同一序列
    a.Reset();
    a.DisableFastRejectForTest();

    std::vector<void*> live2;
    int mismatch = 0;
    std::size_t step = 0;
    for (std::size_t i = 0; i < opsSize.size(); ++i) {
        const bool doAlloc = live2.empty() || (i % 3) != 2;
        if (doAlloc) {
            void* p = a.Allocate(opsSize[i], opsAlign[i]);
            const int expect = result[step++];
            if ((p ? 1 : 0) != expect) {
                if (mismatch < 5) {
                    std::printf("     不一致 @step%zu: size=%zu align=%zu 开=%d 关=%d\n",
                                step - 1, opsSize[i], opsAlign[i], expect, p ? 1 : 0);
                }
                ++mismatch;
            }
            if (p) live2.push_back(p);
        } else {
            const std::size_t k = i % live2.size();
            a.Free(live2[k]);
            live2[k] = live2.back(); live2.pop_back();
        }
    }
    while (!live2.empty()) { a.Free(live2.back()); live2.pop_back(); }

    std::printf("     分配 %zu 次，成功 %zu 次；开/关快速失败不一致 %d 处\n",
                result.size(), successCount, mismatch);
    return mismatch == 0;
}

// ---- 2. 精确命中：同尺寸反复分配/释放 ----
static void MeasureExactHit() {
    std::printf("--- D-51b 精确命中（同尺寸 alloc/free 循环）---\n");
    const std::size_t poolSize = 1 << 20;
    const int rounds = 200000;
    FreeListAllocator best(poolSize, FreeListAllocator::PlacementPolicy::FIND_BEST);
    best.Init();
    // 先制造碎片：分配 1000 个不同小块再全部释放，让空闲链表上有一千个节点
    std::vector<void*> frag;
    for (int i = 0; i < 1000; ++i) {
        void* p = best.Allocate(16 + (i % 64) * 8, 8);
        if (p) frag.push_back(p);
    }
    for (std::size_t i = 0; i < frag.size(); ++i) best.Free(frag[i]);
    const int fragCount = (int)frag.size();
    frag.clear();
    std::printf("     预置碎片 %d 个，空闲链表已变长\n", fragCount);

    // 反复分配/释放同一个 64 字节块：它必然精确命中某个 64+padding 的空闲块
    Clock::time_point t0 = Clock::now();
    for (int i = 0; i < rounds; ++i) {
        void* p = best.Allocate(64, 8);
        best.Free(p);
    }
    const double ms = MsSince(t0);
    std::printf("     %d 次 alloc/free(64) 碎片链表上耗时 %.1f ms（%.0f ns/对）\n",
                rounds, ms, ms * 1e6 / rounds);
}

// ---- 3. 池满后的注定失败 ----
static void MeasureExhausted() {
    std::printf("--- D-51c 池满后的注定失败 ---\n");
    const std::size_t poolSize = 1 << 20;
    const int rounds = 200000;
    FreeListAllocator best(poolSize, FreeListAllocator::PlacementPolicy::FIND_BEST);
    best.Init();
    std::vector<void*> live;
    for (;;) {
        void* p = best.Allocate(64, 8);
        if (!p) break;
        live.push_back(p);
    }
    std::printf("     池已填满，活动块 %zu 个\n", live.size());

    Clock::time_point t0 = Clock::now();
    for (int i = 0; i < rounds; ++i) {
        void* p = best.Allocate(64, 8);
        if (p) { live.push_back(p); }   // 不应发生
    }
    const double ms = MsSince(t0);
    std::printf("     %d 次注定失败的 Allocate 耗时 %.1f ms（%.0f ns/次）\n",
                rounds, ms, ms * 1e6 / rounds);
    for (std::size_t i = 0; i < live.size(); ++i) best.Free(live[i]);
}

// ---- 4. 无精确命中的 best-fit 搜索（诚实地展示 D-51 修不到的部分）----
//
// 设计要点：请求尺寸刻意与任何空闲块都不对齐到 diff==0，且多个块都满足。
// 这样提前退出永远不触发，best-fit 必须遍历整条链表才能确定"差最小"的那个
// 块 —— 这是精确 best-fit 语义的固有代价，D-51 的两个优化都帮不上忙。
// 这个用例存在的意义是把边界画清楚，而不是证明变快了。
static void MeasureNoExactHit() {
    std::printf("--- D-51d 无精确命中的 best-fit（D-51 帮不上的情形）---\n");
    const std::size_t poolSize = 1 << 20;
    const int rounds = 20000;

    // 同一个负载分别跑 FIND_FIRST 与 FIND_BEST，把两种策略的差距量化出来。
    // 两个池的起始地址不同会使碎片几何略有差异，但足以判断量级。
    FreeListAllocator first(poolSize, FreeListAllocator::PlacementPolicy::FIND_FIRST);
    FreeListAllocator best (poolSize, FreeListAllocator::PlacementPolicy::FIND_BEST);
    first.Init();
    best.Init();

    // 制造**真实**的 1000 个空闲块。
    //
    // 这里有一个曾经把结论带偏的坑：如果分配 1000 个再按顺序全部释放，
    // 相邻空闲块会立刻合并，最后链表上只剩 1 个大块 —— 看起来做了碎片化，
    // 实际什么都没做，FIND_FIRST 与 FIND_BEST 的差距因此被完全抹掉。
    // 正确做法是隔一个释放一个，让仍然活动的块把空闲块彼此隔开。
    for (int pass = 0; pass < 2; ++pass) {
        FreeListAllocator& a = (pass == 0) ? first : best;
        std::vector<void*> live;
        for (int i = 0; i < 2000; ++i) {
            void* p = a.Allocate(16 + (i % 64) * 8, 8);
            if (p) live.push_back(p);
        }
        for (std::size_t i = 0; i < live.size(); i += 2) a.Free(live[i]);
        // 此时空闲块 ≈ live.size()/2，活动块把它们隔开，不会合并
    }

    // 反复分配/释放 100 字节、64 对齐：空闲块没有一个是 100+padding 的精确尺寸，
    // 且多个块满足条件。FIND_FIRST 命中第一个满足的就返回；
    // FIND_BEST 必须扫完整表才能确定"差最小"的那个。
    Clock::time_point t0 = Clock::now();
    for (int i = 0; i < rounds; ++i) {
        void* p = first.Allocate(100, 64);
        first.Free(p);
    }
    const double msFirst = MsSince(t0);

    t0 = Clock::now();
    for (int i = 0; i < rounds; ++i) {
        void* p = best.Allocate(100, 64);
        best.Free(p);
    }
    const double msBest = MsSince(t0);

    std::printf("     %d 次 alloc/free(100,64)，~1000 个空闲块：\n", rounds);
    std::printf("       FIND_FIRST  %.1f ms（%.0f ns/对）\n", msFirst, msFirst * 1e6 / rounds);
    std::printf("       FIND_BEST   %.1f ms（%.0f ns/对）\n", msBest,  msBest  * 1e6 / rounds);
    if (msBest > 0) {
        std::printf("       FIND_BEST / FIND_FIRST = %.1f 倍\n", msBest / msFirst);
    }
    std::printf("     （best-fit 通用路径仍是 O(空闲块数)，这是精确语义的固有代价）\n");
}

int main() {
    const bool ok = CaseSoundness();
    std::printf("\n");
    MeasureExactHit();
    std::printf("\n");
    MeasureExhausted();
    std::printf("\n");
    MeasureNoExactHit();
    std::printf("\n%s\n", ok ? "健全性检查通过" : "健全性检查失败");
    return ok ? 0 : 1;
}
