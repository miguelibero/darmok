#include "target_transform.hpp"
#include "renderer.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <bx/math.h>
#include <CEGUI/RenderTarget.h>

namespace darmok
{
	CeguiTargetTransform::CeguiTargetTransform(CEGUI::RenderTarget& target, CeguiRenderer& renderer) noexcept
		: _target(target)
		, _renderer(renderer)
		, _view(1)
		, _proj(1)
	{
	}

	void CeguiTargetTransform::activate(bool matrixValid) noexcept
	{
		auto viewId = _renderer.getViewId();
		auto& area = _target.getArea();
		bgfx::setViewRect(viewId, area.d_min.x, area.d_min.y, area.getWidth(), area.getHeight());

		if (!matrixValid)
		{
			_target.updateMatrix();
		}

		static const uint16_t clearFlags = BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL;
		bgfx::setViewClear(viewId, clearFlags, 1.F, 0U, 1);
		bgfx::touch(viewId);

		bgfx::setViewTransform(viewId, glm::value_ptr(_view), glm::value_ptr(_proj));
	}

	void CeguiTargetTransform::deactivate() noexcept
	{
		_renderer.setViewId(_renderer.getViewId() + 1);
	}

	CeguiRenderer& CeguiTargetTransform::getRenderer() noexcept
	{
		return _renderer;
	}

	const CeguiRenderer& CeguiTargetTransform::getRenderer() const noexcept
	{
		return _renderer;
	}

	glm::mat4 CeguiTargetTransform::getMatrix() const noexcept
	{
		return _view * _proj;
	}

	float CeguiTargetTransform::updateMatrix(float fovyHalfTan) const noexcept
    {
        auto& area = _target.getArea();
        const float w = area.getWidth();
		const float h = area.getHeight();

		const bool widthAndHeightNotZero = (w != 0.0f) && (h != 0.0f);

		const float aspect = widthAndHeightNotZero ? w / h : 1.0f;
		const float midx = widthAndHeightNotZero ? w * 0.5f : 0.5f;
		const float midy = widthAndHeightNotZero ? h * 0.5f : 0.5f;
		auto viewDist = midx / (aspect * fovyHalfTan);

		auto eye = glm::vec3(midx, midy, viewDist);
		auto center = glm::vec3(midx, midy, -1);
		auto up = glm::vec3(0, -1, 0);
		_view = glm::lookAt(eye, center, up);

		auto fovy = _target.getFovY();
		bx::mtxProj(glm::value_ptr(_proj), glm::degrees(fovy), aspect, float(viewDist * 0.5), float(viewDist * 2.0), bgfx::getCaps()->homogeneousDepth);
		return viewDist;
	}
}