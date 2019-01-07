#include "qvulkan.hpp"

#include <QMessageBox>

void QVkInstance::create()
{
    if (layers.isEmpty())
    {
        layers.append("VK_LAYER_LUNARG_standard_validation");
    }
    if (extensions.empty())
    {
        extensions.append("VK_EXT_debug_report");
        extensions.append("VK_KHR_win32_surface");
        extensions.append("VK_KHR_surface");
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
        info.ppEnabledLayerNames = layers.data();
        info.enabledLayerCount = static_cast<quint32>(layers.size());
        info.ppEnabledExtensionNames = extensions.data();
        info.enabledExtensionCount = static_cast<quint32>(extensions.size());
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
    if (surface)
    {
        vkDestroySurfaceKHR(instance, surface, nullptr);
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
    if (vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface) != VK_SUCCESS)
    {
        QMessageBox::critical(nullptr, "Vulkan error", "Failed to create Vulkan surface");
    }
}

void QVkInstance::adapters(QVector<VkPhysicalDevice> &list)
{
    quint32 count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    list.resize(static_cast<int>(count));
    vkEnumeratePhysicalDevices(instance, &count, list.data());
}

void QVkInstance::adapterInfo(VkPhysicalDevice adapter,
                              VkPhysicalDeviceProperties &properties,
                              VkPhysicalDeviceFeatures& features,
                              VkPhysicalDeviceMemoryProperties& memoryProperties)
{
    vkGetPhysicalDeviceProperties(adapter, &properties);
    vkGetPhysicalDeviceFeatures(adapter, &features);
    vkGetPhysicalDeviceMemoryProperties(adapter, &memoryProperties);
}

void QVkDevice::create(QVkInstance& instance, QVkDevice::Type type)
{
    if (!selectAdapter(instance, type))
    {
        QMessageBox::critical(nullptr, "Vulkan error", "No compatible graphics adapters installed");
    }
    else
    {
        //
    }
}

void QVkDevice::createBuffer(QVkBuffer::ContentType contentType, QVkBuffer::AccessType accessType, quint32 size)
{}

void QVkDevice::destroy()
{
    //
}

bool QVkDevice::selectAdapter(QVkInstance &instance, QVkDevice::Type type)
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
        instance.adapterInfo(adapter, properties, features, memoryProperties);
        //pick queue layout based on device properties
        if (type == QVkDevice::Type::GRAPHICS || type == QVkDevice::Type::UNIVERSAL)
        {
            //VkBool32 presentSupport = false;
            //vkGetPhysicalDeviceSurfaceSupportKHR(adapter, i, surface, &presentSupport);
        }
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            if (!discrete)
            {
                discrete = adapter;
                integrated = nullptr;
                break;
            }
        }
        else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            if (!integrated)
            {
                integrated = adapter;
            }
        }
    }
    adapter = (discrete) ? discrete : integrated;
    return (adapter != nullptr);
}
