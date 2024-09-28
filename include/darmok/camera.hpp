#pragma once

#include <darmok/export.h>
#include <memory>
#include <vector>
#include <optional>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/viewport.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/math.hpp>

namespace darmok
{
    class Texture;
    class Transform;
    class ICameraComponent;
    class Scene;
    struct Ray;

    class DARMOK_EXPORT Camera final : IRenderChainDelegate
    {
    public:
        Camera(const glm::mat4& projMatrix = {}) noexcept;
        ~Camera();

        const std::string& getName() const noexcept;
        Camera& setName(const std::string& name) noexcept;

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

        Camera& setCullingMask(uint32_t mask) noexcept;
        uint32_t getCullingMask() const noexcept;

        Camera& setViewport(const std::optional<Viewport>& viewport) noexcept;
        const std::optional<Viewport>& getViewport() const noexcept;
        Viewport getCurrentViewport() const noexcept;

        bool isEnabled() const noexcept;
        Camera& setEnabled(bool enabled) noexcept;

        OptionalRef<Transform> getTransform() const noexcept;
        glm::mat4 getModelMatrix() const noexcept;
        glm::mat4 getModelInverse() const noexcept;

        std::vector<Entity> getEntities() const noexcept;

        /*
        template<typename T>
        std::vector<Entity> getEntities() const
        {
            auto entities = getEntities();
            if (_scene)
            {
                auto itr = std::remove_if(entities.begin(), entities.end(),
                    [this](auto& entity) { return !_scene->hasComponent<T>(entity); });
                entities.erase(itr, entities.end());
            }
            return entities;
        }*/

        template<typename T>
        EntityRuntimeView getEntities() const
        {
            EntityRuntimeView view;
            if (_scene)
            {
                view.iterate(_scene->getRegistry().storage<T>());
            }
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
        std::string _name;
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
        uint32_t _cullingMask;

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

    struct CullingMask final
    {
        uint32_t value;
    };
}