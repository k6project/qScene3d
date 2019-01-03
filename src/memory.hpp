#pragma once

#include <QtGlobal>

class MemAlloc
{
public:
    //
    void* allocLinear(quint64 size);
    void* allocStack(quint64 size);
    void* allocHeap(quint64 size);
    template <typename T> T* allocLinear() { return static_cast<T*>(allocLinear(sizeof(T))); }
    template <typename T> T* allocStack() { return static_cast<T*>(allocStack(sizeof(T))); }
    template <typename T> T* allocHeap() { return static_cast<T*>(allocHeap(sizeof(T))); }
};
