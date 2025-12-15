#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>
#include <darmok/math.hpp>
#include <darmok/utils.hpp>
#include <darmok/expected.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/protobuf/scene.pb.h>

#include <bgfx/bgfx.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <variant>

namespace darmok
{
    class Scene;
    class Transform;
    class ICameraComponent;
    struct Ray;
    struct EntityFilter;
    class EntityView;
    struct Plane;
    struct BoundingBox;
	class IComponentLoadContext;

    using ConstCameraComponentRefs = std::vector<std::reference_wrapper<const ICameraComponent>>;
    using CameraComponentRefs = std::vector<std::reference_wrapper<ICameraComponent>>;

    struct DARMOK_EXPORT CameraPerspectiveData final
    {
        float fovy = glm::radians(60.F);
        float near = Math::defaultPerspNear;
        float far = Math::defaultPerspFar;
    };

    struct DARMOK_EXPORT CameraOrthoData final
    {
        glm::vec2 center = glm::vec2(0.5f);
        float near = Math::defaultOrthoNear;
        float far = Math::defaultOrthoFar;
    };

    using CameraProjectionData = std::variant<CameraPerspectiveData, CameraOrthoData>;

    class DARMOK_EXPORT Camera final : IRenderChainDelegate
    {
    public:
        Camera(const glm::mat4& projMatrix = {}) noexcept;
        Camera(Camera&& other) noexcept;

        Scene& getScene() noexcept;
        const Scene& getScene() const noexcept;

        expected<void, std::string> init(Scene& scene, App& app) noexcept;
        expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept;
        expected<void, std::string> render() noexcept;
        expected<void, std::string> update(float deltaTime) noexcept;
        expected<void, std::string> shutdown() noexcept;

        entt::id_type getId() const noexcept;
        std::string getName() const noexcept;
        std::string getViewName(const std::string& baseName) const noexcept;
        std::string toString() const noexcept;

        const glm::mat4& getProjectionMatrix() const noexcept;
        const glm::mat4& getProjectionInverse() const noexcept;

        glm::mat4 getViewProjectionMatrix() const noexcept;
        glm::mat4 getViewProjectionInverse() const noexcept;

        BoundingBox getPlaneBounds(const Plane& plane) const noexcept;

        const CameraProjectionData& getProjection() const noexcept;
        Camera& setProjection(const CameraProjectionData& data) noexcept;
        Camera& setPerspective(float fovy = 60.F, float near = Math::defaultPerspNear, float far = Math::defaultPerspFar) noexcept;
        Camera& setOrtho(const glm::vec2& center = glm::vec2(0.5f), float near = Math::defaultOrthoNear, float far = Math::defaultOrthoFar) noexcept;

        Camera& setViewport(const glm::vec4& vp) noexcept;
        const glm::vec4& getViewport() const noexcept;
        Camera& setBaseViewport(const std::optional<Viewport>& viewport) noexcept;
        const std::optional<Viewport>& getBaseViewport() const noexcept;
        Viewport getCombinedViewport() const noexcept;

        bool isEnabled() const noexcept;
        Camera& setEnabled(bool enabled) noexcept;

        OptionalRef<Transform> getTransform() const noexcept;
        glm::mat4 getViewMatrix() const noexcept;
        glm::mat4 getViewInverse() const noexcept;

        Camera& setCullingFilter(const EntityFilter& filter) noexcept;
        const EntityFilter& getCullingFilter() const noexcept;

        template<typename T>
        Camera& setCullingFilter() noexcept
        {
            return setCullingFilter(entt::type_hash<T>::value());
        }

        template<typename T>
        EntityView getEntities() const noexcept
        {
            return getScene().getEntities<T>(getCullingFilter());
        }

        EntityView getEntities(const EntityFilter& filter) const noexcept;

        expected<void, std::string> addComponent(std::unique_ptr<ICameraComponent> comp) noexcept;
        bool removeComponent(entt::id_type type) noexcept;
        [[nodiscard]] bool hasComponent(entt::id_type type) const noexcept;
        [[nodiscard]] OptionalRef<ICameraComponent> getComponent(entt::id_type type) noexcept;
        [[nodiscard]] OptionalRef<const ICameraComponent> getComponent(entt::id_type type) const noexcept;
        [[nodiscard]] ConstCameraComponentRefs getComponents() const noexcept;
        [[nodiscard]] CameraComponentRefs getComponents() noexcept;

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
        expected<std::reference_wrapper<T>, std::string> addComponent(A&&... args) noexcept
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            auto result = addComponent(std::move(ptr));
            if (!result)
            {
				return unexpected{ std::move(result).error() };
            }
            return ref;
        }

        template<typename T, typename... A>
        OptionalRef<T> tryAddComponent(A&&... args) noexcept
        {
            auto result = addComponent<T>(std::forward<A>(args)...);
            return result ? &result.value().get() : nullptr;
        }

        template<typename T, typename... A>
        expected<std::reference_wrapper<T>, std::string> getOrAddComponent(A&&... args) noexcept
        {
            if (auto comp = getComponent<T>())
            {
                return comp.value();
            }
            return addComponent<T>(std::forward<A>(args)...);
        }

        template<typename T, typename... A>
        OptionalRef<T> tryGetOrAddComponent(A&&... args) noexcept
        {
            auto result = getOrAddComponent<T>(std::forward<A>(args)...);
            return result ? &result.value().get() : nullptr;
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
        [[nodiscard]] bool isWorldPointVisible(const glm::vec3& point) const noexcept;

        [[nodiscard]] RenderChain& getRenderChain() noexcept;
        [[nodiscard]] const RenderChain& getRenderChain() const noexcept;
        
        void configureView(bgfx::ViewId viewId, const std::string& name) const;
        void setViewTransform(bgfx::ViewId viewId) const noexcept;
        void setEntityTransform(Entity entity, bgfx::Encoder& encoder) const noexcept;
        expected<void, std::string> beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;
        bool shouldEntityBeCulled(Entity entity) const noexcept;
        expected<void, std::string> beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;

        // serialization
        using Definition = protobuf::Camera;
        expected<void, std::string> load(const Definition& def) noexcept;

		static Definition createDefinition();

    private:
        glm::mat4 _view;
        glm::mat4 _proj;
        glm::mat4 _projInv;
        CameraProjectionData _projData;
        std::optional<Viewport> _baseViewport;
        glm::vec4 _viewport;

		RenderChain _renderChain;

        bool _enabled;
        std::optional<bool> _updateEnabled;
        bool _transformChanged;

        EntityFilter _cullingFilter;

        using Components = std::vector<std::shared_ptr<ICameraComponent>>;
        Components _components;

        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;

        bool updateProjection() noexcept;

        Components::iterator findComponent(entt::id_type type) noexcept;
        Components::const_iterator findComponent(entt::id_type type) const noexcept;
        Components copyComponents() const noexcept;

        Viewport getRenderChainViewport() const noexcept override;
        OptionalRef<RenderChain> getRenderChainParent() const noexcept override;
        void onRenderChainChanged() noexcept override;

        void setProjectionMatrix(const glm::mat4& matrix) noexcept;
        glm::mat4 getScreenViewMatrix() const noexcept;
        void onTransformChanged();
    };
}