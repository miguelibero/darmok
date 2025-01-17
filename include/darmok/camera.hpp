#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>
#include <darmok/math.hpp>
#include <darmok/utils.hpp>

#include <bgfx/bgfx.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <variant>

namespace darmok
{
    class Scene;
    class CameraImpl;
    class Transform;
    class ICameraComponent;
    class RenderChain;
    struct Ray;
    struct Viewport;
    struct EntityFilter;
    class EntityView;
    struct Plane;
    struct BoundingBox;

    using ConstCameraComponentRefs = std::vector<std::reference_wrapper<const ICameraComponent>>;
    using CameraComponentRefs = std::vector<std::reference_wrapper<ICameraComponent>>;

    struct DARMOK_EXPORT CameraPerspectiveData final
    {
        float fovy = 60.F;
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

    class DARMOK_EXPORT Camera final
    {
    public:
        Camera(const glm::mat4& projMatrix = {}) noexcept;
        ~Camera() noexcept;

        CameraImpl& getImpl() noexcept;
        const CameraImpl& getImpl() const noexcept;

        Scene& getScene();
        const Scene& getScene() const;

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
        EntityView getEntities() const
        {
            return getScene().getEntities<T>(getCullingFilter());
        }

        EntityView getEntities(const EntityFilter& filter) const;

        Camera& addComponent(std::unique_ptr<ICameraComponent>&& comp) noexcept;
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
        T& addComponent(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            addComponent(std::move(ptr));
            return ref;
        }

        template<typename T, typename... A>
        T& getOrAddComponent(A&&... args) noexcept
        {
            if (auto comp = getComponent<T>())
            {
                return comp.value();
            }
            return addComponent<T>(std::forward<A>(args)...);
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
        void beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;
        bool shouldEntityBeCulled(Entity entity) const noexcept;
        void beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;

        static void bindMeta() noexcept;

        template<class Archive>
        void serialize(Archive& archive);

    private:
        std::unique_ptr<CameraImpl> _impl;

        void afterLoad();
    };
}