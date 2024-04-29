#include "render_target.hpp"

namespace darmok
{
	CeguiRenderTarget::CeguiRenderTarget(CEGUI::Renderer& renderer) noexcept
		: _renderer(renderer)
	{
	}

	bool CeguiRenderTarget::isImageryCache() const
	{
		return false;
	}

	void CeguiRenderTarget::updateMatrix() const
	{
		// TODO
	}

	CEGUI::Renderer& CeguiRenderTarget::getOwner()
	{
		return _renderer;
	}
}