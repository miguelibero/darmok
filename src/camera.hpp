#include <darmok/glm.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/utils.hpp>
#include <darmok/viewport.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/reflect_serialize.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

namespace darmok
{
    class Camera;
    class ICameraComponent;
    class Scene;
    class App;
    class Transform;
    struct Ray;

    using ConstCameraComponentRefs = std::vector<std::reference_wrapper<const ICameraComponent>>;
    using CameraComponentRefs = std::vector<std::reference_wrapper<ICameraComponent>>;

    class CameraComponentCerealListDelegate final : public ICerealReflectSaveListDelegate<ICameraComponent>, public ICerealReflectLoadListDelegate<ICameraComponent>
    {
    public:
        CameraComponentCerealListDelegate(Camera& cam) noexcept;
        entt::meta_any create(const entt::meta_type& type) override;
        std::optional<entt::type_info> getTypeInfo(const ICameraComponent& comp) const noexcept override;
        ConstCameraComponentRefs getList() const noexcept override;
        void afterLoad() noexcept override;
    private:
        Camera& _cam;
    };


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

        BoundingBox getPlaneBounds(const Plane& plane) const noexcept;

        const CameraProjectionData& getProjection() const noexcept;
        void setProjection(const CameraProjectionData& data) noexcept;

        void setViewport(const glm::vec4& viewport) noexcept;
        const glm::vec4& getViewport() const noexcept;
        void setBaseViewport(const std::optional<Viewport>& viewport) noexcept;
        const std::optional<Viewport>& getBaseViewport() const noexcept;
        Viewport getCombinedViewport() const noexcept;

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
        ConstCameraComponentRefs getComponents() const noexcept;
        CameraComponentRefs getComponents() noexcept;

        Ray screenPointToRay(const glm::vec3& point) const noexcept;
        Ray viewportPointToRay(const glm::vec3& point) const noexcept;
        glm::vec3 worldToScreenPoint(const glm::vec3& point) const noexcept;
        glm::vec3 worldToViewportPoint(const glm::vec3& point) const noexcept;
        glm::vec3 screenToWorldPoint(const glm::vec3& point) const noexcept;
        glm::vec3 viewportToWorldPoint(const glm::vec3& point) const noexcept;
        glm::vec3 viewportToScreenPoint(const glm::vec3& point) const noexcept;
        glm::vec3 screenToViewportPoint(const glm::vec3& point) const noexcept;
        bool isWorldPointVisible(const glm::vec3& point) const noexcept;

        RenderChain& getRenderChain() noexcept;
        const RenderChain& getRenderChain() const noexcept;
        
        void configureView(bgfx::ViewId viewId, const std::string& name) const;
        void setViewTransform(bgfx::ViewId viewId) const noexcept;
        void setEntityTransform(Entity entity, bgfx::Encoder& encoder) const noexcept;
        bool shouldEntityBeCulled(Entity entity) const noexcept;
        void beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;
        void beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;

        void afterLoad();

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("name", _name),
                CEREAL_NVP_("enabled", _enabled),
                CEREAL_NVP_("proj", _projData),
                CEREAL_NVP_("viewport", _viewport),
                CEREAL_NVP_("cullingFilter", _cullingFilter),
                CEREAL_NVP_("renderChain", _renderChain),
                CEREAL_NVP_("components", CameraComponentCerealListDelegate(_cam))
            );
        }

    private:
        Camera& _cam;
        std::string _name;
        bool _enabled;
        std::optional<bool> _updateEnabled;
        glm::mat4 _proj;
        glm::mat4 _projInv;
        glm::mat4 _view;
        bool _transformChanged;      

        CameraProjectionData _projData;

        std::optional<Viewport> _baseViewport;
        glm::vec4 _viewport;
        EntityFilter _cullingFilter;

        using Components = std::vector<std::shared_ptr<ICameraComponent>>;
        Components _components;

        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;

        RenderChain _renderChain;
        
        std::string getDescName() const noexcept;
        bool updateProjection() noexcept;

        Components::iterator findComponent(entt::id_type type) noexcept;
        Components::const_iterator findComponent(entt::id_type type) const noexcept;

        Viewport getRenderChainViewport() const noexcept override;
        OptionalRef<RenderChain> getRenderChainParent() const noexcept override;
        void onRenderChainChanged() noexcept override;

        void setProjectionMatrix(const glm::mat4& matrix) noexcept;
        glm::mat4 getScreenViewMatrix() const noexcept;
        void onTransformChanged();
    };

    template<typename Archive>
    void serialize(Archive& archive, CameraPerspectiveData& data)
    {
        CEREAL_NVP(data.fovy);
        CEREAL_NVP(data.near);
        CEREAL_NVP(data.far);
    }

    template<typename Archive>
    void serialize(Archive& archive, CameraOrthoData& data)
    {
        CEREAL_NVP(data.center);
        CEREAL_NVP(data.near);
        CEREAL_NVP(data.far);
    }
}