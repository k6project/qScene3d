#include "scene.hpp"

#include <QtGlobal>

struct Scene::Node
{
    friend class Scene;
    friend class SceneNode;
    QMatrix4x4 transform;
    QVector3D position, scale;
    QQuaternion rotation;
};

struct Scene::Globals
{
    struct { int verticalFOV, viewWidth, viewHeight, clipDistance; } projection;
    struct { QVector3D pos, target, up; } camera;
};

void Scene::create(memsize_t maxNodes)
{
    memsize_t total = maxNodes * sizeof(Node);
    nodes = static_cast<Node*>(malloc(total));
    nodeMem.init(total);
}

void Scene::destroy()
{
    free(nodes);
    nodes = nullptr;
    numNodes = 0;
}

SceneNode Scene::addNode()
{
    SceneNode retval;
    retval.node = nodeMem.allocPtr<Node>(nodes);
    return retval;
}

void Scene::setCamera(const QVector3D &eye, const QVector3D &target, const QVector3D &up)
{
    //view.setToIdentity();
    //view.lookAt(eye, target, up);
}

void Scene::updateParameters(float /*deltaT*/, float /*elapsedT*/)
{
    //projection.setToIdentity();
    //float aspectRatio = viewWidth / static_cast<float>(viewHeight);
    //projection.perspective(verticalFOV, aspectRatio, 0.01f, clipDistance + 0.01f);
    for (quint64 i = 0; i < numNodes; i++)
    {
        Node& node = nodes[i];
        node.transform.setToIdentity();
    }
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
    //commitMatrix(bytes, offset, max, projection);
    //commitMatrix(bytes, offset, max, view);
    alignToBlock(offset);
}

const SceneNode& SceneNode::setRotation(const QVector3D &eulerAngles) const
{
    node->rotation.fromEulerAngles(eulerAngles);
    return *this;
}
