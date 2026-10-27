#include "CAllocator.h"
#include <stdlib.h>     /* malloc, free */
#include <algorithm>    /* std::max */
#include <cstddef>      /* max_align_t */
#include <cstdint>      /* uint32_t */
#include <cassert>
#if defined(_WIN32)
#include <malloc.h>     /* _aligned_malloc, _aligned_free */
#endif

// D-50: 重复释放 / 外部指针的检测，只在 ALLOCATOR_VALIDATE 构建下启用。
//
// 为什么不在所有构建里都开：本类在基准里的角色是"裸 malloc 对照组"。
// 常开检测意味着块头变大、每次 Allocate 多写字段，对照组就变成了"带插桩
// 的分配器"，横向对比的数字失去意义。只在验证构建下开启，两边的目的就
// 都能达到：基准看到纯净对照组，回归 / CI 看到会拒绝误用的版本。
//
// 为什么用"活指针登记表"而不是像 FreeList 那样在块头里塞 magic：
// magic 要多占 4~8 字节，块头 16 → 24，Used() 会随编译开关变化 —— 而
// "所有分配器的 Used() 口径一致、可横向比较"是比检测更重要的性质，
// 不能让一个调试宏把它破坏掉。登记表放在分配器对象自己的堆外存储里，
// 池内几何完全不变，两种构建下 Used() 逐字节相同。
// 代价是 Free 变成 O(活指针数)，且仅存在于验证构建 —— 可以接受。

namespace {

// 数据区之前保留的块头：
//   [-2] 原始分配地址 —— 超对齐时数据区相对原始块有偏移，释放必须交还原始地址
//   [-1] 实际向系统申请的字节数 —— 用于精确回收 Used()
//
// 关于记账口径：本分配器的 Used() 统计的是**实际向 malloc 申请的字节数**，
// 含块头与对齐余量。这与 FreeList（含块头）、Pool（含 chunk）、Linear/Stack
// （含对齐填充）保持一致 —— 全部是"从系统拿到的字节数"，可横向比较。
// 若统计用户字节，CAllocator 会显得比其它分配器省，是假的。
//
// 注意：这个布局在两种构建下**完全相同**，不随 ALLOCATOR_VALIDATE 变化。
const std::size_t kHeaderSize = sizeof(void*) + sizeof(std::size_t);

inline std::size_t AlignUp(std::size_t value, std::size_t alignment) {
    return ((value + alignment - 1) / alignment) * alignment;
}

inline bool IsPowerOfTwo(std::size_t v) {
    return v != 0 && (v & (v - 1)) == 0;
}

} // namespace

CAllocator::CAllocator()
    : Allocator(0) {

}

void CAllocator::Init() {
	// D-16: 与其它分配器保持一致 —— Init() 表示"重新开始一轮统计"。
	// 原实现为空函数体，m_peak 会在多轮基准测试之间累积，无法横向比较。
	m_used = 0;
	m_peak = 0;
	m_rejectedFrees = 0;
#ifdef ALLOCATOR_VALIDATE
	m_live.clear();
#endif
}

CAllocator::~CAllocator(){
    
}

void* CAllocator::Allocate(const std::size_t size, const std::size_t alignment) {
	// D-09: 原实现直接忽略 alignment 参数，一律调用 malloc。
	// malloc 只保证 max_align_t 对齐，对 32/64/128 字节等超对齐请求会违约，
	// 使 CAllocator 作为基准对照组时与其它分配器的对齐语义不一致。
	//
	// 注意：_aligned_malloc / posix_memalign 都要求对齐值是 2 的幂，
	// 非 2 的幂请求退回 max_align_t（这与 FreeList 不同 —— 后者用取模，
	// 任意对齐值都能满足，见 FreeListAllocator::Allocate）。
	std::size_t align = (alignment == 0) ? alignof(std::max_align_t) : alignment;
	if (!IsPowerOfTwo(align)) align = alignof(std::max_align_t);

	// 溢出保护：size 接近 SIZE_MAX 时下面的加法会回绕
	if (size > static_cast<std::size_t>(-1) - kHeaderSize - align) return nullptr;

	const std::size_t total = size + kHeaderSize + align;
	void* raw = nullptr;
#if defined(_WIN32)
	raw = _aligned_malloc(total, align);
#else
	if (align <= alignof(std::max_align_t)) {
		raw = malloc(total);
	} else if (posix_memalign(&raw, align, total) != 0) {
		raw = nullptr;
	}
#endif
	if (raw == nullptr) return nullptr;

	// 把数据区起点对齐到 align，块头紧贴其前
	char* data = (char*) AlignUp((std::size_t)raw + kHeaderSize, align);
	((void**) data)[-2] = raw;
	((std::size_t*) data)[-1] = total;
#ifdef ALLOCATOR_VALIDATE
	// D-50: 登记活指针。块头布局不变，几何与度量口径都不受影响。
	m_live.push_back(data);
#endif

	// D-16: 原实现不维护 m_used / m_peak，基准测试里 CAllocator 的
	// 峰值占用恒为 0，与其它分配器不可比。此处按实际申请字节记账。
	m_used += total;
	m_peak = std::max(m_peak, m_used);

	return data;
}

void CAllocator::Free(void* ptr) {
	if (ptr == nullptr) return;

#ifdef ALLOCATOR_VALIDATE
	// D-50: 先查登记表。不在表里 = 重复释放，或根本不是本分配器发出的指针。
	// 这比 magic 更强：magic 只能识别"本分配器发出且已释放过"的块，对
	// 外部指针只有 1/2^32 的误判率；登记表是精确匹配。
	{
		std::vector<void*>::iterator it = std::find(m_live.begin(), m_live.end(), ptr);
		if (it == m_live.end()) { ++m_rejectedFrees; return; }
		// O(1) 摘除：顺序无意义，把末尾元素挪过来再 pop
		*it = m_live.back();
		m_live.pop_back();
	}
#endif

	void* raw = ((void**) ptr)[-2];
	const std::size_t total = ((std::size_t*) ptr)[-1];

	// D-37: 防御性下溢保护，避免重复释放把 m_used 减成极大值
	m_used = (m_used >= total) ? (m_used - total) : 0;

#if defined(_WIN32)
	// 注意：_aligned_malloc 的返回值必须用 _aligned_free 释放，
	// 不能与 malloc/free 混用。
	_aligned_free(raw);
#else
	free(raw);
#endif
}
