#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/viewport.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <darmok/serialize.hpp>
#include <memory>
#include <vector>
#include <optional>
#include <stdexcept>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>

namespace darmok
{
    class Camera;
    class App;
    class Scene;

    class DARMOK_EXPORT BX_NO_VTABLE ICameraComponent
    {
    public:
        virtual ~ICameraComponent() = default;
        virtual std::optional<entt::type_info> getCameraComponentType() const noexcept { return std::nullopt; }
        virtual void init(Camera& cam, Scene& scene, App& app) {}
        virtual bgfx::ViewId renderReset(bgfx::ViewId viewId) { return viewId; }
        virtual void render() {}
        virtual void update(float deltaTime) {}
        virtual void shutdown() {}
        virtual bool shouldEntityBeCulled(Entity entity) { return false; }
        virtual void beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) {}
        virtual void beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) {}
        virtual void onCameraTransformChanged() {}
        virtual void afterLoad() {}
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
        bgfx::VertexLayout getVertexLayout() const noexcept;

        bool valid() const noexcept;
        bool render(bgfx::Encoder& encoder) const;

        static void bindMeta();

        template<typename Archive>
        void save(Archive& archive) const 
        {
            auto& assets = SerializeContextStack<AssetContext>::get();
            auto meshDef = assets.getMeshLoader().getDefinition(_mesh);
            archive(
                CEREAL_NVP_("mesh", meshDef),
                CEREAL_NVP_("material", _material),
                CEREAL_NVP_("enabled", _enabled)
            );
        }

        template<typename Archive>
        void load(Archive& archive) 
        {
            std::shared_ptr<MeshDefinition> meshDef;
            archive(
                CEREAL_NVP_("mesh", meshDef),
                CEREAL_NVP_("material", _material),
                CEREAL_NVP_("enabled", _enabled)
            );
            auto& assets = SerializeContextStack<AssetContext>::get();
            _mesh = assets.getMeshLoader().loadResource(meshDef);
        }

    private:
        bool _enabled;
        std::shared_ptr<IMesh> _mesh;
        std::shared_ptr<Material> _material;
    };
}