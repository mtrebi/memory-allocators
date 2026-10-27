// 第六轮探针：逐一修复阶段的补充用例
//
//   doubly          D-48 / D-49  DoublyLinkedList 的两个隐患（此前从未被编译过）
//   callocator      D-50         CAllocator 在 ALLOCATOR_VALIDATE 下的误用检测
//   pool_capacity                Pool 容量守恒（穿插分配释放后不丢 chunk）
//   all             以上全部
//
// 构建：需要 -DALLOCATOR_VALIDATE（callocator 用例依赖它；缺了会打印 SKIP）
#include <cstdio>
#include <cstddef>
#include <cstring>
#include <vector>

#include "DoublyLinkedList.h"
#include "CAllocator.h"
#include "PoolAllocator.h"

static int g_fail = 0;

static void verdict(const char* id, bool ok, const char* what) {
    std::printf("%s %-14s %s\n", ok ? "[ OK ]" : "[FAIL]", id, what);
    if (!ok) ++g_fail;
}

// --------------------------------------------------------------- DoublyLinkedList
typedef DoublyLinkedList<int>::Node IntNode;   // 节点布局：{int data; Node* previous; Node* next;}

// 正向遍历收集 data；反向从尾节点走（用 previous 指针），两条路必须互为逆序。
// D-49 的破坏方式恰恰是 previous 指针没被正确设置，所以反向遍历是关键检查。
static void CollectForward(IntNode* head, std::vector<int>& out) {
    out.clear();
    for (IntNode* p = head; p != nullptr; p = p->next) out.push_back(p->data);
}
static void CollectBackward(IntNode* tail, std::vector<int>& out) {
    out.clear();
    for (IntNode* p = tail; p != nullptr; p = p->previous) out.push_back(p->data);
}

static void CaseDoubly() {
    std::printf("--- D-48/D-49 DoublyLinkedList ---\n");
    DoublyLinkedList<int> list;

    // D-48: 构造后 head 必须为 nullptr（原实现构造函数是空函数体）
    verdict("D-48a", list.head == nullptr, "构造后 head 为 nullptr");

    IntNode nodes[6];
    for (int i = 0; i < 6; ++i) {
        nodes[i].data = i;
        nodes[i].previous = nullptr;
        nodes[i].next = nullptr;
    }

    // 尾插 0..4（每次都以当前尾为 previousNode）—— 这正是 D-49 原实现的死角
    for (int i = 0; i < 5; ++i) list.insert(i ? &nodes[i - 1] : nullptr, &nodes[i]);
    std::vector<int> fwd, bwd;
    CollectForward(list.head, fwd);
    std::printf("     正向: ");
    for (std::size_t i = 0; i < fwd.size(); ++i) std::printf("%d ", fwd[i]);
    std::printf("\n");
    verdict("D-49a", fwd == std::vector<int>({0, 1, 2, 3, 4}),
            "连续尾插得到 0..4 的正向序列");

    // 反向遍历：previous 指针必须全部正确（D-49 原实现在这里断）
    CollectBackward(&nodes[4], bwd);
    verdict("D-49b", bwd == std::vector<int>({4, 3, 2, 1, 0}),
            "反向遍历得到 4..0，previous 指针全部正确");

    // 摘中间、摘头、摘尾，之后两个方向的遍历都必须仍然完整一致
    list.remove(&nodes[2]);   // 中间
    list.remove(&nodes[0]);   // 头
    list.remove(&nodes[4]);   // 尾
    CollectForward(list.head, fwd);
    CollectBackward(&nodes[3], bwd);   // 现在的尾是 3
    std::printf("     摘 2/0/4 后正向: ");
    for (std::size_t i = 0; i < fwd.size(); ++i) std::printf("%d ", fwd[i]);
    std::printf("\n");
    verdict("D-49c", fwd == std::vector<int>({1, 3}),
            "摘中间/头/尾后剩余 1,3");
    verdict("D-49d", bwd == std::vector<int>({3, 1}),
            "同一次摘除后反向遍历一致");

    // 头插：previousNode 为 nullptr 的分支
    list.insert(nullptr, &nodes[2]);   // 头插 2 -> {2,1,3}
    CollectForward(list.head, fwd);
    verdict("D-49e", fwd == std::vector<int>({2, 1, 3}),
            "头插后得到 2,1,3");

    // 重复 remove 同一节点：不得破坏链表（实现里对孤立节点有防御）
    list.remove(&nodes[2]);
    const std::size_t before = fwd.size();
    list.remove(&nodes[2]);
    CollectForward(list.head, fwd);
    verdict("D-49f", fwd.size() == before - 1 && fwd[0] == 1,
            "重复 remove 孤立节点不影响链表其余部分");

    // 摘到只剩一个再摘：head 必须回到 nullptr
    list.remove(&nodes[1]);
    list.remove(&nodes[3]);
    verdict("D-49g", list.head == nullptr, "全部摘除后 head 回到 nullptr");
    std::fflush(stdout);
}

// --------------------------------------------------------------- CAllocator
static void CaseCAllocator() {
#ifdef ALLOCATOR_VALIDATE
    std::printf("--- D-50 CAllocator 误用检测（VALIDATE 构建）---\n");
    CAllocator c;
    c.Init();

    // 口径不随构建开关变化：这是 D-50 第一次尝试（往块头塞 magic）翻车的地方
    void* p = c.Allocate(1024, 8);
    const std::size_t used = c.Used();
    std::printf("     Allocate(1024,8) -> Used=%zu（期望 1048 = 1024+16+8）\n", used);
    verdict("D-50a", used == 1048, "块头几何不随 VALIDATE 变化，口径仍为 1048");

    c.Free(p);
    verdict("D-50b", c.RejectedFrees() == 0 && c.Used() == 0,
            "正常释放被放行，用量归零");

    c.Free(p);   // 重复释放：原实现直接交给 free()，堆状态被静默破坏
    std::printf("     重复 Free 后 Rejected=%zu Used=%zu\n",
                c.RejectedFrees(), c.Used());
    verdict("D-50c", c.RejectedFrees() == 1, "重复释放被拒绝并计数");

    int stackVar = 0;
    c.Free(&stackVar);   // 外部指针
    verdict("D-50d", c.RejectedFrees() == 2, "外部指针被拒绝");

    // 登记表要能正常消化多次分配/释放（顺序无关的摘除）
    void* q[8];
    for (int i = 0; i < 8; ++i) q[i] = c.Allocate(64, 8);
    const std::size_t used8 = c.Used();
    c.Free(q[3]);
    c.Free(q[0]);
    c.Free(q[7]);
    verdict("D-50e", c.Used() == used8 - 3 * (64 + 16 + 8),
            "乱序释放三次后账目精确");
    for (int i = 0; i < 8; ++i) {
        if (i != 0 && i != 3 && i != 7) c.Free(q[i]);
    }
    verdict("D-50f", c.Used() == 0 && c.RejectedFrees() == 2,
            "其余块释放后归零，拒绝计数不被误增");
#else
    std::printf("--- D-50 CAllocator 误用检测 ---\n");
    std::printf("     [SKIP] 本用例需要 -DALLOCATOR_VALIDATE 构建\n");
#endif
    std::fflush(stdout);
}

// --------------------------------------------------------------- Pool 容量守恒
static void CasePoolCapacity() {
    std::printf("--- Pool 容量守恒 ---\n");
    const std::size_t chunks = 32;
    PoolAllocator p(chunks * 64, 64);
    p.Init();
    std::vector<void*> live;

    bool allRoundsFull = true;
    for (int round = 0; round < 200; ++round) {
        // 全部分配
        for (std::size_t i = 0; i < chunks; ++i) {
            void* q = p.Allocate(64, 8);
            if (q == nullptr) { allRoundsFull = false; break; }
            live.push_back(q);
        }
        if (live.size() != chunks) { allRoundsFull = false; break; }
        if (p.InconsistentPops() != 0) { allRoundsFull = false; break; }
        // 全部释放（乱序，让空闲链表顺序与派发顺序不同）
        for (std::size_t i = 0; i < live.size(); ++i) p.Free(live[(i * 7) % chunks]);
        live.clear();
    }
    std::printf("     200 轮全满-全清后：LiveCount=%zu InconsistentPops=%zu\n",
                p.LiveCount(), p.InconsistentPops());
    verdict("POOL-a", allRoundsFull && live.empty(),
            "200 轮全满-全清，每轮 32 个 chunk 都能再次派发");
    verdict("POOL-b", p.LiveCount() == 0, "结束后位图归零");
    verdict("POOL-c", p.InconsistentPops() == 0, "内部状态从未不一致");
    std::fflush(stdout);
}

int main(int argc, char** argv) {
    const char* mode = (argc > 1) ? argv[1] : "all";
    if (!std::strcmp(mode, "doubly")        || !std::strcmp(mode, "all")) CaseDoubly();
    if (!std::strcmp(mode, "callocator")    || !std::strcmp(mode, "all")) CaseCAllocator();
    if (!std::strcmp(mode, "pool_capacity") || !std::strcmp(mode, "all")) CasePoolCapacity();
    std::printf("\n失败项: %d\n", g_fail);
    return g_fail == 0 ? 0 : 1;
}
