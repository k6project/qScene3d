#pragma once

#include <QVector3D>
#include <QMatrix4x4>

#define BLOCK_ALIGN 256
#define MAT4X4_SIZE (16 * sizeof(float))

class Scene
{
public:
    void setCamera(const QVector3D& eye, const QVector3D& target, const QVector3D& up);
    void updateParameters(float deltaT, float elapsedT);
    void commitParameters(void* buffer, quint64 max) const;
private:
    quint32 verticalFOV, viewWidth, viewHeight, clipDistance;
    QMatrix4x4 projection, view;
};
