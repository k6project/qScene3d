#pragma once

#include <QtGlobal>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

struct QVkBuffer
{
    enum class ContentType { GEOMETRY, PARAMETER };
    enum class AccessType { GPU_ONLY, GPU_AND_CPU };
    VkBuffer buffer;
    VkDeviceMemory memory;
    quint32 offset, size;
    void* mappedPtr;
};

class QVkInstance
{
public:
    void create();
protected:
    VkInstance instance;
};

class QVkDevice
{
public:
    void createBuffer(QVkBuffer::ContentType contentType, QVkBuffer::AccessType accessType, quint32 size);
    void mapBuffer(QVkBuffer& buffer) const;
protected:
    //memory allocator for vertex/index
    //memory allocator for uniform
    //memory allocator for textures
    VkDevice device;
};
