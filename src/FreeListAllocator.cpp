#include "FreeListAllocator.h"
#include "Utils.h"  /* CalculatePaddingWithHeader */
#include <stdlib.h>     /* malloc, free */
#include <cassert>   /* assert		*/
#include <limits>  /* limits_max */
#include <algorithm>    // std::max

// D-35: _DEBUG 只由 MSVC 的 /D_DEBUG 定义，GCC/Clang 下恒不成立，
// 导致调试输出在非 MSVC 环境静默消失。改用标准的 NDEBUG 语义。
#ifdef ALLOCATOR_TRACE
#include <iostream>
#endif

FreeListAllocator::FreeListAllocator(const std::size_t totalSize, const PlacementPolicy pPolicy)
: Allocator(totalSize) {
    m_pPolicy = pPolicy;
}

void FreeListAllocator::Init() {
    if (m_start_ptr != nullptr) {
        free(m_start_ptr);
        m_start_ptr = nullptr;
    }
    m_start_ptr = malloc(m_totalSize);
    // D-17: malloc 失败时原实现继续把 nullptr 交给 Reset() 解引用。
    if (m_start_ptr == nullptr) {
        m_totalSize = 0;
        return;
    }
    this->Reset();
}

FreeListAllocator::~FreeListAllocator() {
    free(m_start_ptr);
    m_start_ptr = nullptr;
}

void* FreeListAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
    const std::size_t allocationHeaderSize = sizeof(FreeListAllocator::AllocationHeader);

    if (m_start_ptr == nullptr) return nullptr;

    // D-25: 接口默认 alignment = 0，若直接进入对齐计算会触发除零。
    const std::size_t align = (alignment < sizeof(std::size_t)) ? sizeof(std::size_t) : alignment;

    // D-36 修订：原实现要求 alignment 必须是 2 的幂。这个约束是多余的 ——
    // 本分配器的对齐计算走 Utils::CalculatePadding，用的是取模与减法，
    // 任意对齐值都能算对（实测 alignment=24 返回地址 mod 24 == 0）。
    // 真正要求 2 的幂的是 _aligned_malloc / posix_memalign，那是 CAllocator
    // 的限制，不该强加到这里。仅保留"对齐要求不得超过池容量"的合理性检查。
    if (align > m_totalSize) return nullptr;

    // D-32: 用减法形式做上界检查，避免 size + padding 整数回绕后绕过比较。
    if (size > m_totalSize) return nullptr;

    // D-51: O(1) 快速失败。池满之后每一次 Allocate 原本都要扫完整条空闲
    // 链表才能确认"装不下"，节点上千时这就是最贵的那条路径。
    if (FastReject(size)) return nullptr;

    // Search through the free list for a free block that has enough space to allocate our data
    std::size_t padding = 0;
    Node * affectedNode = nullptr,
         * previousNode = nullptr;
    this->Find(size, align, padding, previousNode, affectedNode);

    // D-04: 原实现 assert 后仍解引用 affectedNode，Release 构建直接崩溃。
    // 内存不足应以返回值表达，交由调用方处理。
    if (affectedNode == nullptr) return nullptr;

    const std::size_t alignmentPadding = padding - allocationHeaderSize;
    // D-45: padding 字段是 uint32，超过 4 GiB 的对齐无法表示。
    // align 已限制在 m_totalSize 以内，这里只是把上界说清楚。
    if (alignmentPadding > 0xFFFFFFFFu) return nullptr;

    std::size_t requiredSize = size + padding;
    std::size_t rest = affectedNode->data.blockSize - requiredSize;

    // D-26 / D-39: 余量小于一个节点时无法再挂入空闲链表。原实现直接丢弃，
    // 这段地址就成了永久空洞，使后续相邻块永远无法合并。此处把余量并入
    // 本次分配的块大小，保证空间账目守恒。
    if (rest > 0 && rest < sizeof(Node)) {
        requiredSize += rest;
        rest = 0;
    }

    // D-41（实施期新增）: 原实现在摘链之前就写入了数据块头，而块头所在地址
    // 恰好覆盖 affectedNode 自身（alignmentPadding == 0 时），导致
    // affectedNode->next 被 requiredSize 覆盖，链表断裂。顺序改为：
    // 保存后继 -> 摘链 -> 切分新空闲块 -> 最后写块头。
    Node * nextFree = affectedNode->next;
    // D-51: 摘链前记下这个块原本多大 —— 切分之后 affectedNode 的内容会被
    // 块头覆盖，那时再读就是 requiredSize 了。
    const std::size_t consumedBlockSize = affectedNode->data.blockSize;
    m_freeList.remove(previousNode, affectedNode);

    if (rest > 0) {
        // We have to split the block into the data block and a free block of size 'rest'
        Node * newFreeNode = (Node *)((std::size_t) affectedNode + requiredSize);
        newFreeNode->data.blockSize = rest;
        newFreeNode->next = nextFree;
        m_freeList.insert(previousNode, newFreeNode);
    }

    // D-51: 若本次吃掉的正好是"上界对应的那个块"，上界可能不再成立，
    // 需要重扫一次取真实最大值。其余情况上界仍然有效（只会偏高，不会偏低）。
    if (consumedBlockSize >= m_maxFreeBlock) {
        RecomputeMaxFreeBlock();
    }

    // Setup data block
    const std::size_t headerAddress = (std::size_t) affectedNode + alignmentPadding;
    const std::size_t dataAddress = headerAddress + allocationHeaderSize;
    FreeListAllocator::AllocationHeader* hdr =
        (FreeListAllocator::AllocationHeader*) headerAddress;
    hdr->blockSize = requiredSize;
    hdr->padding   = (std::uint32_t) alignmentPadding;
    // D-45: 标记为"已分配"，Free 时据此识别重复释放
    hdr->magic     = kAllocatedMagic;

    m_used += requiredSize;
    m_peak = std::max(m_peak, m_used);

#ifdef ALLOCATOR_TRACE
    std::cout << "A" << "\t@H " << (void*) headerAddress << "\tD@ " <<(void*) dataAddress << "\tS " << requiredSize <<  "\tAP " << alignmentPadding << "\tP " << padding << "\tM " << m_used << "\tR " << rest << std::endl;
#endif

    return (void*) dataAddress;
}

void FreeListAllocator::Find(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node *& previousNode, Node *& foundNode) {
    switch (m_pPolicy) {
        case FIND_FIRST:
            FindFirst(size, alignment, padding, previousNode, foundNode);
            break;
        case FIND_BEST:
            FindBest(size, alignment, padding, previousNode, foundNode);
            break;
        default:
            // 原实现无 default 分支，非法策略下 foundNode 保持未初始化。
            previousNode = nullptr;
            foundNode = nullptr;
            break;
    }
}

void FreeListAllocator::FindFirst(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node *& previousNode, Node *& foundNode) {
    //Iterate list and return the first free block with a size >= than given size
    Node * it = m_freeList.head,
         * itPrev = nullptr;
    
    while (it != nullptr) {
        const std::size_t p = Utils::CalculatePaddingWithHeader((std::size_t)it, alignment, sizeof (FreeListAllocator::AllocationHeader));
        // D-44: 原写法 "it->data.blockSize >= size + p" 中 size 与 p 都可来自
        // 调用方，相加可能整数回绕成极小值，使这个"装不下"的块被判定为命中。
        // 改用两个减法：先保证 p 本身不超块大小，再比较剩余量。
        if (p <= it->data.blockSize && size <= it->data.blockSize - p) {
            // D-08: 原实现只记录最后一次迭代的 padding，与命中节点不对应。
            padding = p;
            previousNode = itPrev;
            foundNode = it;
            return;
        }
        itPrev = it;
        it = it->next;
    }
    // 未命中时原实现让 foundNode = nullptr 但 padding 仍是最后一次
    // 迭代的残留值，调用方无从分辨。
    previousNode = nullptr;
    foundNode = nullptr;
}

void FreeListAllocator::FindBest(const std::size_t size, const std::size_t alignment, std::size_t& padding, Node *& previousNode, Node *& foundNode) {
    // Iterate WHOLE list keeping a pointer to the best fit
    std::size_t smallestDiff = std::numeric_limits<std::size_t>::max();
    Node * bestBlock = nullptr;
    Node * bestPrev = nullptr;      // D-01: 记录 bestBlock 的真实前驱
    std::size_t bestPadding = 0;

    Node * it = m_freeList.head,
         * itPrev = nullptr;
    while (it != nullptr) {
        const std::size_t p = Utils::CalculatePaddingWithHeader((std::size_t)it, alignment, sizeof (FreeListAllocator::AllocationHeader));
        // D-44: 同上，用减法避免 size + p 回绕
        if (p <= it->data.blockSize && size <= it->data.blockSize - p) {
            const std::size_t diff = it->data.blockSize - size - p;
            if (diff < smallestDiff) {
                // D-01: 原实现从未更新 smallestDiff，条件恒真，
                // 最终取到的是链尾最后一个可容纳块 —— best-fit 退化为 last-fit。
                smallestDiff = diff;
                bestBlock = it;
                // D-01: 原实现取 itPrev（遍历结束时的链尾），
                // 用它摘链会摘掉错误的节点。
                bestPrev = itPrev;
                // D-08: padding 必须与 bestBlock 配对保存。
                bestPadding = p;
                // D-51: 差为 0 就是精确命中，不可能再有更优解，可以立刻收工。
                // 这里保留"地址序上的第一个"语义：diff 为 0 的块可能不止一个，
                // 我们取遇到的第一个，与原实现的中序遍历顺序一致。
                // 对"反复分配释放同一尺寸"的工作负载，这一句把整表扫描降到 O(1)。
                if (diff == 0) break;
            }
        }
        itPrev = it;
        it = it->next;
    }
    previousNode = bestPrev;
    foundNode = bestBlock;
    if (bestBlock != nullptr) padding = bestPadding;
}

void FreeListAllocator::Free(void* ptr) {
    // 原实现对 nullptr 直接做地址运算，会读到非法块头。
    if (ptr == nullptr) return;
    if (m_start_ptr == nullptr) { ++m_rejectedFrees; return; }

    const std::size_t headerSize      = sizeof(FreeListAllocator::AllocationHeader);
    const std::size_t poolStart       = (std::size_t) m_start_ptr;
    const std::size_t poolEnd         = poolStart + m_totalSize;
    const std::size_t currentAddress  = (std::size_t) ptr;

    // D-45: 归属检查。指针必须落在池内，且前面留得下整个块头。
    // 越界指针若直接参与下面的减法，会算出池外地址并在那里写入。
    if (currentAddress < poolStart + headerSize || currentAddress >= poolEnd) {
        ++m_rejectedFrees;
        return;
    }

    const std::size_t headerAddress = currentAddress - headerSize;
    FreeListAllocator::AllocationHeader* allocationHeader =
        (FreeListAllocator::AllocationHeader*) headerAddress;

    // D-45: 重复释放检测。
    // 没有这个校验时，第二次 Free 读到的是空闲节点数据被当成 padding 的值，
    // 据此算出的 blockStart 落在池外，随后写入即段错误（实测 SIGSEGV）。
    // magic 不匹配 → 这块要么已经释放过，要么根本不是本分配器发出的指针。
    if (allocationHeader->magic != kAllocatedMagic) {
        ++m_rejectedFrees;
        return;
    }

    // D-24: 原实现 freeNode 落在块头地址而非块起点，且 blockSize 写成
    // "blockSize + padding" —— padding 在分配时已计入 blockSize，这里重复累加。
    // 双重错误叠加，导致空闲块覆盖范围错位、相邻块被误判为不连续。
    const std::size_t blockSize  = allocationHeader->blockSize;
    const std::size_t blockStart = headerAddress - allocationHeader->padding;

    // 块起点同样要在池内（padding 是分配时写下的，但仍需防御被踩坏的块头）
    if (blockStart < poolStart || blockStart + blockSize > poolEnd || blockSize < sizeof(Node)) {
        ++m_rejectedFrees;
        return;
    }

    // 关键：先清掉 magic，再写空闲节点。
    // 如果只依赖"空闲节点的 next 会覆盖 magic"，那么当 alignmentPadding >= 16 时
    // 块头区域根本不会被覆盖，重复释放就识别不出来了。
    allocationHeader->magic = 0;

    Node * freeNode = (Node *) blockStart;
    freeNode->data.blockSize = blockSize;
    freeNode->next = nullptr;

    // D-04: 原实现把插入动作放在 while 循环体内，链表为空或新块地址大于
    // 所有现有节点时循环走完却没有插入，这块内存彻底丢失。
    Node * itPrev = nullptr;
    Node * it = m_freeList.head;
    while (it != nullptr && (std::size_t) it < blockStart) {
        itPrev = it;
        it = it->next;
    }
    m_freeList.insert(itPrev, freeNode);

    // D-37: 防御性下溢保护
    m_used = (m_used >= freeNode->data.blockSize) ? (m_used - freeNode->data.blockSize) : 0;

    // Merge contiguous nodes
    Node* survivor = Coalescence(itPrev, freeNode);

    // D-51: 一次 Free 只会让"存活下来的那个块"变大，所以上界只需向上修正。
    // 这样它永远是安全上界（绝不低估），FastReject 不会误拒。
    if (survivor->data.blockSize > m_maxFreeBlock) {
        m_maxFreeBlock = survivor->data.blockSize;
    }

#ifdef ALLOCATOR_TRACE
    std::cout << "F" << "\t@ptr " <<  ptr <<"\tH@ " << (void*) freeNode << "\tS " << freeNode->data.blockSize << "\tM " << m_used << std::endl;
#endif
}

FreeListAllocator::Node* FreeListAllocator::Coalescence(Node* previousNode, Node * freeNode) {   
    if (freeNode->next != nullptr && 
            (std::size_t) freeNode + freeNode->data.blockSize == (std::size_t) freeNode->next) {
        // 先保存被吞并的节点地址：remove() 会改写 freeNode->next，
        // 之后再打印会输出 nullptr（原实现的日志就存在这个误导）。
        Node * absorbed = freeNode->next;
        freeNode->data.blockSize += absorbed->data.blockSize;
        m_freeList.remove(freeNode, absorbed);
#ifdef ALLOCATOR_TRACE
    std::cout << "\tMerging(n) " << (void*) freeNode << " & " << (void*) absorbed << "\tS " << freeNode->data.blockSize << std::endl;
#endif
    }
    
    if (previousNode != nullptr &&
            (std::size_t) previousNode + previousNode->data.blockSize == (std::size_t) freeNode) {
        previousNode->data.blockSize += freeNode->data.blockSize;
        m_freeList.remove(previousNode, freeNode);
#ifdef ALLOCATOR_TRACE
    std::cout << "\tMerging(p) " << (void*) previousNode << " & " << (void*) freeNode << "\tS " << previousNode->data.blockSize << std::endl;
#endif
        // D-51: 返回真正留在链表里的那个节点。被向后吞并后 freeNode 已不在链表中，
        // 用它的 blockSize 去更新"最大空闲块"上界会读到过期值。
        return previousNode;
    }
    return freeNode;
}

void FreeListAllocator::RecomputeMaxFreeBlock() {
    // O(空闲块数)。只在"当前上界对应的块被消耗掉"时调用，不是热路径。
    std::size_t m = 0;
    for (Node* it = m_freeList.head; it != nullptr; it = it->next) {
        if (it->data.blockSize > m) m = it->data.blockSize;
    }
    m_maxFreeBlock = m;
}

#ifdef ALLOCATOR_VALIDATE
#include <algorithm>

namespace {
struct Range {
    std::size_t b, e;
    bool isFree;
    bool operator<(const Range& o) const { return b < o.b; }
};
}

std::size_t FreeListAllocator::FreeBlockCount() const {
    std::size_t n = 0;
    for (Node* it = m_freeList.head; it != nullptr; it = it->next) ++n;
    return n;
}

std::size_t FreeListAllocator::LargestFreeBlock() const {
    std::size_t m = 0;
    for (Node* it = m_freeList.head; it != nullptr; it = it->next) {
        if (it->data.blockSize > m) m = it->data.blockSize;
    }
    return m;
}

bool FreeListAllocator::Validate(const std::vector<void*>& livePtrs, std::string* err) const {
    const std::size_t start = (std::size_t)m_start_ptr;
    const std::size_t end   = start + m_totalSize;
    std::vector<Range> rs;

    std::size_t prev = 0;
    bool first = true;
    for (Node* it = m_freeList.head; it != nullptr; it = it->next) {
        const std::size_t a = (std::size_t)it;
        if (a < start || a >= end)          { if (err) *err = "空闲节点越出池范围"; return false; }
        if (!first && a <= prev)            { if (err) *err = "空闲链表未按地址严格递增"; return false; }
        const std::size_t sz = it->data.blockSize;
        if (sz < sizeof(Node))              { if (err) *err = "空闲块小于节点尺寸"; return false; }
        if (a + sz > end)                   { if (err) *err = "空闲块越出池尾"; return false; }
        rs.push_back(Range{a, a + sz, true});
        prev = a;
        first = false;
    }

    std::size_t liveSum = 0;
    for (std::size_t i = 0; i < livePtrs.size(); ++i) {
        void* p = livePtrs[i];
        if (p == nullptr) continue;
        const std::size_t d = (std::size_t)p;
        if (d < start + sizeof(AllocationHeader) || d >= end) { if (err) *err = "活动指针越界"; return false; }
        const AllocationHeader* h = (const AllocationHeader*)(d - sizeof(AllocationHeader));
        // D-45 延伸到校验器：活动指针的块头 magic 必须仍然有效。
        // 若测试记录里混入了已释放的指针，这里会直接指出来 —— 否则下面
        // 读到的 blockSize 是空闲节点数据，算出的区间毫无意义，校验形同虚设。
        if (h->magic != kAllocatedMagic) { if (err) { *err = "活动指针的块头 magic 失效（已释放或损坏）"; } return false; }
        const std::size_t b = d - sizeof(AllocationHeader) - h->padding;
        if (b < start || b >= end || b + h->blockSize > end) { if (err) *err = "活动块越界"; return false; }
        if (h->blockSize < sizeof(Node)) { if (err) *err = "活动块尺寸异常"; return false; }
        rs.push_back(Range{b, b + h->blockSize, false});
        liveSum += h->blockSize;
    }

    std::sort(rs.begin(), rs.end());
    for (std::size_t i = 1; i < rs.size(); ++i) {
        if (rs[i].b < rs[i - 1].e) { if (err) *err = "区间重叠"; return false; }
        if (rs[i].isFree && rs[i - 1].isFree && rs[i - 1].e == rs[i].b) {
            if (err) { *err = "相邻空闲块未合并"; }
            return false;
        }
    }
    if (liveSum != m_used) { if (err) *err = "m_used 与活动块总和不符"; return false; }
    return true;
}
#endif

void FreeListAllocator::Reset() {
    m_used = 0;
    m_peak = 0;
    m_rejectedFrees = 0;
    m_freeList.head = nullptr;
    // D-17: 分配失败后 Reset 仍会被调用，此处需防御空指针。
    if (m_start_ptr == nullptr) return;
    Node * firstNode = (Node *) m_start_ptr;
    firstNode->data.blockSize = m_totalSize;
    firstNode->next = nullptr;
    m_freeList.insert(nullptr, firstNode);
    // D-51: 池刚初始化时整块都是空闲的
    m_maxFreeBlock = m_totalSize;
}
