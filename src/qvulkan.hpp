#pragma once

#include <QtGlobal>
#include <QVector>
#include <QLibrary>

class QWidget;

struct QVkQueueLayout;

#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

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

class QVkSurface
{
private:
    friend class QVkInstance;
    VkSurfaceKHR id_ = nullptr;
};

class QVkQueue
{
private:
    friend class QVkDevice;
    quint32 family_, index_;
    VkQueue id_ = nullptr;
};

class QVkSwapchain
{
private:
    friend class QVkDevice;
    VkSwapchainKHR id_;
};

/*class QVkBuffer
{
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
};*/

class QVkInstance
{
public:
    static const QVkInstance& get();
    static void destroy();
    void createSurface(QVkSurface& surface, QWidget* widget) const;
    void destroySurface(QVkSurface& surface) const;
    void createDevice(QVkDevice& device, const QVkSurface& surface, quint32 numExt = 0, const char **ext = nullptr) const;
    void destroyDevice(QVkDevice& device) const;
    const QVector<const char*>& extensions() const;
    const QVector<const char*>& layers() const;
private:
    static QVkInstance gVkInstance;
    void initInstanceFunctions();
    void initDeviceFunctions(QVkDevice& device);
    const QVkQueueLayout& queueLayout(quint32 vendorId, quint32 deviceId) const;
    #define VULKAN_API_GLOBAL(proc) PFN_vk ## proc vk ## proc = nullptr;
    #define VULKAN_API_INSTANCE(proc) PFN_vk ## proc vk ## proc = nullptr;
    #include "qvulkan.inl"
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
    QVector<const char*> layers_, extensions_;
    VkInstance id_ = nullptr;
    QLibrary library_;
};

class QVkDevice
{
public:
    /*void createBuffer(QVkBuffer &buffer, QVkBufferType contentType, QVkBufferAccess accessType, quint32 size);
    void mapBuffer(QVkBuffer& buffer) const;
    void unmapBuffer(QVkBuffer& buffer) const;
    void destroyBuffer(QVkBuffer& buffer) const;*/
    void waitIdle() const;
    const VkPhysicalDeviceProperties& properties() const;
    const QVector<const char*>& extensions() const;
protected:
    friend class QVkInstance;
    #define VULKAN_API_DEVICE(proc) PFN_vk ## proc vk ## proc = nullptr;
    #include "qvulkan.inl"
    QVector<const char*> extensions_;
    VkPhysicalDeviceProperties properties_;
    VkPhysicalDeviceFeatures features_;
    VkPhysicalDeviceMemoryProperties memoryProperties_;
    VkPhysicalDevice adapter_;
    VkDevice id_ = nullptr;
};
