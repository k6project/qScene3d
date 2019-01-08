#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <QMainWindow>

class QVkDevice;
class QVkInstance;

namespace Ui
{
class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT
public:
    explicit Window(QVkInstance& instance, QVkDevice& device, QWidget *parent = nullptr);
    QWidget* displayWidget();
    int execute();
    ~Window();
protected:
    void resizeEvent(QResizeEvent*) override;
private:
    const QVkInstance& instance;
    const QVkDevice& device;
    Ui::Window *ui;
};

#endif // WINDOW_HPP
