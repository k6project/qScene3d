#pragma once

#include <QMainWindow>

class QVkDevice;
class QVkSurface;
class QVkInstance;

namespace Ui
{
    class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT
public:
    explicit Window(QWidget *parent = nullptr);
    virtual ~Window() override;
protected:
    virtual void showEvent(QShowEvent* event) override;
    virtual void closeEvent(QCloseEvent* event) override;
private:
    Ui::Window* ui;
};
