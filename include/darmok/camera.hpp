#pragma once

#include <darmok/export.h>
#include <memory>
#include <vector>
#include <optional>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/entity_filter.hpp>
#include <darmok/shape.hpp>
#include <darmok/viewport.hpp>

namespace darmok
{
    class Mesh;
    class Camera;
    class App;
    class Scene;

    class DARMOK_EXPORT BX_NO_VTABLE ICameraComponent
    {
    public:
        virtual ~ICameraComponent() = default;
        virtual void init(Camera& cam, Scene& scene, App& app) {};
        virtual void update(float deltaTime) {}
        virtual bgfx::ViewId beforeRender(bgfx::ViewId viewId) { return viewId; };
        virtual void beforeRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) { };
        virtual void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) {};
        virtual void afterRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) {};
        virtual void afterRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) { };
        virtual bgfx::ViewId afterRender(bgfx::ViewId viewId) { return viewId; };
        virtual void shutdown() {};
    };

    class DARMOK_EXPORT BX_NO_VTABLE ICameraRenderer
    {
    public:
        virtual void init(Camera& cam, Scene& scene, App& app) {};
        virtual void update(float deltaTime) {}
        virtual bgfx::ViewId render(bgfx::ViewId viewId) const = 0;
        virtual void shutdown() {}
    };

    class Mesh;
    class Texture;
    class IEntityFilter;
    class Transform;

    class DARMOK_EXPORT Camera final
    {
    public:
        Camera(const glm::mat4& projMatrix = {}) noexcept;
        ~Camera();

        const glm::mat4& getProjectionMatrix() const noexcept;

        Camera& setProjectionMatrix(const glm::mat4& matrix) noexcept;
        Camera& setPerspective(float fovy, float aspect, float near = 0.f) noexcept;
        Camera& setPerspective(float fovy, float aspect, float near, float far) noexcept;
        Camera& setPerspective(float fovy, const glm::uvec2& size, float near = 0.f) noexcept;
        Camera& setPerspective(float fovy, const glm::uvec2& size, float near, float far) noexcept;
        
        Camera& setOrtho(const Viewport& viewport, const glm::vec2& center = glm::vec2(0.5f), float near = -bx::kFloatLargest, float far = bx::kFloatLargest) noexcept;
        Camera& setOrtho(const glm::uvec2& size, const glm::vec2& center = glm::vec2(0.5f), float near = -bx::kFloatLargest, float far = bx::kFloatLargest) noexcept;
        Camera& setEntityFilter(std::unique_ptr<IEntityFilter>&& filter) noexcept;

        Camera& setTargetTextures(const std::vector<std::shared_ptr<Texture>>& textures) noexcept;
        const std::vector<std::shared_ptr<Texture>>& getTargetTextures() const noexcept;

        Camera& setViewport(const std::optional<Viewport>& viewport) noexcept;
        const std::optional<Viewport>& getViewport() const noexcept;
        Viewport getCurrentViewport() const noexcept;

        bool isEnabled() const noexcept;
        Camera& setEnabled(bool enabled) noexcept;

        bool isRendererEnabled() const noexcept;
        Camera& setRendererEnabled(bool enabled) noexcept;

        OptionalRef<Transform> getTransform() const noexcept;
        glm::mat4 getModelMatrix() const noexcept;

        Camera& setModelMatrix(std::optional<glm::mat4> mat) noexcept;

        template<typename T>
        Camera& setEntityComponentFilter() noexcept
        {
            return setEntityFilter(std::make_unique<EntityComponentFilter<T>>());
        }

        void filterEntityView(EntityRuntimeView& view) const noexcept;

        template<typename T>
        EntityRuntimeView createEntityView() const
        {
            return createEntityView<T>(getRegistry());
        }

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

        Camera& setRenderer(std::unique_ptr<ICameraRenderer>&& renderer) noexcept;
        Camera& addComponent(std::unique_ptr<ICameraComponent>&& renderer) noexcept;

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

        // trying to maintain Unity API https://docs.unity3d.com/ScriptReference/Camera.html
        Ray screenPointToRay(const glm::vec3& point) const noexcept;
        Ray viewportPointToRay(const glm::vec3& point) const noexcept;
        glm::vec3 worldToScreenPoint(const glm::vec3& point) const noexcept;
        glm::vec3 worldToViewportPoint(const glm::vec3& point) const noexcept;
        glm::vec3 screenToWorldPoint(const glm::vec3& point) const noexcept;
        glm::vec3 viewportToWorldPoint(const glm::vec3& point) const noexcept;
        glm::vec3 viewportToScreenPoint(const glm::vec3& point) const noexcept;
        glm::vec3 screenToViewportPoint(const glm::vec3& point) const noexcept;

        bgfx::ViewId render(bgfx::ViewId viewId) const;

        void beforeRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept;
        void afterRenderView(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept;

        template<typename T>
        void renderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId, T callback)
        {
            beforeRenderEntity(entity, encoder, viewId);
            callback();
            afterRenderEntity(entity, encoder, viewId);
        }

    private:
        bool _enabled;
        bool _rendererEnabled;
        glm::mat4 _proj;
        std::optional<glm::mat4> _model;

        std::optional<Viewport> _viewport;
        std::unique_ptr<IEntityFilter> _entityFilter;
        std::unique_ptr<ICameraRenderer> _renderer;
        std::vector<std::unique_ptr<ICameraComponent>> _components;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;
        std::vector<std::shared_ptr<Texture>> _targetTextures;
        bgfx::FrameBufferHandle _frameBuffer;


        const EntityRegistry& getRegistry() const;
        void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept;
        void afterRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept;
    };
}