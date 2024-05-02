#include "render_target.hpp"
#include "renderer.hpp"
#include <bgfx/bgfx.h>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
	CeguiRenderTarget::CeguiRenderTarget(CeguiRenderer& renderer) noexcept
		: _renderer(renderer)
	{
		setArea(CEGUI::Rectf(glm::vec2(0), renderer.getDisplaySize()));
	}

	bool CeguiRenderTarget::isImageryCache() const noexcept
	{
		return false;
	}

	void CeguiRenderTarget::activate() noexcept
	{
		CEGUI::RenderTarget::activate();
		auto viewId = _renderer.getViewId();
		bgfx::setViewRect(viewId, d_area.left(), d_area.bottom(), d_area.right(), d_area.top());

		if (!d_matrixValid)
		{
			updateMatrix();
		}

		bgfx::setViewTransform(viewId, nullptr, glm::value_ptr(d_matrix));
	}

	void CeguiRenderTarget::deactivate() noexcept
	{
		CEGUI::RenderTarget::deactivate();
	}

	void CeguiRenderTarget::updateMatrix() const noexcept
	{
		RenderTarget::updateMatrix(createViewProjMatrixForDirect3D());
	}

	CEGUI::Renderer& CeguiRenderTarget::getOwner() noexcept
	{
		return _renderer;
	}
}