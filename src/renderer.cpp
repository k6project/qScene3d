#include "renderer.hpp"

#include "scene.hpp"
#include "qvulkan.hpp"

#include <QEvent>
#include <QThread>
#include <QCloseEvent>

/*
RingQueue with atomics:
enqueue()->
    load tail (acquire)
    next_tail = (tail + 1) mod MAX
    do
        load head(acquire)
    while head == next_tail
    <...init new queue item...>
    store tail=next_tail (release)
dequeue()->
    load tail (acquire)
    if (head != tail)
        return result
    return null
*/

class RenderThread : QThread
{
public:
    void startThread(const QVkDevice& device)
    {
        setObjectName("RenderThread");
        device_ = &device;
        keepRunning_ = true;
        start();
    }
    void stopAndWait()
    {
        keepRunning_ = false;
        wait();
    }
private:
    const QVkDevice* device_;
    bool keepRunning_ = true;
    virtual void run() override
    {
        while (keepRunning_)
        {
            yieldCurrentThread();
        }
        device_->waitIdle();
    }
};

struct SceneRendererData
{
    RenderThread renderThread;
    QVkSurface surface;
    QVkDevice device;
    QVkSwapchain swapchain;
    QVkQueue cmdQueue;
};


SceneRenderer::SceneRenderer(QWidget *parent)
    : QWidget(parent)
    , data_(*(new SceneRendererData))
{
}

SceneRenderer::~SceneRenderer()
{
    delete &data_;
}

QPaintEngine* SceneRenderer::paintEngine() const
{
    return nullptr;
}

void SceneRenderer::initialize()
{
    const QVkInstance& instance = QVkInstance::get();
    instance.createSurface(data_.surface, this);
    instance.createDevice(data_.device, data_.surface);
    //data_.device.createSwapchain(data_.swapchain, data_.surface);
    data_.renderThread.startThread(data_.device);
}

void SceneRenderer::finalize()
{
    data_.renderThread.stopAndWait();
    const QVkInstance& instance = QVkInstance::get();
    instance.destroyDevice(data_.device);
    instance.destroySurface(data_.surface);
    instance.destroy();
}

void SceneRenderer::paintEvent(QPaintEvent*)
{
}

void SceneRenderer::resizeEvent(QResizeEvent*)
{
    //data_.device.createSwapchain(data_.swapchain, data_.surface);
}

bool SceneRenderer::event(QEvent* event)
{
    switch(event->type())
    {
        case QEvent::UpdateRequest:
            return true;
        default:
             return QWidget::event(event);
    }
}
