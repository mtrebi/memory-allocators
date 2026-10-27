// PoolAllocator 随机对拍：检测重复派发、重叠、越界
// 用法：fuzz_pool <seed> <ops>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <vector>
#include <random>

#include "check_allocator.h"
#include "PoolAllocator.h"

int main(int argc, char** argv) {
    const unsigned seed = (argc > 1) ? static_cast<unsigned>(std::atoi(argv[1])) : 1u;
    const int ops = (argc > 2) ? std::atoi(argv[2]) : 200000;

    const std::size_t chunk = 64;
    const std::size_t total = 64 * 4096;   // 4096 个 chunk

    PoolAllocator a(total, chunk);
    a.Init();

    Checker c;
    std::vector<void*> live;
    live.reserve(4096);
    std::mt19937 rng(seed);
    long long allocs = 0, frees = 0, fails = 0;

    for (int i = 0; i < ops; ++i) {
        const bool doAlloc = live.empty() || (rng() % 100) < 55;
        if (doAlloc) {
            void* p = c.Allocate(&a, chunk, 8);
            if (p == nullptr) ++fails;
            else { live.push_back(p); ++allocs; }
        } else {
            const std::size_t idx = rng() % live.size();
            c.Free(&a, live[idx]);
            live[idx] = live.back();
            live.pop_back();
            ++frees;
        }
        if (c.errors > 0) {
            std::printf("首次违规在第 %d 步（alloc=%lld free=%lld）\n", i, allocs, frees);
            break;
        }
    }

    while (!live.empty()) { c.Free(&a, live.back()); live.pop_back(); ++frees; }

    std::printf("seed=%u ops=%d alloc=%lld free=%lld fail=%lld 违规=%d\n",
                seed, ops, allocs, frees, fails, c.errors);
    return c.errors == 0 ? 0 : 1;
}
