#pragma once

#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>

#include "memory.hpp"

#define BLOCK_ALIGN 256
#define MAT4X4_SIZE (16 * sizeof(float))

class SceneNode;

class Scene
{
public:
    static const memsize_t DEFAULT_MAX_NODES = 65536;
    void create(memsize_t maxNodes = DEFAULT_MAX_NODES);
    void destroy();
    SceneNode addNode();
    void setCamera(const QVector3D& eye, const QVector3D& target, const QVector3D& up);
    void updateParameters(float deltaT, float elapsedT);
    void commitParameters(void* buffer, quint64 max) const;
protected:
    struct Node;
    struct Globals;
    friend class SceneNode;
private:
    //quint32 verticalFOV, viewWidth, viewHeight, clipDistance;
    //QMatrix4x4 projection, view;
    quint64 numNodes = 0;
    MemBlock nodeMem;
    Node* nodes = nullptr;
};

class SceneNode
{
public:
    const SceneNode& setPosition(const QVector3D& position) const;
    const SceneNode& setRotation(const QVector3D& eulerAngles) const;
    const SceneNode& setScale(const QVector3D& axisScale) const;
    const SceneNode& setScale(float uniformScale) const;
private:
    friend class Scene;
    Scene::Node* node;
};
