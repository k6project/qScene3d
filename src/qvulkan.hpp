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
    friend class QVkDevice;
    friend class QVkInstance;
    void initSwapchainCreateInfo(VkSwapchainCreateInfoKHR& info) const;
    VkSurfaceCapabilitiesKHR capabilities_;
    QVector<VkSurfaceFormatKHR> formats_;
    QVector<VkPresentModeKHR> presentModes_;
    VkSurfaceKHR id_ = nullptr;
};

class QVkQueue
{
private:
    friend class QVkDevice;
    quint32 family_, index_;
    VkQueue id_ = nullptr;
};

class QVkImage
{
private:
    friend class QVkDevice;
    friend class QVkSwapchain;
    VkImageUsageFlagBits usage_;
    VkSampleCountFlagBits samples_;
    VkFormat format_;
    VkImageView view_;
    VkImage id_;
};

class QVkSwapchain
{
public:
    const QVkImage& current() const;
private:
    friend class QVkDevice;
    quint32 size_ = 0, current_ = 0;
    VkSwapchainKHR id_ = nullptr;
    QVector<VkImage> image_;
    QVkImage proxy_;
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

/*
device.createRenderPass(quint32 numSubpass, quint32 numColor)
    .addColorBuffer(swapchain.current())
        .attachmentOp(VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE)
        .layout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
    .addSubPass(numColor, numInput, numResolve)
        .colorAttachment()
        .inputAttachment()
        .resolveAttachment(index)
*/

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
    VkDebugReportCallbackEXT debug_ = nullptr;
    QVector<const char*> layers_, extensions_;
    VkInstance id_ = nullptr;
    QLibrary library_;
};

class QVkDevice
{
public:
    void createSwapchain(QVkSwapchain& swapchain, QVkSurface& surface);
    /*void createBuffer(QVkBuffer &buffer, QVkBufferType contentType, QVkBufferAccess accessType, quint32 size);
    void mapBuffer(QVkBuffer& buffer) const;
    void unmapBuffer(QVkBuffer& buffer) const;
    void destroyBuffer(QVkBuffer& buffer) const;*/
    bool acquireNextImage(const QVkSwapchain& swapchain) const;
    void queuePresent(const QVkSwapchain& swapchain) const;
    void waitIdle() const;
    const VkPhysicalDeviceProperties& properties() const;
    const QVector<const char*>& extensions() const;
protected:
    friend class QVkInstance;
    void setQueues(const QVkQueueLayout& qLayout);
    void setSurfaceCapabilities(QVkSurface& surface) const;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
    #define VULKAN_API_DEVICE(proc) PFN_vk ## proc vk ## proc = nullptr;
    #include "qvulkan.inl"
    QVector<const char*> extensions_;
    VkPhysicalDeviceProperties properties_;
    VkPhysicalDeviceFeatures features_;
    VkPhysicalDeviceMemoryProperties memoryProperties_;
    VkPhysicalDevice adapter_;
    VkDevice id_ = nullptr;
    QVkQueue drawQueue_;
    QVkQueue copyQueue_;
};
