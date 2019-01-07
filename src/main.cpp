#include "scene.hpp"
#include "qvulkan.hpp"

#include <QWidget>
#include <QApplication>

int main(int argc, char* argv[])
{
    QVkInstance vkInstance;
    QVkDevice device;
    QApplication app(argc, argv);
    QWidget mainWidget;
    mainWidget.resize(512, 512);
    vkInstance.create();
    device.create(vkInstance, QVkDevice::Type::COMPUTE);
    mainWidget.show();
    int retval = app.exec();
    vkInstance.destroy();
    return retval;
}
