// 分配器一致性检查器：检测重叠派发、对齐违约、数据被覆盖
#ifndef CHECK_ALLOCATOR_H
#define CHECK_ALLOCATOR_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <utility>
#include <vector>

#include "Allocator.h"

class Checker {
public:
    struct Block { std::size_t size; unsigned char tag; };

    void* Allocate(Allocator* a, std::size_t size, std::size_t align) {
        void* p = a->Allocate(size, align);
        if (p == nullptr) return nullptr;
        const std::uintptr_t s = (std::uintptr_t)p;
        const std::uintptr_t e = s + size;

        if (align != 0 && (s % align) != 0) {
            std::printf("  [VIOLATION] 对齐违约: ptr=%p 请求 alignment=%zu (mod=%zu)\n",
                        p, align, s % align);
            ++errors;
        }
        for (auto& kv : live) {
            const std::uintptr_t bs = kv.first;
            const std::uintptr_t be = bs + kv.second.size;
            if (s < be && bs < e) {
                std::printf("  [VIOLATION] 重叠派发: 新块 [%p,+%zu) 与活动块 [%p,+%zu) 相交\n",
                            (void*)s, size, (void*)bs, kv.second.size);
                ++errors;
            }
        }
        const unsigned char tag = static_cast<unsigned char>(++counter_);
        std::memset(p, tag, size);
        live[s] = Block{size, tag};
        return p;
    }

    void Free(Allocator* a, void* p) {
        if (p == nullptr) return;
        const std::uintptr_t s = (std::uintptr_t)p;
        auto it = live.find(s);
        if (it == live.end()) { std::printf("  [VIOLATION] 释放未知指针 %p\n", p); ++errors; return; }
        const unsigned char* b = static_cast<const unsigned char*>(p);
        for (std::size_t i = 0; i < it->second.size; ++i) {
            if (b[i] != it->second.tag) {
                std::printf("  [VIOLATION] 数据被覆盖: ptr=%p 偏移 %zu 期望 %u 实际 %u\n",
                            p, i, it->second.tag, b[i]);
                ++errors;
                break;
            }
        }
        live.erase(it);
        a->Free(p);
    }

    std::size_t liveBytes() const {
        std::size_t total = 0;
        for (auto& kv : live) total += kv.second.size;
        return total;
    }

    int errors = 0;
    std::map<std::uintptr_t, Block> live;

private:
    unsigned counter_ = 0;
};

#endif
