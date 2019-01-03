#pragma once

#include <QVector>
#include <QVector3D>
#include <QMatrix4x4>

#define BLOCK_ALIGN 256
#define MAT4X4_SIZE (16 * sizeof(float))

class SceneNode;

class Scene
{
public:
    void setCamera(const QVector3D& eye, const QVector3D& target, const QVector3D& up);
    void updateParameters(float deltaT, float elapsedT);
    void commitParameters(void* buffer, quint64 max) const;
protected:
    class Node;
    class Globals;
    friend class SceneNode;
private:
    quint32 verticalFOV, viewWidth, viewHeight, clipDistance;
    QMatrix4x4 projection, view;
    quint64 numNodes;
    Node* nodes;
};

class SceneNode
{
public:
    void setPosition(const QVector3D& position) const;
    void setRotation(const QVector3D& eulerAngles) const;
    void setScale(const QVector3D& axisScale) const;
    void setScale(float uniformScale) const;
private:
    Scene::Node* node;
};
