// 压力测试 + 内部不变量校验
// 用法：stress_validate <first|best> <seed> <ops> [uniform|skewed|longlived] [validate_every]
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
#include <string>
#include <vector>
#include <random>

#include "FreeListAllocator.h"

int main(int argc, char** argv) {
    const char* policy = (argc > 1) ? argv[1] : "first";
    const unsigned seed = (argc > 2) ? static_cast<unsigned>(std::atoi(argv[2])) : 1u;
    const int ops = (argc > 3) ? std::atoi(argv[3]) : 100000;
    const char* mode = (argc > 4) ? argv[4] : "uniform";
    const int every = (argc > 5) ? std::atoi(argv[5]) : 1000;

    const std::size_t poolSize = 4u << 20;
    FreeListAllocator a(poolSize, std::strcmp(policy, "best") == 0
                                     ? FreeListAllocator::PlacementPolicy::FIND_BEST
                                     : FreeListAllocator::PlacementPolicy::FIND_FIRST);
    a.Init();

    std::vector<void*> live;
    std::vector<std::size_t> liveSize;
    live.reserve(4096);
    std::mt19937 rng(seed);
    const std::size_t aligns[] = {8, 16, 32, 64};
    long long allocs = 0, frees = 0, fails = 0;
    std::size_t liveBytes = 0, peakLive = 0;

    for (int i = 0; i < ops; ++i) {
        const bool doAlloc = live.empty() || (rng() % 100) < 60;
        if (doAlloc) {
            std::size_t size;
            if (!std::strcmp(mode, "skewed")) {
                // 小对象占绝大多数：16..64 为主，偶发大对象
                const unsigned r = rng() % 1000;
                size = (r < 900) ? (16 + rng() % 48) : (16 + rng() % 4000);
            } else {
                size = 16 + rng() % 496;
            }
            const std::size_t align = aligns[rng() % 4];
            void* p = a.Allocate(size, align);
            if (p == nullptr) ++fails;
            else {
                std::memset(p, 0xAB, size);
                live.push_back(p);
                liveSize.push_back(size);
                liveBytes += size;
                if (liveBytes > peakLive) peakLive = liveBytes;
                ++allocs;
            }
        } else {
            std::size_t idx;
            if (!std::strcmp(mode, "longlived") && live.size() > 8) {
                // 优先释放后段（长期存活），制造碎片
                idx = live.size() - 1 - (rng() % 2);
            } else {
                idx = rng() % live.size();
            }
            liveBytes -= liveSize[idx];
            a.Free(live[idx]);
            live[idx] = live.back();
            liveSize[idx] = liveSize.back();
            live.pop_back();
            liveSize.pop_back();
            ++frees;
        }

        if (every > 0 && (i + 1) % every == 0) {
            std::string err;
            if (!a.Validate(live, &err)) {
                std::printf("[FAIL] 第 %d 步不变量被破坏：%s（alloc=%lld free=%lld live=%zu）\n",
                            i + 1, err.c_str(), allocs, frees, live.size());
                return 1;
            }
        }
    }

    while (!live.empty()) { a.Free(live.back()); live.pop_back(); liveSize.pop_back(); ++frees; }

    std::string err;
    if (!a.Validate(live, &err)) {
        std::printf("[FAIL] 收尾校验失败：%s\n", err.c_str());
        return 1;
    }

    const std::size_t blocks = a.FreeBlockCount();
    const std::size_t largest = a.LargestFreeBlock();
    std::printf("policy=%-5s mode=%-9s ops=%d alloc=%lld fail=%lld(%.1f%%) 峰值活动=%zuKB/池%zuKB(%.1f%%) 释放后空闲块=%zu 最大块=%zuKB %s\n",
                policy, mode, ops, allocs, fails, 100.0 * fails / (allocs + fails),
                peakLive / 1024, poolSize / 1024, 100.0 * peakLive / poolSize, blocks, largest / 1024,
                (blocks == 1 && largest == poolSize) ? "→ 完全合并" : "→ 未完全合并");
    return 0;
}
