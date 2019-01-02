#include "scene.hpp"

#include <QtGlobal>

void Scene::setCamera(const QVector3D &eye, const QVector3D &target, const QVector3D &up)
{
    view.setToIdentity();
    view.lookAt(eye, target, up);
}

void Scene::updateParameters(float /*deltaT*/, float /*elapsedT*/)
{
    projection.setToIdentity();
    projection.perspective(30.f, 1.f, 0.01f, 1000.f);
}

static void alignToBlock(quintptr& offset)
{
    if (offset > 0)
        offset = (((offset - 1) / BLOCK_ALIGN) + 1) * BLOCK_ALIGN;
}

static void commitMatrix(quint8 *bytes, quintptr &offset, quint64 max, const QMatrix4x4 &matrix)
{
    Q_ASSERT((offset + MAT4X4_SIZE) <= max);
    memcpy(bytes + offset, matrix.constData(), MAT4X4_SIZE);
    offset += MAT4X4_SIZE;
}

void Scene::commitParameters(void* buffer, quint64 max) const
{
    quintptr offset = 0;
    max = (max / BLOCK_ALIGN) * BLOCK_ALIGN;
    quint8* bytes = static_cast<quint8*>(buffer);
    commitMatrix(bytes, offset, max, projection);
    commitMatrix(bytes, offset, max, view);
    alignToBlock(offset);
}
