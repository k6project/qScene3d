#include "qvulkan.hpp"

#include <QMessageBox>
#include <QApplication>

void QVkInstance::create()
{
    if (!instance)
    {
        //load library
        //global functions
        VkApplicationInfo appInfo = {};
        appInfo.pApplicationName = QApplication::applicationName().toLatin1().data();
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pEngineName = "qScene3d";
        VkInstanceCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &appInfo;
        //create instance
        //instance level functions
    }
}

void QVkDevice::createBuffer(QVkBuffer::ContentType contentType, QVkBuffer::AccessType accessType, quint32 size)
{
}
