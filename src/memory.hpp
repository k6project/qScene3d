#pragma once

#ifdef USE_QT_INTTYPES
#include <QtGlobal>
typedef quint32 memsize_t;
typedef quint8  membyte_t;
#else
#include <cstdint>
typedef uint32_t memsize_t;
typedef uint8_t  membyte_t;
#endif

class MemBlock
{
public:
    void init(memsize_t capacity, memsize_t alignment = 16);
    memsize_t alloc(memsize_t bytes);
    void* allocPtr(memsize_t bytes, void* base);
    template <typename T> T* allocPtr(void* base) { return static_cast<T*>(allocPtr(sizeof(T), base)); }
    void free(memsize_t offset);
private:
    memsize_t max = 0, current = 0;
    memsize_t align;
};
