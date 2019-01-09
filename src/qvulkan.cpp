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
    bool isSingleQueue() const { return (numFamilies == 1 && count[0] == 1); }
    quint32 graphicsQueueFamily() const { return family[0]; }
};

static const QVkQueueLayout gDefaultQueueLayout
{
    1, {0, 0, 0}, {1, 0, 0}, {1.f, 0.f, 0.f}, {{0, 0}, {0, 0}, {0, 0}}
};

QVkInstance QVkInstance::gVkInstance;

const QVkInstance& QVkInstance::get()
{
    if (!gVkInstance.id_)
    {
        if (gVkInstance.layers_.isEmpty())
        {
            gVkInstance.layers_.append("VK_LAYER_LUNARG_standard_validation");
        }
        if (gVkInstance.extensions_.empty())
        {
            gVkInstance.extensions_.append("VK_EXT_debug_report");
            gVkInstance.extensions_.append("VK_KHR_win32_surface");
            gVkInstance.extensions_.append("VK_KHR_surface");
        }
        if (!gVkInstance.library_.isLoaded())
        {
            gVkInstance.library_.setFileName("vulkan-1");
            Q_ASSERT(gVkInstance.library_.load());
            gVkInstance.initInstanceFunctions();
        }
        VkApplicationInfo appInfo = {};
        appInfo.pApplicationName = "QScene3D Application";
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pEngineName = "qScene3d";
        appInfo.apiVersion = VK_MAKE_VERSION(1, 1, 0);
        VkInstanceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &appInfo;
        info.ppEnabledLayerNames = gVkInstance.layers_.data();
        info.enabledLayerCount = static_cast<quint32>(gVkInstance.layers_.size());
        info.ppEnabledExtensionNames = gVkInstance.extensions_.data();
        info.enabledExtensionCount = static_cast<quint32>(gVkInstance.extensions_.size());
        if (gVkInstance.vkCreateInstance(&info, nullptr, &gVkInstance.id_) == VK_SUCCESS)
        {
            gVkInstance.initInstanceFunctions();
        }
        else
        {
            QMessageBox::critical(nullptr, "Vulkan error", "Failed to create Vulkan instance");
        }
    }
    return gVkInstance;
}

void QVkInstance::destroy()
{
    if (gVkInstance.id_)
    {
        gVkInstance.vkDestroyInstance(gVkInstance.id_, nullptr);
        gVkInstance.id_ = nullptr;
    }
    gVkInstance.library_.unload();
}

void QVkInstance::createSurface(QVkSurface& surface, QWidget* widget) const
{
    Q_ASSERT(id_);
#ifdef WIN32
    VkWin32SurfaceCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    info.hwnd =  reinterpret_cast<HWND>(widget->winId());
    info.hinstance = GetModuleHandle(nullptr);
    if (vkCreateWin32SurfaceKHR(id_, &info, nullptr, &surface.id_) != VK_SUCCESS)
    {
        QMessageBox::critical(nullptr, "Vulkan error", "Failed to create Vulkan surface");
    }
#endif
}

void QVkInstance::destroySurface(QVkSurface& surface) const
{
    vkDestroySurfaceKHR(id_, surface.id_, nullptr);
    surface.id_ = nullptr;
}

void QVkInstance::createDevice(QVkDevice& device, const QVkSurface& surface, quint32 numExt, const char **ext) const
{
    quint32 phdCount = 0;
    vkEnumeratePhysicalDevices(id_, &phdCount, nullptr);
    QVarLengthArray<VkPhysicalDevice, 8> adapters(static_cast<int>(phdCount));
    vkEnumeratePhysicalDevices(id_, &phdCount, adapters.data());
    VkPhysicalDevice discrete = nullptr, integrated = nullptr;
    for (VkPhysicalDevice adapter : adapters)
    {
        quint32 numFamilies = 0, present = VK_FALSE;
        vkGetPhysicalDeviceFeatures(adapter, &device.features_);
        vkGetPhysicalDeviceProperties(adapter, &device.properties_);
        vkGetPhysicalDeviceMemoryProperties(adapter, &device.memoryProperties_);
        vkGetPhysicalDeviceQueueFamilyProperties(adapter, &numFamilies, nullptr);
        QVarLengthArray<VkQueueFamilyProperties, 8> props(static_cast<int>(numFamilies));
        vkGetPhysicalDeviceQueueFamilyProperties(adapter, &numFamilies, props.data());
        const QVkQueueLayout& qLayout = queueLayout(device.properties_.vendorID, device.properties_.deviceID);
        vkGetPhysicalDeviceSurfaceSupportKHR(adapter, qLayout.graphicsQueueFamily(), surface.id_, &present);
        if (present == VK_TRUE)
        {
            if (device.properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                if (!discrete)
                {
                    discrete = adapter;
                    integrated = nullptr;
                    break;
                }
            }
            else if (device.properties_.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                if (!integrated)
                {
                    integrated = adapter;
                }
            }
        }
    }
    device.adapter_ = (discrete) ? discrete : integrated;
    if (device.adapter_)
    {
        quint32 queueOffs = 0;
        const QVkQueueLayout& qLayout = queueLayout(device.properties_.vendorID, device.properties_.deviceID);
        QVarLengthArray<VkDeviceQueueCreateInfo, 3> queueInfo(static_cast<int>(qLayout.numFamilies));
        for (int i = 0; i < queueInfo.size(); i++)
        {
            queueInfo[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo[i].pNext = nullptr;
            queueInfo[i].flags = 0;
            queueInfo[i].queueFamilyIndex = qLayout.family[i];
            queueInfo[i].queueCount = qLayout.count[i];
            queueInfo[i].pQueuePriorities = qLayout.priority + queueOffs;
            queueOffs += qLayout.count[i];
        }
        VkDeviceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.queueCreateInfoCount = qLayout.numFamilies;
        info.pQueueCreateInfos = queueInfo.data();
        device.extensions_.append("VK_KHR_swapchain");
        for (quint32 i = 0; i < numExt; i++)
        {
            device.extensions_.append(ext[i]);
        }
        info.enabledExtensionCount = static_cast<quint32>(device.extensions_.size());
        info.ppEnabledExtensionNames = device.extensions_.data();
        if (vkCreateDevice(device.adapter_, &info, nullptr, &device.id_) != VK_SUCCESS)
        {
            QMessageBox::critical(nullptr, "Vulkan error", "Failed to create Vulkan device");
            return;
        }
        #define VULKAN_API_DEVICE(proc) device.vk##proc=reinterpret_cast<PFN_vk##proc>(vkGetDeviceProcAddr(device.id_,"vk" #proc));Q_ASSERT(device.vk##proc);
        #include "qvulkan.inl"
    }
    else
    {
        QMessageBox::critical(nullptr, "Vulkan error", "No compatible graphics adapters installed");
    }
}

void QVkInstance::destroyDevice(QVkDevice &device) const
{
    device.vkDestroyDevice(device.id_, nullptr);
    device.id_ = nullptr;
}

const QVector<const char *>& QVkInstance::extensions() const
{
    return extensions_;
}

const QVector<const char *>& QVkInstance::layers() const
{
    return layers_;
}

void QVkInstance::initInstanceFunctions()
{
    if (id_)
    {
        #define VULKAN_API_INSTANCE(proc) vk##proc = reinterpret_cast<PFN_vk##proc>(vkGetInstanceProcAddr( id_, "vk" #proc )); Q_ASSERT( vk##proc );
        #include "qvulkan.inl"
    }
    else
    {
        vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(library_.resolve("vkGetInstanceProcAddr"));
        #define VULKAN_API_GLOBAL(proc) vk##proc = reinterpret_cast<PFN_vk##proc>(vkGetInstanceProcAddr( nullptr, "vk" #proc )); Q_ASSERT( vk##proc );
        #include "qvulkan.inl"
    }
}

const QVkQueueLayout& QVkInstance::queueLayout(quint32 /*vendorId*/, quint32 /*deviceId*/) const
{
    static const QVkQueueLayout defaultQueueLayout
    {
        1, {0, 0, 0}, {1, 0, 0}, {1.f, 0.f, 0.f}, {{0, 0}, {0, 0}, {0, 0}}
    };
    return defaultQueueLayout;
}

const VkPhysicalDeviceProperties& QVkDevice::properties() const
{
    return properties_;
}

const QVector<const char*>& QVkDevice::extensions() const
{
    return extensions_;
}

///////////////////////////////////////////////////////////////////


/*void QVkDevice::createBuffer(QVkBuffer& buffer, QVkBufferType contentType, QVkBufferAccess accessType, quint32 size)
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
    if (vkCreateBuffer(id_, &info, nullptr, &buffer.buffer_) == VK_SUCCESS)
    {
        buffer.memory_ = nullptr;
        buffer.size_ = size;
        buffer.offset_ = 0;
        buffer.ptr_ = nullptr;
        VkMemoryRequirements memReqs = {};
        vkGetBufferMemoryRequirements(id_, buffer.buffer_, &memReqs);
        for (quint32 i = 0; i < memoryProperties_.memoryTypeCount; i++)
        {
            if ((memReqs.memoryTypeBits & (1 << i)) && ((memoryProperties_.memoryTypes[i].propertyFlags & pFlags) == pFlags))
            {
                VkMemoryAllocateInfo allocInfo = {};
                allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocInfo.allocationSize = memReqs.size;
                allocInfo.memoryTypeIndex = i;
                if (vkAllocateMemory(id_, &allocInfo, nullptr, &buffer.memory_) == VK_SUCCESS)
                {
                    vkBindBufferMemory(id_, buffer.buffer_, buffer.memory_, 0);
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
        if (vkMapMemory(id_, mem, buffer.offset_, buffer.size_, 0, &buffer.ptr_) == VK_SUCCESS)
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
        vkUnmapMemory(id_, mem);
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
        vkDestroyBuffer(id_, buffer.buffer_, nullptr);
        vkFreeMemory(id_, buffer.memory_, nullptr);
        buffer.memory_ = nullptr;
        buffer.buffer_ = nullptr;
    }
    buffer.size_ = 0;
    buffer.offset_ = 0;
}*/
