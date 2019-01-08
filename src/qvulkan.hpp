#pragma once

#include <QtGlobal>
#include <QVector>
#include <QLibrary>

class QWidget;

struct QVkQueueLayout;

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

enum class QVkDeviceType
{
    UNIVERSAL,
    GRAPHICS,
    COMPUTE
};

enum class QVkBufferType
{
    GEOMETRY,
    PARAMETER
};

enum class QVkBufferAccess
{
    GPU_ONLY,
    GPU_AND_CPU
};

class QVkBuffer
{
public:
    void* ptr();
private:
    friend class QVkDevice;
    union
    {
        VkBuffer buffer_;
        QVkBuffer* parent_;
    };
    VkDeviceMemory memory_;
    quint32 offset_, size_;
    void* ptr_;
};

class QVkInstance
{
public:
    void create();
    void destroy();
    void setDisplayWidget(QWidget* widget);
    void adapters(QVector<VkPhysicalDevice>& list) const;
    void adapterInfo(VkPhysicalDevice adapter,
                     VkPhysicalDeviceProperties& properties,
                     VkPhysicalDeviceFeatures& features,
                     VkPhysicalDeviceMemoryProperties& memoryProperties) const;
    bool canPresent(VkPhysicalDevice adapter, quint32 queueFamily) const;
    void createDevice(QVkDevice& device, QVkDeviceType type) const;
    const QVector<const char*>& extensions() const;
    const QVector<const char*>& layers() const;
protected:
    #define VULKAN_API_GOBAL(proc) PFN_vk ## proc vk ## proc = nullptr;
    #define VULKAN_API_INSTANCE(proc) PFN_vk ## proc vk ## proc = nullptr;
    #include "qvulkan.inl"
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
    QVector<const char*> layers_, extensions_;
    VkInstance instance = nullptr;
    VkSurfaceKHR surface_ = nullptr;
    QLibrary library;
};

class QVkDevice
{
public:
    void viewResized(int width, int height) const;
    void createBuffer(QVkBuffer &buffer, QVkBufferType contentType, QVkBufferAccess accessType, quint32 size);
    void mapBuffer(QVkBuffer& buffer) const;
    void unmapBuffer(QVkBuffer& buffer) const;
    void destroyBuffer(QVkBuffer& buffer) const;
    const VkPhysicalDeviceProperties& properties() const;
    const QVector<const char*>& extensions() const;
    void destroy();
protected:
    friend class QVkInstance;
    #define VULKAN_API_DEVICE(proc) PFN_vk ## proc vk ## proc = nullptr;
    #include "qvulkan.inl"
    bool selectAdapter(const QVkInstance& instance, QVkDeviceType type);
    const QVkQueueLayout& queueLayout() const;
    QVector<const char*> extensions_;
    mutable VkSwapchainKHR swapchain_ = nullptr;
    VkPhysicalDeviceProperties properties_;
    VkPhysicalDeviceFeatures features_;
    VkPhysicalDeviceMemoryProperties memoryProperties_;
    VkPhysicalDevice adapter_;
    VkDevice device_;
};
