#include "renderer.hpp"

#include "scene.hpp"
#include "qvulkan.hpp"

#include <QEvent>
#include <QMutex>
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
    RenderThread(const SceneRendererDevice& device)
        : QThread()
        , device_(device)
    {}
    void startThread()
    {
        setObjectName("RenderThread");
        keepRunning_ = true;
        start();
    }
    void stopAndWait()
    {
        keepRunning_ = false;
        wait();
    }
    void suspend()
    {
        suspendMutex_.lock();
    }
    void resume()
    {
        suspendMutex_.unlock();
    }
private:
    virtual void run() override;
    const SceneRendererDevice& device_;
    bool keepRunning_ = true;
    QMutex suspendMutex_;
};

class SceneRendererDevice : public QVkDevice
{
public:
    bool isReady = false;
    QVkSurface surface;
    QVkSwapchain swapchain;
    QVkQueue cmdQueue;
    RenderThread renderThread;
    SceneRendererDevice() : renderThread(*this) {}
};


void RenderThread::run()
{
    while (keepRunning_)
    {
        suspendMutex_.lock();
        if (device_.acquireNextImage(device_.swapchain))
        {
            device_.queuePresent(device_.swapchain);
        }
        suspendMutex_.unlock();
        yieldCurrentThread();
    }
    device_.waitIdle();
}

SceneRenderer::SceneRenderer(QWidget *parent)
    : QWidget(parent)
    , device_(*(new SceneRendererDevice))
{
}

SceneRenderer::~SceneRenderer()
{
    delete &device_;
}

QPaintEngine* SceneRenderer::paintEngine() const
{
    return nullptr;
}

void SceneRenderer::initialize()
{
    const QVkInstance& instance = QVkInstance::get();
    instance.createSurface(device_.surface, this);
    instance.createDevice(device_, device_.surface);
    device_.createSwapchain(device_.swapchain, device_.surface);
    device_.isReady = true;
    device_.renderThread.startThread();
}

void SceneRenderer::finalize()
{
    device_.renderThread.stopAndWait();
    const QVkInstance& instance = QVkInstance::get();
    instance.destroyDevice(device_);
    instance.destroySurface(device_.surface);
    instance.destroy();
}

void SceneRenderer::paintEvent(QPaintEvent*)
{
}

void SceneRenderer::resizeEvent(QResizeEvent*)
{
    if (device_.isReady)
    {
        device_.renderThread.suspend();
        device_.waitIdle();
        device_.createSwapchain(device_.swapchain, device_.surface);
        device_.renderThread.resume();
    }
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
