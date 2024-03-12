#pragma once

#include <memory>
#include <glm/glm.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>

namespace darmok
{
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

        void update(EntityRegistry& registry) noexcept;
        EntityRuntimeView bgfxConfig(EntityRegistry& registry, bgfx::ViewId viewId) const noexcept;

    private:
        glm::mat4 _matrix;
        std::unique_ptr<IEntityFilter> _entityFilter;
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