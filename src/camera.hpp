#include <darmok/glm.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/utils.hpp>
#include <darmok/viewport.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/scene_filter.hpp>

namespace darmok
{
    class Camera;
    class ICameraComponent;
    class Scene;
    class App;
    class Transform;
    struct Ray;

    class CameraImpl final : IRenderChainDelegate
    {
    public:
        CameraImpl(Camera& cam, const glm::mat4& projMatrix = {}) noexcept;

        entt::id_type getId() const noexcept;
        const std::string& getName() const noexcept;
        void setName(const std::string& name) noexcept;
        std::string getViewName(const std::string& baseName) const noexcept;
        std::string toString() const noexcept;

        Scene& getScene();
        const Scene& getScene() const;

        const glm::mat4& getProjectionMatrix() const noexcept;
        const glm::mat4& getProjectionInverse() const noexcept;

        glm::mat4 getViewProjectionMatrix() const noexcept;
        glm::mat4 getViewProjectionInverse() const noexcept;

        void setProjectionMatrix(const glm::mat4& matrix) noexcept;
        void setPerspective(float fovy, float aspect, float near = Math::defaultPerspNear, float far = Math::defaultPerspFar) noexcept;
        void setPerspective(float fovy, const glm::uvec2& size, float near = Math::defaultPerspNear, float far = Math::defaultPerspFar) noexcept;
        
        void setOrtho(const Viewport& viewport, const glm::vec2& center = glm::vec2(0.5f), float near = Math::defaultOrthoNear, float far = Math::defaultOrthoFar) noexcept;
        void setOrtho(const glm::uvec2& size, const glm::vec2& center = glm::vec2(0.5f), float near = Math::defaultOrthoNear, float far = Math::defaultOrthoFar) noexcept;

        void setViewportPerspective(float fovy, float near = Math::defaultPerspNear, float far = Math::defaultPerspFar) noexcept;
        void setViewportOrtho(const glm::vec2& center = glm::vec2(0.5f), float near = Math::defaultOrthoNear, float far = Math::defaultPerspFar) noexcept;

        void setViewport(const std::optional<Viewport>& viewport) noexcept;
        const std::optional<Viewport>& getViewport() const noexcept;
        Viewport getCurrentViewport() const noexcept;

        bool isEnabled() const noexcept;
        void setEnabled(bool enabled) noexcept;

        OptionalRef<Transform> getTransform() const noexcept;
        glm::mat4 getViewMatrix() const noexcept;
        glm::mat4 getViewInverse() const noexcept;

        void setCullingFilter(const EntityFilter& filter) noexcept;
        const EntityFilter& getCullingFilter() const noexcept;

        template<typename T>
        EntityView getEntities() const noexcept
        {
            return getScene().getEntities(getCullingFilter() & entt::type_hash<T>::value());
        }

        void init(Scene& scene, App& app);
        void update(float deltaTime);
        bgfx::ViewId renderReset(bgfx::ViewId viewId);
        void render();
        void shutdown();

        void addComponent(std::unique_ptr<ICameraComponent>&& comp) noexcept;
        bool removeComponent(entt::id_type type) noexcept;
        bool hasComponent(entt::id_type type) const noexcept;
        OptionalRef<ICameraComponent> getComponent(entt::id_type type) noexcept;
        OptionalRef<const ICameraComponent> getComponent(entt::id_type type) const noexcept;

        Ray screenPointToRay(const glm::vec3& point) const noexcept;
        Ray viewportPointToRay(const glm::vec3& point) const noexcept;
        glm::vec3 worldToScreenPoint(const glm::vec3& point) const noexcept;
        glm::vec3 worldToViewportPoint(const glm::vec3& point) const noexcept;
        glm::vec3 screenToWorldPoint(const glm::vec3& point) const noexcept;
        glm::vec3 viewportToWorldPoint(const glm::vec3& point) const noexcept;
        glm::vec3 viewportToScreenPoint(const glm::vec3& point) const noexcept;
        glm::vec3 screenToViewportPoint(const glm::vec3& point) const noexcept;

        RenderChain& getRenderChain() noexcept;
        const RenderChain& getRenderChain() const noexcept;
        
        void configureView(bgfx::ViewId viewId, const std::string& name) const;
        void setViewTransform(bgfx::ViewId viewId) const noexcept;
        void setEntityTransform(Entity entity, bgfx::Encoder& encoder) const noexcept;
        bool shouldEntityBeCulled(Entity entity) const noexcept;
        void beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;
        void beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;

        static void registerComponentDependency(entt::id_type typeId1, entt::id_type typeId2);

        template<typename T1, typename T2>
        static void registerComponentDependency()
        {
            registerComponentDependency(entt::type_hash<T1>::value(), entt::type_hash<T2>::value());
        }

    private:
        Camera& _cam;
        std::string _name;
        bool _enabled;
        std::optional<bool> _updateEnabled;
        glm::mat4 _proj;
        glm::mat4 _projInv;

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
        EntityFilter _cullingFilter;

        using Components = std::vector<std::shared_ptr<ICameraComponent>>;
        Components _components;
        using ComponentDependencies = std::unordered_map<entt::id_type, std::unordered_set<entt::id_type>>;
        static ComponentDependencies _compDeps;

        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;

        RenderChain _renderChain;
        
        std::string getDescName() const noexcept;
        bool updateViewportProjection() noexcept;

        Components::iterator findComponent(entt::id_type type) noexcept;
        Components::const_iterator findComponent(entt::id_type type) const noexcept;

        Viewport getRenderChainViewport() const noexcept override;
        OptionalRef<RenderChain> getRenderChainParent() const noexcept override;
        void onRenderChainChanged() noexcept override;

        void doSetProjectionMatrix(const glm::mat4& matrix) noexcept;
        glm::mat4 getScreenViewMatrix() const noexcept;
    };
}