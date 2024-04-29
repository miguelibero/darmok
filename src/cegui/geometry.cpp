#include "geometry.hpp"

namespace darmok
{
	CeguiGeometryBuffer::CeguiGeometryBuffer(CEGUI::RefCounted<CEGUI::RenderMaterial> material) noexcept
		: CEGUI::GeometryBuffer(material)
	{
	}

	void CeguiGeometryBuffer::draw(uint32_t drawModeMask) const
	{
	}
}