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
        Camera(const glm::mat4& matrix = {}, bgfx::ViewId viewId = 0);
        const glm::mat4& getMatrix() const;
        Camera& setMatrix(const glm::mat4& matrix);
        Camera& setProjection(float fovy, float aspect, float near, float far);
        Camera& setProjection(float fovy, float aspect, float near = 0.f);
        Camera& setOrtho(float left, float right, float bottom, float top, float near = 0.f, float far = bx::kFloatLargest, float offset = 0.f);
        Camera& setViewId(bgfx::ViewId viewId);
        bgfx::ViewId getViewId() const;
        Camera& setEntityFilter(std::unique_ptr<IEntityFilter>&& filter);

        template<typename T>
        Camera& setEntityComponentFilter()
        {
            return setEntityFilter(std::make_unique<EntityComponentFilter<T>>());
        }

        OptionalRef<const IEntityFilter> getEntityFilter() const;
        OptionalRef<IEntityFilter> getEntityFilter();
    private:
        glm::mat4 _matrix;
        bgfx::ViewId _viewId;
        std::unique_ptr<IEntityFilter> _entityFilter;
    };

        typedef glm::vec<2, uint16_t> ViewVec;

    class ViewRect final
    {
    public:
        ViewRect(const ViewVec& size, const ViewVec& origin = {});
        void setSize(const ViewVec& size);
        void setOrigin(const ViewVec& origin);
        const ViewVec& getSize() const;
        const ViewVec& getOrigin() const;
        void bgfxConfig(bgfx::ViewId viewId) const; 
    private:
        ViewVec _size;
        ViewVec _origin;
    };
}