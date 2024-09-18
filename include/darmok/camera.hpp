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
#include <darmok/math.hpp>

namespace darmok
{
    class Texture;
    class IEntityFilter;
    class Transform;
    class ICameraComponent;
    class Scene;
    struct Ray;

    class DARMOK_EXPORT Camera final : IRenderChainDelegate
    {
    public:
        Camera(const glm::mat4& projMatrix = {}) noexcept;
        ~Camera();

        const glm::mat4& getProjectionMatrix() const noexcept;

        Camera& setProjectionMatrix(const glm::mat4& matrix) noexcept;
        Camera& setPerspective(float fovy, float aspect, float near = Math::defaultNear) noexcept;
        Camera& setPerspective(float fovy, float aspect, float near, float far) noexcept;
        Camera& setPerspective(float fovy, const glm::uvec2& size, float near = Math::defaultNear) noexcept;
        Camera& setPerspective(float fovy, const glm::uvec2& size, float near, float far) noexcept;
        
        Camera& setOrtho(const Viewport& viewport, const glm::vec2& center = glm::vec2(0.5f), float near = Math::defaultNear, float far = Math::defaultFar) noexcept;
        Camera& setOrtho(const glm::uvec2& size, const glm::vec2& center = glm::vec2(0.5f), float near = Math::defaultNear, float far = Math::defaultFar) noexcept;

        Camera& setViewportPerspective(float fovy, float near = Math::defaultNear, float far = Math::defaultFar) noexcept;
        Camera& setViewportOrtho(const glm::vec2& center = glm::vec2(0.5f), float near = Math::defaultNear, float far = Math::defaultFar) noexcept;

        Camera& setEntityFilter(std::unique_ptr<IEntityFilter>&& filter) noexcept;

        Camera& setViewport(const std::optional<Viewport>& viewport) noexcept;
        const std::optional<Viewport>& getViewport() const noexcept;
        Viewport getCurrentViewport() const noexcept;

        bool isEnabled() const noexcept;
        Camera& setEnabled(bool enabled) noexcept;

        OptionalRef<Transform> getTransform() const noexcept;
        glm::mat4 getModelMatrix() const noexcept;
        glm::mat4 getModelInverse() const noexcept;

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

        Camera& addComponent(entt::id_type type, std::unique_ptr<ICameraComponent>&& renderer) noexcept;
        bool removeComponent(entt::id_type type) noexcept;
        [[nodiscard]] bool hasComponent(entt::id_type type) const noexcept;
        [[nodiscard]] OptionalRef<ICameraComponent> getComponent(entt::id_type type) noexcept;
        [[nodiscard]] OptionalRef<const ICameraComponent> getComponent(entt::id_type type) const noexcept;

        template<typename T>
        [[nodiscard]] OptionalRef<T> getComponent() noexcept
        {
            auto ref = getComponent(entt::type_hash<T>::value());
            if (ref)
            {
                return (T*)ref.ptr();
            }
            return nullptr;
        }

        template<typename T>
        [[nodiscard]] OptionalRef<const T> getComponent() const noexcept
        {
            auto ref = getComponent(entt::type_hash<T>::value());
            if (ref)
            {
                return (const T*)ref.ptr();
            }
            return nullptr;
        }

        template<typename T, typename... A>
        T& addComponent(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addComponent(entt::type_hash<T>::value(), std::move(ptr));
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
        
        void configureView(bgfx::ViewId viewId) const;

        void setViewTransform(bgfx::ViewId viewId) const noexcept;
        void setEntityTransform(Entity entity, bgfx::Encoder& encoder) const noexcept;

        void beforeRenderView(IRenderGraphContext& context) const noexcept;
        void beforeRenderEntity(Entity entity, IRenderGraphContext& context) const noexcept;

        static void registerComponentDependency(entt::id_type typeId1, entt::id_type typeId2);

        template<typename T1, typename T2>
        static void registerComponentDependency()
        {
            registerComponentDependency(entt::type_hash<T1>::value(), entt::type_hash<T2>::value());
        }

    private:
        bool _enabled;
        glm::mat4 _proj;

        struct PerspectiveData final
        {
            float fovy;
            float near;
            float far;
        };

        struct OrthoData final
        {
            glm::vec2 center;
            float near;
            float far;
        };

        using ProjectionData = std::variant<PerspectiveData, OrthoData>;
        std::optional<ProjectionData> _vpProj;

        std::optional<Viewport> _viewport;
        std::unique_ptr<IEntityFilter> _entityFilter;

        using Components = std::vector<std::pair<entt::id_type, std::unique_ptr<ICameraComponent>>>;
        Components _components;
        using ComponentDependencies = std::unordered_map<entt::id_type, std::unordered_set<entt::id_type>>;
        static ComponentDependencies _compDeps;

        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;

        RenderGraphDefinition _renderGraph;
        RenderChain _renderChain;
        
        const EntityRegistry& getRegistry() const;

        bool updateViewportProjection() noexcept;
        void updateRenderGraph() noexcept;

        Components::iterator findComponent(entt::id_type type) noexcept;
        Components::const_iterator findComponent(entt::id_type type) const noexcept;

        Viewport getRenderChainViewport() const noexcept override;
        OptionalRef<RenderChain> getRenderChainParent() const noexcept override;
        RenderGraphDefinition& getRenderChainParentGraph() noexcept override;
        void onRenderChainInputChanged() noexcept override;

        using ComponentRefs = std::vector<std::reference_wrapper<ICameraComponent>>;
        ComponentRefs copyComponentContainer() const noexcept;
    };
}