#pragma once

#include <memory>
#include <vector>
#include <optional>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/entity_filter.hpp>
#include <darmok/shape.hpp>

namespace darmok
{
    class Mesh;
    class Camera;
    class App;
    class Scene;

    class BX_NO_VTABLE ICameraComponent
    {
    public:
        DLLEXPORT virtual ~ICameraComponent() = default;
        DLLEXPORT virtual void init(Camera& cam, Scene& scene, App& app) { };
        DLLEXPORT virtual void update(float deltaTime) { }
        DLLEXPORT virtual void beforeRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) const {};
        DLLEXPORT virtual void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) const { };
        DLLEXPORT virtual void afterRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) const {};
        DLLEXPORT virtual void shutdown() { }
    };

    class BX_NO_VTABLE ICameraRenderer : public ICameraComponent
    {
    public:
        DLLEXPORT virtual bgfx::ViewId render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const = 0;
    };

    class Mesh;
    class Texture;
    class IEntityFilter;

    class Camera final
    {
    public:
        DLLEXPORT Camera(const glm::mat4& matrix = {}) noexcept;
        DLLEXPORT ~Camera();

        DLLEXPORT const glm::mat4& getMatrix() const noexcept;

        DLLEXPORT Camera& setMatrix(const glm::mat4& matrix) noexcept;
        DLLEXPORT Camera& setProjection(float fovy, float aspect, const glm::vec2& range) noexcept;
        DLLEXPORT Camera& setProjection(float fovy, float aspect, float near = 0.f) noexcept;
        DLLEXPORT Camera& setProjection(float fovy, const glm::uvec2& size, const glm::vec2& range) noexcept;
        DLLEXPORT Camera& setProjection(float fovy, const glm::uvec2& size, float near = 0.f) noexcept;
        
        DLLEXPORT Camera& setOrtho(const glm::vec4& edges, const glm::vec2& range = glm::vec2(0.f, bx::kFloatLargest), float offset = 0.f) noexcept;
        DLLEXPORT Camera& setOrtho(const glm::uvec2& size, const glm::vec2& range = glm::vec2(0.f, bx::kFloatLargest), float offset = 0.f) noexcept;
        DLLEXPORT Camera& setEntityFilter(std::unique_ptr<IEntityFilter>&& filter) noexcept;

        DLLEXPORT Camera& setTargetTextures(const std::vector<std::shared_ptr<Texture>>& textures) noexcept;
        DLLEXPORT const std::vector<std::shared_ptr<Texture>>& getTargetTextures() noexcept;

        template<typename T>
        Camera& setEntityComponentFilter() noexcept
        {
            return setEntityFilter(std::make_unique<EntityComponentFilter<T>>());
        }

        DLLEXPORT void filterEntityView(EntityRuntimeView& view) const noexcept;

        template<typename T>
        EntityRuntimeView createEntityView(const EntityRegistry& registry) const noexcept
        {
            EntityRuntimeView view;
            auto s = registry.storage(entt::type_hash<T>::value());
            if (s != nullptr)
            {
                view.iterate(*s);
            }
            filterEntityView(view);
            return view;
        }

        void init(Scene& scene, App& app);
        void update(float deltaTime);
        void shutdown();

        DLLEXPORT Camera& setRenderer(std::unique_ptr<ICameraRenderer>&& renderer) noexcept;
        DLLEXPORT Camera& addComponent(std::unique_ptr<ICameraComponent>&& renderer) noexcept;

        template<typename T, typename... A>
        T& setRenderer(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            setRenderer(std::move(ptr));
            return ref;
        }

        template<typename T, typename... A>
        T& addComponent(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addComponent(std::move(ptr));
            return ref;
        }

        DLLEXPORT std::optional<Ray> screenPointToRay(const glm::vec2& point) const noexcept;
        DLLEXPORT bgfx::ViewId render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const;

        DLLEXPORT void beforeRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept;
        DLLEXPORT void afterRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept;
        DLLEXPORT void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept;

    private:
        glm::mat4 _matrix;
        std::unique_ptr<IEntityFilter> _entityFilter;
        std::unique_ptr<ICameraRenderer> _renderer;
        std::vector<std::unique_ptr<ICameraComponent>> _components;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        std::vector<std::shared_ptr<Texture>> _targetTextures;
        bgfx::FrameBufferHandle _frameBuffer;
    };

    class ViewRect final
    {
    public:
        ViewRect(const glm::ivec4& viewport) noexcept;
        [[nodiscard]] const glm::ivec4& getViewport() const noexcept;
        void setViewport(const glm::ivec4& vp) noexcept;

        void beforeRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept;
    private:
        glm::ivec4 _viewport;
    };
}