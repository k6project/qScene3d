#include "qvulkan.hpp"

#include <QMessageBox>

#define ALIGN_(v,b) ((v)?(((v-1)/b+1)*b):(v))
#define ALIGN_256(v) ALIGN_(v,256)

struct QVkQueueLayout
{
    quint32 numFamilies;
    quint32 family[3];
    quint32 count[3];
    float priority[3];
    QPair<quint32, quint32> queues[3];
    bool isSingleQueue() const
    {
        return (numFamilies == 1 && count[0] == 1);
    }
};

static const QVkQueueLayout gDefaultQueueLayout
{
    1, {0, 0, 0}, {1, 0, 0}, {1.f, 0.f, 0.f}, {{0, 0}, {0, 0}, {0, 0}}
};

void QVkInstance::create()
{
    if (layers_.isEmpty())
    {
        layers_.append("VK_LAYER_LUNARG_standard_validation");
    }
    if (extensions_.empty())
    {
        extensions_.append("VK_EXT_debug_report");
        extensions_.append("VK_KHR_win32_surface");
        extensions_.append("VK_KHR_surface");
    }
    if (!library.isLoaded() || !instance)
    {
        if (!library.isLoaded())
        {
            library.setFileName("vulkan-1");
            Q_ASSERT(library.load());
            vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(library.resolve("vkGetInstanceProcAddr"));
            #define VULKAN_API_GOBAL(proc) vk##proc = reinterpret_cast<PFN_vk##proc>(vkGetInstanceProcAddr( nullptr, "vk" #proc )); Q_ASSERT( vk##proc );
            #include "qvulkan.inl"
        }
        VkApplicationInfo appInfo = {};
        appInfo.pApplicationName = "QScene3D Application";
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pEngineName = "qScene3d";
        appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);
        VkInstanceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &appInfo;
        info.ppEnabledLayerNames = layers_.data();
        info.enabledLayerCount = static_cast<quint32>(layers_.size());
        info.ppEnabledExtensionNames = extensions_.data();
        info.enabledExtensionCount = static_cast<quint32>(extensions_.size());
        if (vkCreateInstance(&info, nullptr, &instance) == VK_SUCCESS)
        {
            #define VULKAN_API_INSTANCE(proc) vk##proc = reinterpret_cast<PFN_vk##proc>(vkGetInstanceProcAddr( instance, "vk" #proc )); Q_ASSERT( vk##proc );
            #include "qvulkan.inl"
        }
        else
        {
            QMessageBox::critical(nullptr, "Vulkan error", "Failed to create Vulkan instance");
        }
    }
}

void QVkInstance::destroy()
{
    if (surface_)
    {
        vkDestroySurfaceKHR(instance, surface_, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
    library.unload();
}

void QVkInstance::setDisplayWidget(QWidget *widget)
{
    Q_ASSERT(instance);
    VkWin32SurfaceCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    info.hwnd =  reinterpret_cast<HWND>(widget->winId());
    info.hinstance = GetModuleHandle(nullptr);
    if (vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface_) != VK_SUCCESS)
    {
        QMessageBox::critical(nullptr, "Vulkan error", "Failed to create Vulkan surface");
    }
}

void QVkInstance::adapters(QVector<VkPhysicalDevice> &list) const
{
    quint32 count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    list.resize(static_cast<int>(count));
    vkEnumeratePhysicalDevices(instance, &count, list.data());
}

void QVkInstance::adapterInfo(VkPhysicalDevice adapter,
                              VkPhysicalDeviceProperties &properties,
                              VkPhysicalDeviceFeatures& features,
                              VkPhysicalDeviceMemoryProperties& memoryProperties) const
{
    vkGetPhysicalDeviceProperties(adapter, &properties);
    vkGetPhysicalDeviceFeatures(adapter, &features);
    vkGetPhysicalDeviceMemoryProperties(adapter, &memoryProperties);
}

bool QVkInstance::canPresent(VkPhysicalDevice adapter, quint32 queueFamily) const
{
    quint32 count = 0;
    VkBool32 retval = VK_FALSE;
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &count, nullptr);
    if (count > 0)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(adapter, queueFamily, surface_, &retval);
    }
    return (retval == VK_TRUE);
}

void QVkInstance::createDevice(QVkDevice &device, QVkDeviceType type) const
{
    if (!device.selectAdapter(*this, type))
    {
        QMessageBox::critical(nullptr, "Vulkan error", "No compatible graphics adapters installed");
    }
    else
    {
        quint32 queueOffs = 0, count = 0;
        const QVkQueueLayout& queueLayout = device.queueLayout();
        vkGetPhysicalDeviceQueueFamilyProperties(device.adapter_, &count, nullptr);
        QVarLengthArray<VkQueueFamilyProperties, 8> props(static_cast<int>(count > 8 ? 8 : count));
        vkGetPhysicalDeviceQueueFamilyProperties(device.adapter_, &count, props.data());
        QVarLengthArray<VkDeviceQueueCreateInfo, 3> queueInfo(static_cast<int>(queueLayout.numFamilies));
        for (int i = 0; i < queueInfo.size(); i++)
        {
            queueInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo[i].pNext = nullptr;
            queueInfo[i].flags = 0;
            queueInfo[i].queueFamilyIndex = queueLayout.family[i];
            queueInfo[i].queueCount = queueLayout.count[i];
            queueInfo[i].pQueuePriorities = queueLayout.priority + queueOffs;
            queueOffs += queueLayout.count[i];
        }
        VkDeviceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.queueCreateInfoCount = queueLayout.numFamilies;
        info.pQueueCreateInfos = queueInfo.data();
        if (type != QVkDeviceType::COMPUTE)
        {
            device.extensions_.append("VK_KHR_swapchain");
        }
        info.enabledExtensionCount = static_cast<quint32>(device.extensions_.size());
        info.ppEnabledExtensionNames = device.extensions_.data();
        if (vkCreateDevice(device.adapter_, &info, nullptr, &device.device_) != VK_SUCCESS)
        {
            QMessageBox::critical(nullptr, "Vulkan error", "Failed to create Vulkan device");
            return;
        }
        #define VULKAN_API_DEVICE(proc) device.vk##proc=reinterpret_cast<PFN_vk##proc>(vkGetDeviceProcAddr(device.device_,"vk" #proc));Q_ASSERT(device.vk##proc);
        #include "qvulkan.inl"
    }
}

const QVector<const char *> &QVkInstance::extensions() const
{
    return extensions_;
}

const QVector<const char *> &QVkInstance::layers() const
{
    return layers_;
}

void QVkDevice::viewResized(int width, int height) const
{
    Q_ASSERT(!swapchain_);
}

void QVkDevice::createBuffer(QVkBuffer& buffer, QVkBufferType contentType, QVkBufferAccess accessType, quint32 size)
{
    size = ALIGN_256(size);
    VkBufferCreateInfo info = {};
    VkMemoryPropertyFlags pFlags = 0;
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.sharingMode = (queueLayout().isSingleQueue()) ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount = queueLayout().numFamilies;
    info.pQueueFamilyIndices = queueLayout().family;
    switch (contentType)
    {
        case QVkBufferType::GEOMETRY:
            info.usage |= (VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
            break;
        case QVkBufferType::PARAMETER:
            info.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            break;
    }
    switch (accessType)
    {
        case QVkBufferAccess::GPU_ONLY:
            info.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            pFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
        case QVkBufferAccess::GPU_AND_CPU:
            pFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            break;
    }
    info.size = size;
    if (vkCreateBuffer(device_, &info, nullptr, &buffer.buffer_) == VK_SUCCESS)
    {
        buffer.memory_ = nullptr;
        buffer.size_ = size;
        buffer.offset_ = 0;
        buffer.ptr_ = nullptr;
        VkMemoryRequirements memReqs = {};
        vkGetBufferMemoryRequirements(device_, buffer.buffer_, &memReqs);
        for (quint32 i = 0; i < memoryProperties_.memoryTypeCount; i++)
        {
            if ((memReqs.memoryTypeBits & (1 << i)) && ((memoryProperties_.memoryTypes[i].propertyFlags & pFlags) == pFlags))
            {
                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memReqs.size;
                allocInfo.memoryTypeIndex = i;
                if (vkAllocateMemory(device_, &allocInfo, nullptr, &buffer.memory_) == VK_SUCCESS)
                {
                    vkBindBufferMemory(device_, buffer.buffer_, buffer.memory_, 0);
                }
                break;
            }
        }
        Q_ASSERT(buffer.memory_);
    }
}

void QVkDevice::mapBuffer(QVkBuffer &buffer) const
{
    if (!buffer.ptr_)
    {
        VkDeviceMemory mem = buffer.memory_;
        if (!buffer.memory_)
        {
            Q_ASSERT(buffer.parent_->ptr_ == nullptr);
            mem = buffer.parent_->memory_;
        }
        if (vkMapMemory(device_, mem, buffer.offset_, buffer.size_, 0, &buffer.ptr_) == VK_SUCCESS)
        {
            if (!buffer.memory_)
            {
                buffer.parent_->ptr_ = buffer.ptr_;
            }
        }
        else
        {
            QMessageBox::critical(nullptr, "Vulkan error", "Failed to create map buffer");
        }
    }
}

void QVkDevice::unmapBuffer(QVkBuffer &buffer) const
{
    if (buffer.ptr_)
    {
        VkDeviceMemory mem = (buffer.memory_) ? buffer.memory_ : buffer.parent_->memory_;
        vkUnmapMemory(device_, mem);
        buffer.ptr_ = nullptr;
        if (!buffer.memory_)
        {
            buffer.parent_->ptr_ = nullptr;
        }
    }
}

void QVkDevice::destroyBuffer(QVkBuffer &buffer) const
{
    unmapBuffer(buffer);
    if (buffer.memory_)
    {
        vkDestroyBuffer(device_, buffer.buffer_, nullptr);
        vkFreeMemory(device_, buffer.memory_, nullptr);
        buffer.memory_ = nullptr;
        buffer.buffer_ = nullptr;
    }
    buffer.size_ = 0;
    buffer.offset_ = 0;
}

void QVkDevice::destroy()
{
    vkDestroyDevice(device_, nullptr);
}

const VkPhysicalDeviceProperties &QVkDevice::properties() const
{
    return properties_;
}

const QVector<const char *> &QVkDevice::extensions() const
{
    return extensions_;
}

bool QVkDevice::selectAdapter(const QVkInstance &instance, QVkDeviceType type)
{
    VkPhysicalDevice discrete = nullptr;
    VkPhysicalDevice integrated = nullptr;
    QVector<VkPhysicalDevice> adapters;
    instance.adapters(adapters);
    if (adapters.isEmpty())
    {
        QMessageBox::critical(nullptr, "Vulkan error", "No compatible graphics adapters installed");
    }
    else for (VkPhysicalDevice adapter : adapters)
    {
        instance.adapterInfo(adapter, properties_, features_, memoryProperties_);
        if (type == QVkDeviceType::GRAPHICS || type == QVkDeviceType::UNIVERSAL)
        {
            quint32 queueFamily = queueLayout().queues[0].first;
            if (!instance.canPresent(adapter, queueFamily))
            {
                continue;
            }
        }
        if (properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            if (!discrete)
            {
                discrete = adapter;
                integrated = nullptr;
                break;
            }
        }
        else if (properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            if (!integrated)
            {
                integrated = adapter;
            }
        }
    }
    adapter_ = (discrete) ? discrete : integrated;
    return (adapter_ != nullptr);
}

const QVkQueueLayout &QVkDevice::queueLayout() const
{
    return gDefaultQueueLayout;
}

void *QVkBuffer::ptr()
{
    return ptr_;
}
