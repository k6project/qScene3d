#include "renderer.hpp"

#include "scene.hpp"
#include "qvulkan.hpp"

#include <QEvent>
#include <QCloseEvent>

struct SceneRendererData
{
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
