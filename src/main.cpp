#include "window.hpp"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    Window mainWindow(nullptr);
    mainWindow.showMaximized();
    int retval = app.exec();
    return retval;
}
