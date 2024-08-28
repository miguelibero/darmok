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
#include <darmok/viewport.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/render_chain.hpp>

namespace darmok
{
    class Texture;
    class IEntityFilter;
    class Transform;
    class IRenderer;
    class Scene;
    struct Ray;

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

        Camera& setWindowPerspective(float fovy, float near = 0.f, float far = bx::kFloatLargest) noexcept;
        Camera& setWindowOrtho(const glm::vec2& center = glm::vec2(0.5f), float near = -bx::kFloatLargest, float far = bx::kFloatLargest) noexcept;

        Camera& setEntityFilter(std::unique_ptr<IEntityFilter>&& filter) noexcept;

        Camera& setViewport(const std::optional<Viewport>& viewport) noexcept;
        const std::optional<Viewport>& getViewport() const noexcept;
        Viewport getCurrentViewport() const noexcept;

        bool isEnabled() const noexcept;
        Camera& setEnabled(bool enabled) noexcept;

        OptionalRef<Transform> getTransform() const noexcept;
        glm::mat4 getModelMatrix() const noexcept;

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
        void renderReset();
        void shutdown();

        Camera& addRenderer(std::unique_ptr<IRenderer>&& renderer) noexcept;

        template<typename T, typename... A>
        T& addRenderer(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addRenderer(std::move(ptr));
            return ref;
        }

        // trying to maintain Unity API https://docs.unity3d.com/ScriptReference/Camera.html
        [[nodiscard]] Ray screenPointToRay(const glm::vec3& point) const noexcept;
        [[nodiscard]] Ray viewportPointToRay(const glm::vec3& point) const noexcept;
        [[nodiscard]] glm::vec3 worldToScreenPoint(const glm::vec3& point) const noexcept;
        [[nodiscard]] glm::vec3 worldToViewportPoint(const glm::vec3& point) const noexcept;
        [[nodiscard]] glm::vec3 screenToWorldPoint(const glm::vec3& point) const noexcept;
        [[nodiscard]] glm::vec3 viewportToWorldPoint(const glm::vec3& point) const noexcept;
        [[nodiscard]] glm::vec3 viewportToScreenPoint(const glm::vec3& point) const noexcept;
        [[nodiscard]] glm::vec3 screenToViewportPoint(const glm::vec3& point) const noexcept;

        [[nodiscard]] RenderGraphDefinition& getRenderGraph() noexcept;
        [[nodiscard]] const RenderGraphDefinition& getRenderGraph() const noexcept;
        [[nodiscard]] RenderChain& getRenderChain() noexcept;
        [[nodiscard]] const RenderChain& getRenderChain() const noexcept;
        
        void configureView(bgfx::ViewId viewId) const noexcept;
        void beforeRenderView(bgfx::ViewId viewId) const noexcept;
        void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder) const noexcept;

    private:
        bool _enabled;
        glm::mat4 _proj;

        struct WindowPerspectiveData final
        {
            float fovy;
            float near;
            float far;
        };

        struct WindowOrthoData final
        {
            glm::vec2 center;
            float near;
            float far;
        };

        using WindowProjectionData = std::variant<WindowPerspectiveData, WindowOrthoData>;
        std::optional<WindowProjectionData> _winProj;

        std::optional<Viewport> _viewport;
        std::unique_ptr<IEntityFilter> _entityFilter;
        std::vector<std::unique_ptr<IRenderer>> _renderers;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;

        RenderGraphDefinition _renderGraph;
        RenderChain _renderChain;
        
        const EntityRegistry& getRegistry() const;

        bool updateWindowProjection() noexcept;
        void updateRenderGraph() noexcept;
    };
}