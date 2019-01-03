#include "memory.hpp"

#define ALIGN(n,a) ((((n-1)/a)+1)*a)

void MemBlock::init(memsize_t capacity, memsize_t alignment)
{
    align = (alignment > 1) ? alignment : 1;
    capacity = ALIGN(capacity, align);
    if (capacity < max)
        current = capacity;
    max = capacity;
}

memsize_t MemBlock::alloc(memsize_t bytes)
{
    Q_ASSERT(bytes);
    bytes = ALIGN(bytes, align);
    memsize_t retval = current;
    Q_ASSERT((max - current) >= bytes);
    current += bytes;
    return retval;
}

void* MemBlock::allocPtr(memsize_t bytes, void* base)
{
    memsize_t offset = alloc(bytes);
    return (static_cast<membyte_t*>(base) + offset);
}

void MemBlock::free(memsize_t offset)
{
    if (offset < current)
        current = offset;
}
