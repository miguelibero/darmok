#pragma once

#include <CEGUI/GeometryBuffer.h>

namespace darmok
{
	class CeguiGeometryBuffer final : public CEGUI::GeometryBuffer
	{
	public:
		CeguiGeometryBuffer(CEGUI::RefCounted<CEGUI::RenderMaterial> material) noexcept;
		void draw(uint32_t drawModeMask = CEGUI::DrawModeMaskAll) const override;
	};
}