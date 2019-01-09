#include "renderer.hpp"

#include "scene.hpp"
#include "qvulkan.hpp"

#include <QEvent>
#include <QThread>
#include <QCloseEvent>

struct SceneRendererData
{
    QVkSurface surface;
    QVkDevice device;
    QVkSwapchain swapchain;
    QVkQueue cmdQueue;
};

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

class RenderThread : public QThread
{
    Q_OBJECT
public:

private:
    const QVkDevice* device_;
    virtual void run() override
    {
        while (true)
        {
            yieldCurrentThread();
        }
        device_->waitIdle();
    }
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
    //launch render and upload threads
}

void SceneRenderer::finalize()
{
    // stop render and upload threads
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
