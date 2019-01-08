#include "window.hpp"
#include "ui_window.h"

#include "qvulkan.hpp"

#include <QResizeEvent>

Window::Window(QVkInstance& instance, QVkDevice& device, QWidget *parent) :
    QMainWindow(parent),
    instance(instance),
    device(device),
    ui(new Ui::Window)
{
    ui->setupUi(this);
}

QWidget *Window::displayWidget()
{
    return ui->widget;
}

int Window::execute()
{
    for (const char* item : instance.layers())
    {
        ui->layersList_->addItem(item);
    }
    for (const char* item : instance.extensions())
    {
        ui->instExtList_->addItem(item);
    }
    for (const char* item : device.extensions())
    {
        ui->devExtList_->addItem(item);
    }
    QString deviceName = QString("%1 (PCI VEN: %2, DEV: %3)")
            .arg(device.properties().deviceName)
            .arg(device.properties().vendorID)
            .arg(device.properties().deviceID);
    ui->adapterName_->setText(deviceName);
    return QApplication::exec();
}

Window::~Window()
{
    delete ui;
}

void Window::resizeEvent(QResizeEvent* event)
{
    device.viewResized(event->size().width(), event->size().height());
}
