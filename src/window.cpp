#include "window.hpp"
#include "ui_window.h"

#include "qvulkan.hpp"

Window::Window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Window())
{
    ui->setupUi(this);
}


Window::~Window()
{
    delete ui;
}

void Window::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    ui->renderer_->initialize();
}

void Window::closeEvent(QCloseEvent *event)
{
    ui->renderer_->finalize();
    QMainWindow::closeEvent(event);
}
