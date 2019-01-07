#pragma once

#include <QtGlobal>
#include <QVector>
#include <QLibrary>


#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

struct QVkBuffer
{
    enum class ContentType { GEOMETRY, PARAMETER };
    enum class AccessType { GPU_ONLY, GPU_AND_CPU };
    VkBuffer buffer;
    VkDeviceMemory memory;
    quint32 offset, size;
    void* ptr;
};

class QVkInstance
{
public:
    void create();
    void destroy();
    void adapters(QVector<VkPhysicalDevice>& list);
    void adapterInfo(VkPhysicalDevice adapter,
                     VkPhysicalDeviceProperties& properties,
                     VkPhysicalDeviceFeatures& features,
                     VkPhysicalDeviceMemoryProperties& memoryProperties);
protected:
    #define VULKAN_API_GOBAL(proc) PFN_vk ## proc vk ## proc = nullptr;
    #define VULKAN_API_INSTANCE(proc) PFN_vk ## proc vk ## proc = nullptr;
    #include "qvulkan.inl"
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    QVector<const char*> layers, extensions;
    VkInstance instance;
    QLibrary library;
};

class QVkDevice
{
public:
    enum class Type { UNIVERSAL, GRAPHICS, COMPUTE };
    void create(QVkInstance& instance, Type type = QVkDevice::Type::UNIVERSAL);
    void createBuffer(QVkBuffer::ContentType contentType, QVkBuffer::AccessType accessType, quint32 size);
    void mapBuffer(QVkBuffer& buffer) const;
    void destroy();
protected:
    bool selectAdapter(QVkInstance& instance, Type type);
    //memory allocator for vertex/index
    //memory allocator for uniform
    //memory allocator for textures
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    VkPhysicalDevice adapter;
    VkDevice device;
};
