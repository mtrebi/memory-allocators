// 随机对拍：FreeListAllocator 随机分配/释放序列，检测重叠派发、对齐违约、数据覆盖
// 用法：fuzz_freelist <first|best> <seed> <ops>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <vector>
#include <random>

#include "check_allocator.h"
#include "FreeListAllocator.h"

int main(int argc, char** argv) {
    const char* policy = (argc > 1) ? argv[1] : "first";
    const unsigned seed = (argc > 2) ? static_cast<unsigned>(std::atoi(argv[2])) : 1u;
    const int ops = (argc > 3) ? std::atoi(argv[3]) : 20000;

    const std::size_t poolSize = 1u << 20;   // 1 MiB
    FreeListAllocator a(poolSize, std::strcmp(policy, "best") == 0
                                     ? FreeListAllocator::PlacementPolicy::FIND_BEST
                                     : FreeListAllocator::PlacementPolicy::FIND_FIRST);
    a.Init();

    Checker c;
    std::vector<void*> live;
    std::mt19937 rng(seed);
    const std::size_t aligns[] = {8, 16, 32, 64};
    int allocs = 0, frees = 0, fails = 0;

    for (int i = 0; i < ops; ++i) {
        const bool doAlloc = live.empty() || (rng() % 100) < 60;
        if (doAlloc) {
            const std::size_t size  = 16 + (rng() % 496);
            const std::size_t align = aligns[rng() % 4];
            void* p = c.Allocate(&a, size, align);
            if (p == nullptr) { ++fails; }
            else { live.push_back(p); ++allocs; }
        } else {
            const std::size_t idx = rng() % live.size();
            c.Free(&a, live[idx]);
            live[idx] = live.back();
            live.pop_back();
            ++frees;
        }
        if (c.errors > 0) {
            std::printf("首次违规出现在第 %d 步（alloc=%d free=%d）\n", i, allocs, frees);
            break;
        }
    }

    while (!live.empty()) { c.Free(&a, live.back()); live.pop_back(); ++frees; }

    std::printf("policy=%s seed=%u ops=%d alloc=%d free=%d 分配失败=%d 违规=%d\n",
                policy, seed, ops, allocs, frees, fails, c.errors);
    return c.errors == 0 ? 0 : 1;
}
