#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <cstddef> // size_t

class Allocator {
protected:
    std::size_t m_totalSize;
    std::size_t m_used;   
    std::size_t m_peak;
public:
    
    Allocator(const std::size_t totalSize) : m_totalSize { totalSize }, m_used { 0 }, m_peak { 0 } { }

    virtual ~Allocator() { m_totalSize = 0; }

    virtual void* Allocate(const std::size_t size, const std::size_t alignment = 0) = 0;

    virtual void Free(void* ptr) = 0;

    virtual void Init() = 0;

    // 原实现把 m_used / m_peak 完全封闭，只有 Benchmark 通过 friend
    // 才能读取，外部无法查询分配器的实际占用。补齐只读访问器。
    std::size_t TotalSize() const { return m_totalSize; }
    std::size_t Used() const { return m_used; }
    std::size_t Peak() const { return m_peak; }

    // D-31: 线性分配器不支持单次释放，Benchmark 对四种分配器一视同仁地
    // 调用 Free() 会触发断言。用该接口让调用方自行判断。
    virtual bool IsFreeable() const { return true; }

    friend class Benchmark;
};

#endif /* ALLOCATOR_H */
