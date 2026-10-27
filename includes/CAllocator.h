#ifndef CALLOCATOR_H
#define CALLOCATOR_H

#include "Allocator.h"

#ifdef ALLOCATOR_VALIDATE
#include <vector>
#endif

class CAllocator : public Allocator {
public:
	CAllocator();

	virtual ~CAllocator();

	virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;

	// 关于检测：本函数在**基准构建**下不做任何指针归属或重复释放校验，
	// 直接把块头里记录的原始地址交给 free() / _aligned_free()。
	//
	// 为什么不常开检测：CAllocator 在基准里的角色是"裸 malloc 对照组"。
	// 要检测的两种路子里，塞 magic 会让块头从 16 字节涨到 24 字节，对照组
	// 立刻变成"带插桩的分配器"，横向对比的数据失去意义；而外部登记表
	// 等于每次分配多一次 malloc。两条都不利于对照组定位。
	//
	// D-50：在 `-DALLOCATOR_VALIDATE` 构建下（回归 / CI 用的就是它）会启用
	// "活指针登记表"检测，重复释放与外部指针被精确拒绝并计入 RejectedFrees()。
	// 关键：登记表放在分配器对象自己的堆外存储里，块头几何与 Used() 口径
	// 在两种构建下**逐字节相同**，不破坏"所有分配器口径一致、可横向比较"。
	// 基准构建（不定义该宏）仍是零开销、零几何变化的纯净对照组。
	virtual void Free(void* ptr) override;

	// 被拒绝的 Free 次数。未定义 ALLOCATOR_VALIDATE 时恒为 0（不做检测）。
	std::size_t RejectedFrees() const { return m_rejectedFrees; }

	virtual void Init() override;
private:
	// D-34: 该分配器持有外部资源（malloc 得到的块），禁止拷贝
	CAllocator(CAllocator &cAllocator) = delete;
	CAllocator& operator=(CAllocator &cAllocator) = delete;

	std::size_t m_rejectedFrees = 0;

#ifdef ALLOCATOR_VALIDATE
	// D-50: 活指针登记表。仅验证构建存在；Free 时查表，命中才放行并摘除。
	std::vector<void*> m_live;
#endif
};

#endif /* CALLOCATOR_H */

