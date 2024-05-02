#include "geometry.hpp"
#include "renderer.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include <map>
#include <CEGUI/RenderMaterial.h>
#include <CEGUI/RenderEffect.h>
#include <CEGUI/Exceptions.h>
#include <glm/gtc/type_ptr.hpp>
#include <darmok/program.hpp>

namespace darmok
{
	const std::unordered_map<CEGUI::VertexAttributeType, CeguiGeometryBuffer::BgfxAttribData> CeguiGeometryBuffer::_attribDataMap = {
		{CEGUI::VertexAttributeType::Position0,	{bgfx::Attrib::Position, 3, bgfx::AttribType::Float}},
		{CEGUI::VertexAttributeType::Colour0,	{bgfx::Attrib::Color0, 4, bgfx::AttribType::Float}},
		{CEGUI::VertexAttributeType::TexCoord0,	{bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float}}
	};

	CeguiGeometryBuffer::CeguiGeometryBuffer(CEGUI::RefCounted<CEGUI::RenderMaterial> material, CeguiRenderer& renderer)
		: CEGUI::GeometryBuffer(material)
		, _renderer(renderer)
		, _vertexHandle{ bgfx::kInvalidHandle }
		, _textureUniformHandle(bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler))
		, _matrix(1)
	{
		auto& layout = getDarmokProgram().getVertexLayout();
		_vertexHandle = bgfx::createDynamicVertexBuffer(uint32_t(0), layout, BGFX_BUFFER_ALLOW_RESIZE);

		std::map<uint16_t, CEGUI::VertexAttributeType> attribOffsetMap;
		for (auto& elm : _attribDataMap)
		{
			if (layout.has(elm.second.attrib))
			{
				uint8_t num;
				bgfx::AttribType::Enum attribType;
				bool normalized, asInt;
				layout.decode(elm.second.attrib, num, attribType, normalized, asInt);
				if (num != elm.second.num || attribType != elm.second.type)
				{
					throw RendererException("invalid vertex layout attribute");
				}
				auto offset = layout.getOffset(elm.second.attrib);
				attribOffsetMap[offset] = elm.first;
			}
		}
		for (auto& elm : attribOffsetMap)
		{
			addVertexAttribute(elm.second);
		}
	}

	const Program& CeguiGeometryBuffer::getDarmokProgram() const noexcept
	{
		auto shaderWrapper = static_cast<const CeguiShaderWrapper*>(getRenderMaterial()->getShaderWrapper());
		return *shaderWrapper->getDarmokProgram();
	}

	CeguiGeometryBuffer::~CeguiGeometryBuffer()
	{
		if (isValid(_vertexHandle))
		{
			bgfx::destroy(_vertexHandle);
		}
		if (isValid(_textureUniformHandle))
		{
			bgfx::destroy(_textureUniformHandle);
		}
	}

	void CeguiGeometryBuffer::onGeometryChanged() noexcept
	{
		CEGUI::GeometryBuffer::onGeometryChanged();
		if (!d_vertexData.empty())
		{
			auto mem = bgfx::makeRef(&d_vertexData.front(), sizeof(float) * d_vertexData.size());
			bgfx::update(_vertexHandle, 0, mem);
		}
	}

	void CeguiGeometryBuffer::updateMatrix() const
	{
		_matrix = getModelMatrix();
		d_matrixValid = true;
	}

	void CeguiGeometryBuffer::draw(uint32_t drawModeMask) const noexcept
	{
		CEGUI_UNUSED(drawModeMask);

		if (d_vertexData.empty())
		{
			return;
		}

		if (!d_matrixValid)
		{
			updateMatrix();
		}
		
		if (d_clippingActive)
		{
			bgfx::setScissor(
				d_preparedClippingRegion.d_min.x,
				d_preparedClippingRegion.d_min.y,
				d_preparedClippingRegion.getWidth(),
				d_preparedClippingRegion.getHeight()
			);
		}

		auto transCache = bgfx::setTransform(glm::value_ptr(_matrix));

		const int passCount = d_effect ? d_effect->getPassCount() : 1;
		auto& program = getDarmokProgram();
		auto viewId = _renderer.getViewId();

		auto ceguiTexture = static_cast<const CeguiTexture *>(getMainTexture());
		auto texHandle = ceguiTexture->getBgfxHandle();

		for (int pass = 0; pass < passCount; ++pass)
		{
			if (d_effect)
			{
				d_effect->performPreRenderFunctions(pass);
			}
			d_renderMaterial->prepareForRendering();

			bgfx::setTransform(transCache);
			bgfx::setVertexBuffer(0, _vertexHandle, 0, d_vertexCount);
			if (isValid(texHandle))
			{
				bgfx::setTexture(0, _textureUniformHandle, texHandle);
			}

			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
			bgfx::submit(viewId, program.getHandle());

			if (d_effect)
			{
				d_effect->performPostRenderFunctions();
			}
		}
	}
}