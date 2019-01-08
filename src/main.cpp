#include "scene.hpp"
#include "window.hpp"
#include "qvulkan.hpp"

#include <QWidget>
#include <QApplication>

int main(int argc, char* argv[])
{
    QVkDevice device;
    QVkInstance instance;
    QApplication app(argc, argv);
    instance.create();
    Window mainWindow(instance, device);
    mainWindow.showMaximized();
    instance.setDisplayWidget(mainWindow.displayWidget());
    instance.createDevice(device, QVkDeviceType::COMPUTE);
    int retval = mainWindow.execute();
    device.destroy();
    instance.destroy();
    return retval;
}
