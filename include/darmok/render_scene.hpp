#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/expected.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/viewport.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <darmok/protobuf/scene.pb.h>
#include <memory>
#include <vector>
#include <optional>
#include <stdexcept>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

namespace darmok
{
    class Camera;
    class App;
    class Scene;
    class IComponentLoadContext;

    class DARMOK_EXPORT BX_NO_VTABLE ICameraComponent
    {
    public:
        virtual ~ICameraComponent() = default;
        virtual std::optional<entt::type_info> getCameraComponentType() const noexcept { return std::nullopt; }
        virtual expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept { return {}; }
        virtual expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept { return viewId; }
        virtual expected<void, std::string> render() noexcept { return {}; }
        virtual expected<void, std::string> update(float deltaTime) noexcept { return {}; }
        virtual expected<void, std::string> shutdown() noexcept { return {}; }
        virtual bool shouldEntityBeCulled(Entity entity) { return false; }
        virtual expected<void, std::string> beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept { return {}; }
        virtual expected<void, std::string> beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept { return {}; }
        virtual void onCameraTransformChanged() noexcept {}
        virtual expected<void, std::string> afterLoad() noexcept { return {}; }
    };

    template<typename T>
    class DARMOK_EXPORT BX_NO_VTABLE ITypeCameraComponent : public ICameraComponent
    {
    public:
        std::optional<entt::type_info> getCameraComponentType() const noexcept override
        {
            return entt::type_id<T>();
        }
    };

    struct Material;
    class Texture;
    class Mesh;
    class Program;

    class DARMOK_EXPORT Renderable final
    {
    public:
        Renderable(const std::shared_ptr<Mesh>& mesh = nullptr, const std::shared_ptr<Material>& material = nullptr) noexcept;
        Renderable(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& texture) noexcept;
        Renderable(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Program>& program, const Color& color) noexcept;
        Renderable(const std::shared_ptr<Material>& material) noexcept;
        std::shared_ptr<Mesh> getMesh() const noexcept;
        Renderable& setMesh(const std::shared_ptr<Mesh>& mesh) noexcept;
        std::shared_ptr<Material> getMaterial() const noexcept;
        Renderable& setMaterial(const std::shared_ptr<Material>& material) noexcept;
        bool isEnabled() const noexcept;
        Renderable& setEnabled(bool enabled) noexcept;
        bgfx::VertexLayout getVertexLayout() const noexcept;

        bool valid() const noexcept;
        expected<void, std::string> render(bgfx::Encoder& encoder) const noexcept;

        using Definition = protobuf::Renderable;

        expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt) noexcept;
		static Definition createDefinition() noexcept;

    private:
        bool _enabled;
        std::shared_ptr<Mesh> _mesh;
        std::shared_ptr<Material> _material;
    };
}