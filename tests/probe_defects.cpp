// 缺陷复现探针（回归验证用，不属于产品代码）
// 用途：以最小可复现用例验证《修复计划书》中登记的各条缺陷。
// 用法：probe_defects [case]
//       case ∈ all | linear | stack | pool | lost | findbest | list
//       其中 findbest 在未修复版本上会死循环，请配合超时运行。
// 构建：g++ -std=c++11 -I../includes -DNDEBUG -g -o probe_defects probe_defects.cpp \
//          ../src/Allocator.cpp ../src/LinearAllocator.cpp ../src/StackAllocator.cpp \
//          ../src/PoolAllocator.cpp ../src/FreeListAllocator.cpp

#include <cstdio>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

#include "LinearAllocator.h"
#include "StackAllocator.h"
#include "PoolAllocator.h"
#include "FreeListAllocator.h"
#include "SinglyLinkedList.h"

static int g_fail = 0;

static void Check(bool ok, const char* id, const char* msg) {
    std::printf("[%s] %-6s %s\n", ok ? " OK " : "FAIL", id, msg);
    if (!ok) ++g_fail;
}

// D-05：LinearAllocator 用 m_offset % alignment 判定对齐，与绝对地址无关
static void CaseLinear() {
    std::printf("--- D-05 LinearAllocator 对齐 ---\n");
    LinearAllocator a(1 << 20);
    a.Init();
    void* p1 = a.Allocate(64, 64);
    void* p2 = a.Allocate(64, 64);
    void* p3 = a.Allocate(64, 128);
    std::printf("     p1=%p (mod64=%zu) p2=%p (mod64=%zu) p3=%p (mod128=%zu)\n",
                p1, (std::size_t)p1 % 64, p2, (std::size_t)p2 % 64, p3, (std::size_t)p3 % 128);
    Check((std::size_t)p1 % 64 == 0, "D-05a", "64 字节对齐请求返回对齐地址");
    Check((std::size_t)p2 % 64 == 0, "D-05b", "连续第二次 64 字节对齐请求返回对齐地址");
    Check((std::size_t)p3 % 128 == 0, "D-05c", "128 字节对齐请求返回对齐地址");
}

// D-06：StackAllocator 在地址已对齐时仍额外填充一个 alignment
static void CaseStack() {
    std::printf("--- D-06 StackAllocator 填充 ---\n");
    StackAllocator a(1 << 20);
    a.Init();
    void* p1 = a.Allocate(16, 8);
    void* p2 = a.Allocate(16, 8);
    const std::size_t gap = (std::size_t)p2 - (std::size_t)p1;
    std::printf("     p1=%p p2=%p 间距=%zu（期望 16）\n", p1, p2, gap);
    Check(gap == 16, "D-06", "已对齐时不应产生额外填充");
}

// D-02：PoolAllocator::Reset 未清空 free list，导致同一 chunk 重复派发
static void CasePool() {
    std::printf("--- D-02 PoolAllocator Reset ---\n");
    PoolAllocator a(4096, 64);
    a.Init();
    void* first = a.Allocate(64, 8);
    a.Free(first);
    a.Reset();
    std::vector<void*> got;
    for (int i = 0; i < 3; ++i) got.push_back(a.Allocate(64, 8));
    std::printf("     first=%p -> %p %p %p\n", first, got[0], got[1], got[2]);
    const bool distinct = got[0] != got[1] && got[1] != got[2] && got[0] != got[2];
    Check(distinct, "D-02", "Reset 后不得重复派发同一 chunk");
}

// D-04：Free 时若空闲链表已空，节点未被插入 -> 块丢失；OOM 仅断言 -> Release 段错误
// 注意：未修复版本在本用例末尾会 SIGSEGV（NDEBUG），请单独进程运行
static void CaseFreeListLost() {
    std::printf("--- D-04 FreeListAllocator 空闲链表耗尽后的块丢失 ---\n");
    FreeListAllocator a(480, FreeListAllocator::PlacementPolicy::FIND_FIRST);  // 480 = 6 x 80
    a.Init();
    std::vector<void*> v;
    for (int i = 0; i < 6; ++i) {
        v.push_back(a.Allocate(64, 8));
        std::printf("     alloc[%d]=%p\n", i, v.back());
    }
    a.Free(v[5]);                              // 链表已空 -> 该节点不会被插入
    std::printf("     free(%p) 完成，随后再次分配：\n", v[5]);
    std::fflush(stdout);
    void* again = a.Allocate(64, 8);           // 未修复版本在此处崩溃
    std::printf("     again=%p\n", again);
    Check(again == v[5], "D-04", "释放的块应重新可分配");
}

// D-01：FIND_BEST 记录错误前驱 -> 链表自环 -> 死循环 + 重复派发
// 注意：未修复版本会在第二次 Allocate 处无限循环，外部需加超时
static void CaseFindBest() {
    std::printf("--- D-01 FreeListAllocator FIND_BEST ---\n");
    FreeListAllocator a(8192, FreeListAllocator::PlacementPolicy::FIND_BEST);
    a.Init();
    std::vector<void*> v;
    for (int i = 0; i < 3; ++i) {
        v.push_back(a.Allocate(128, 8));
        std::printf("     alloc[%d]=%p\n", i, v.back());
        std::fflush(stdout);
    }
    const bool distinct = v[0] != v[1] && v[1] != v[2];
    Check(distinct, "D-01a", "FIND_BEST 连续分配不得返回重叠地址");
    a.Free(v[2]);
    a.Free(v[0]);
    void* q = a.Allocate(64, 8);
    std::printf("     q=%p (v0=%p v2=%p)\n", q, v[0], v[2]);
    Check(q == v[0], "D-01b", "FIND_BEST 应优先复用最小满足空洞 v[0]");
}

// D-03：SinglyLinkedList 构造后 head 未初始化
static void CaseList() {
    std::printf("--- D-03 SinglyLinkedList head ---\n");
    SinglyLinkedList<int> list;
    std::printf("     head=%p（期望 nullptr）\n", (void*)list.head);
    Check(list.head == nullptr, "D-03", "构造后 head 必须为 nullptr");
}

int main(int argc, char** argv) {
    const std::string mode = (argc > 1) ? argv[1] : "all";
    if (mode == "all" || mode == "linear")   CaseLinear();
    if (mode == "all" || mode == "stack")    CaseStack();
    if (mode == "all" || mode == "pool")     CasePool();
    if (mode == "all" || mode == "lost")     CaseFreeListLost();
    if (mode == "all" || mode == "findbest") CaseFindBest();
    if (mode == "all" || mode == "list")     CaseList();
    std::printf("\n失败项: %d\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
