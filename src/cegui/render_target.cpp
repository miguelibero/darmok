#include "render_target.hpp"
#include "renderer.hpp"
#include <bgfx/bgfx.h>
#include <glm/gtc/type_ptr.hpp>
#include <bx/math.h>
#include <bgfx/bgfx.h>

namespace darmok
{
	CeguiRenderTarget::CeguiRenderTarget(CeguiRenderer& renderer) noexcept
		: _trans(*this, renderer)
	{
		setArea(CEGUI::Rectf(glm::vec2(0), renderer.getDisplaySize()));
	}

	bool CeguiRenderTarget::isImageryCache() const noexcept
	{
		return false;
	}

	void CeguiRenderTarget::activate() noexcept
	{
		RenderTarget::activate();
		_trans.activate(d_matrixValid);
		auto viewId = _trans.getRenderer().getViewId();
		bgfx::setViewFrameBuffer(viewId, { bgfx::kInvalidHandle });
	}

	void CeguiRenderTarget::deactivate() noexcept
	{
		RenderTarget::deactivate();
		_trans.deactivate();
	}

	void CeguiRenderTarget::updateMatrix() const noexcept
	{
		d_viewDistance = _trans.updateMatrix(d_fovY_halftan);
		d_matrix = _trans.getMatrix();
		d_matrixValid = true;
	}

	CEGUI::Renderer& CeguiRenderTarget::getOwner() noexcept
	{
		return _trans.getRenderer();
	}
}