#pragma once

#include <CEGUI/GeometryBuffer.h>
#include <bgfx/bgfx.h>
#include <unordered_map>

namespace darmok
{
	class Program;
	class CeguiRenderer;

	class CeguiGeometryBuffer final : public CEGUI::GeometryBuffer
	{
	public:
		CeguiGeometryBuffer(CEGUI::RefCounted<CEGUI::RenderMaterial> material, CeguiRenderer& renderer);
		~CeguiGeometryBuffer();
		void draw(uint32_t drawModeMask = CEGUI::DrawModeMaskAll) const noexcept override;
	protected:
		void onGeometryChanged() noexcept override;
	private:
		mutable glm::mat4 _matrix;
		CeguiRenderer& _renderer;
		bgfx::DynamicVertexBufferHandle _vertexHandle;
		bgfx::UniformHandle _textureUniformHandle;

		struct BgfxAttribData final
		{
			bgfx::Attrib::Enum attrib;
			uint8_t num;
			bgfx::AttribType::Enum type;
		};

		static const std::unordered_map<CEGUI::VertexAttributeType, BgfxAttribData> _attribDataMap;

		void updateMatrix() const;
		const Program& getDarmokProgram() const noexcept;
	};
}