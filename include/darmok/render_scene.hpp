#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/viewport.hpp>
#include <memory>
#include <vector>
#include <stdexcept>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

namespace darmok
{
    class Camera;
    class App;
    class Scene;

    class DARMOK_EXPORT BX_NO_VTABLE ICameraComponent
    {
    public:
        virtual ~ICameraComponent() = default;
        virtual entt::id_type getCameraComponentType() const noexcept { return 0; };
        virtual void init(Camera& cam, Scene& scene, App& app) {};
        virtual bgfx::ViewId renderReset(bgfx::ViewId viewId) { return viewId; };
        virtual void render() {};
        virtual void update(float deltaTime) {};
        virtual void shutdown() {};

        virtual bool shouldEntityBeCulled(Entity entity) { return false; };
        virtual void beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) {};
        virtual void beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) { };
        virtual void onCameraTransformChanged() {};
    };

    template<typename T>
    class DARMOK_EXPORT BX_NO_VTABLE ITypeCameraComponent : public ICameraComponent
    {
    public:
        entt::id_type getCameraComponentType() const noexcept override
        {
            return entt::type_hash<T>::value();
        }
    };

    class Material;
    class Texture;
    class IMesh;
    class Program;

    class DARMOK_EXPORT Renderable final
    {
    public:
        Renderable(const std::shared_ptr<IMesh>& mesh = nullptr, const std::shared_ptr<Material>& material = nullptr) noexcept;
        Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& texture) noexcept;
        Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& program, const Color& color) noexcept;
        Renderable(const std::shared_ptr<Material>& material) noexcept;
        std::shared_ptr<IMesh> getMesh() const noexcept;
        Renderable& setMesh(const std::shared_ptr<IMesh>& mesh) noexcept;
        std::shared_ptr<Material> getMaterial() const noexcept;
        Renderable& setMaterial(const std::shared_ptr<Material>& material) noexcept;
        bool isEnabled() const noexcept;
        Renderable& setEnabled(bool enabled) noexcept;
        bool valid() const noexcept;
        bool render(bgfx::Encoder& encoder) const;
    private:
        bool _enabled;
        std::shared_ptr<IMesh> _mesh;
        std::shared_ptr<Material> _material;
    };
}