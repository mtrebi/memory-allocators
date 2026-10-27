#ifndef LINEARALLOCATOR_H
#define LINEARALLOCATOR_H

#include "Allocator.h"

class LinearAllocator : public Allocator {
protected:
	void* m_start_ptr = nullptr;
	// D-40（实施期新增）: 原实现未初始化 m_offset，构造函数里也未赋值，
	// 首次 Allocate 会以不确定的偏移量做地址运算。
	std::size_t m_offset = 0;
public:
	LinearAllocator(const std::size_t totalSize);

	virtual ~LinearAllocator();

	virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) override;
	
	virtual void Free(void* ptr) override;

	virtual void Init() override;

	virtual void Reset();

	// D-31: 线性分配器只支持整体 Reset，不支持单次释放
	virtual bool IsFreeable() const override { return false; }
private:
	// D-34: 禁用拷贝
	LinearAllocator(LinearAllocator &linearAllocator) = delete;
	LinearAllocator& operator=(LinearAllocator &linearAllocator) = delete;
};

#endif /* LINEARALLOCATOR_H */
