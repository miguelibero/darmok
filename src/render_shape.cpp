#include <darmok/render_shape.hpp>
#include <darmok/reflect_serialize.hpp>

namespace darmok
{
    void CubeRenderable::bindMeta()
    {
        ReflectionSerializeUtils::metaSerialize<CubeRenderable>();
        SceneReflectionUtils::metaEntityComponent<CubeRenderable>("CubeRenderable")
            .ctor();
    }

    void SphereRenderable::bindMeta()
    {
        ReflectionSerializeUtils::metaSerialize<SphereRenderable>();
        SceneReflectionUtils::metaEntityComponent<SphereRenderable>("SphereRenderable")
            .ctor();
    }

    void CapsuleRenderable::bindMeta()
    {
        ReflectionSerializeUtils::metaSerialize<CapsuleRenderable>();
        SceneReflectionUtils::metaEntityComponent<CapsuleRenderable>("CapsuleRenderable")
            .ctor();
    }

    void PlaneRenderable::bindMeta()
    {
        ReflectionSerializeUtils::metaSerialize<PlaneRenderable>();
        SceneReflectionUtils::metaEntityComponent<PlaneRenderable>("PlaneRenderable")
            .ctor();
    }
}