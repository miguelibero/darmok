#pragma once

#include <memory>
#include <bx/bx.h>
#include <glm/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>

namespace darmok
{
    class BX_NO_VTABLE ICameraComponent
    {
    public:
        virtual ~ICameraComponent() = default;
        virtual void init(Camera& cam, Scene& scene, App& app) { };
        virtual void shutdown() { }
        virtual void update(float deltaTime) { }
    };

    class BX_NO_VTABLE ICameraRenderer : public ICameraComponent
    {
    public:
        virtual bgfx::ViewId render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const = 0;
    };

    class Camera final
    {
    public:
        Camera(const glm::mat4& matrix = {}) noexcept;

        Camera& setMatrix(const glm::mat4& matrix) noexcept;
        Camera& setProjection(float fovy, float aspect, float near, float far) noexcept;
        Camera& setProjection(float fovy, float aspect, float near = 0.f) noexcept;
        Camera& setOrtho(float left, float right, float bottom, float top, float near = 0.f, float far = bx::kFloatLargest, float offset = 0.f) noexcept;
        Camera& setEntityFilter(std::unique_ptr<IEntityFilter>&& filter) noexcept;

        template<typename T>
        Camera& setEntityComponentFilter() noexcept
        {
            return setEntityFilter(std::make_unique<EntityComponentFilter<T>>());
        }

        void filterEntityView(EntityRuntimeView& view) const noexcept;

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
        bgfx::ViewId render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const;

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

    private:
        glm::mat4 _matrix;
        std::unique_ptr<IEntityFilter> _entityFilter;
        std::unique_ptr<ICameraRenderer> _renderer;
        std::vector<std::unique_ptr<ICameraComponent>> _components;
        OptionalRef<Scene> _scene;
        OptionalRef<App> _app;

        void bgfxConfig(const EntityRegistry& registry, bgfx::ViewId viewId) const noexcept;
    };

    using ViewVec = glm::vec<2, uint16_t>;

    class ViewRect final
    {
    public:
        ViewRect(const ViewVec& size, const ViewVec& origin = {}) noexcept;

        [[nodiscard]] const ViewVec& getSize() const noexcept;
        [[nodiscard]] const ViewVec& getOrigin() const noexcept;

        void setSize(const ViewVec& size) noexcept;
        void setOrigin(const ViewVec& origin) noexcept;

        void bgfxConfig(bgfx::ViewId viewId) const noexcept;
    private:
        ViewVec _size;
        ViewVec _origin;
    };
}