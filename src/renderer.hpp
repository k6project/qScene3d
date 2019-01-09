#pragma once

#include <QWidget>

class Scene;

struct SceneRendererData;

/*
   Renderer provides renderScene() method to perform the main loop iteration (may run on any thread),
   and methods to create and destroy following resources: meshes and materials. Material construction
   involves specifying shaders, textures and fixed-function settings.
*/

class SceneRenderer : public QWidget
{
    Q_OBJECT
public:
    explicit SceneRenderer(QWidget *parent = nullptr);
    virtual ~SceneRenderer() override;
    virtual QPaintEngine* paintEngine() const override;
    void initialize();
    void finalize();
protected:
    virtual void paintEvent(QPaintEvent* paintEvent) override;
    virtual void resizeEvent(QResizeEvent* resizeEvent) override;
    virtual bool event(QEvent* event) override;
private:
    SceneRendererData& data_;
};
