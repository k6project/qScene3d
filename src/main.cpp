#include "scene.hpp"
#include "qvulkan.hpp"

#include <QWidget>
#include <QApplication>

int main(int argc, char* argv[])
{
    Scene scene;
    QVkInstance vkInstance;
    scene.setCamera({0.f, 0.f, 20.f}, {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f});
    //SceneNode cube = scene.addNode("SpinningCube")
    //    .setMesh(...)
    //    .addAnimation(...);
    //scene.updateParameters(0.f, 0.f);
    //device.mapBuffer(pBuff);
    //scene.commitParameters(pBuff.ptr(), pBuff.max());
    vkInstance.create();
    QApplication app(argc, argv);
    QWidget mainWidget;
    mainWidget.resize(512, 512);
    mainWidget.show();
    //if (IsWindow(reinterpret_cast<HWND>(mainWidget.winId())))
    return app.exec();
}
