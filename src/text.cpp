#include <darmok/text.hpp>
#include <darmok/scene.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/shape.hpp>
#include <darmok/scene_filter.hpp>

namespace darmok
{
	Text::Text(const std::shared_ptr<IFont>& font, const std::string& content) noexcept
		: _font{ font }
		, _color{ Colors::black() }
		, _changed{ false }
		, _vertexNum{ 0 }
		, _indexNum{ 0 }
	{
		setContent(content);
	}

	const Color& Text::getColor() const noexcept
	{
		return _color;
	}

	Text& Text::setColor(const Color& color) noexcept
	{
		if (_color != color)
		{
			_color = color;
			_changed = true;
		}
		return *this;
	}

	const Text::RenderConfig& Text::getRenderConfig() const noexcept
	{
		return _renderConfig;
	}

	Text::RenderConfig& Text::getRenderConfig() noexcept
	{
		return _renderConfig;
	}

	Text& Text::setRenderConfig(const RenderConfig& config) noexcept
	{
		_renderConfig = config;
		return *this;
	}

	const glm::uvec2& Text::getContentSize() const noexcept
	{
		return _renderConfig.contentSize;
	}

	Text& Text::setContentSize(const glm::uvec2& size) noexcept
	{
		_renderConfig.contentSize = size;
		return *this;
	}

	Text::Orientation Text::getOrientation() const noexcept
	{
		return _renderConfig.orientation;
	}

	Text& Text::setOrientation(Orientation ori) noexcept
	{
		_renderConfig.orientation = ori;
		return *this;
	}

	std::shared_ptr<IFont> Text::getFont() noexcept
	{
		return _font;
	}

	Text& Text::setFont(const std::shared_ptr<IFont>& font) noexcept
	{
		if (_font == font)
		{
			return *this;
		}
		_font = font;
		_changed = true;
		return *this;
	}

	std::string Text::getContentString() const
	{
		return StringUtils::toUtf8(_content);
	}

	const std::u32string& Text::getContent() const noexcept
	{
		return _content;
	}

	Text& Text::setContent(const std::string& str)
	{
		return setContent(StringUtils::toUtf32(str));
	}

	Text& Text::setContent(const std::u32string& str)
	{
		if (_content != str)
		{
			_content = str;
			_changed = true;
		}
		return *this;
	}

	expected<void, std::string> Text::render(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept
	{
		if (!_font || !_mesh)
		{
			return {};
		}
		if (_vertexNum == 0 || _indexNum == 0)
		{
			return {};
		}
		return _mesh->render(encoder, {
			.numVertices = _vertexNum,
			.numIndices = _indexNum,
		});
	}

	expected<void, std::string> Text::update(const bgfx::VertexLayout& layout) noexcept
	{
		if (!_font || !_changed)
		{
			return {};
		}

		auto data = createMeshData(_content, *_font, _renderConfig);
		if (data.empty())
		{
			_vertexNum = 0;
			_indexNum = 0;
			return {};
		}

		data *= _color;

		Data vertexData;
		Data indexData;
		data.exportData(layout, vertexData, indexData);
		if (!_mesh)
		{
			Mesh::Config config{ .type = Mesh::Definition::Dynamic };
			auto meshResult = Mesh::load(layout, vertexData, indexData, config);
			if (!meshResult)
			{
				return unexpected{ std::move(meshResult).error() };
			}
			_mesh = std::move(meshResult).value();
		}
		else
		{
			auto result = _mesh->updateVertices(vertexData);
			if(!result)
			{
				return unexpected{ std::move(result).error() };
			}
			result = _mesh->updateIndices(indexData);
			if (!result)
			{
				return unexpected{ std::move(result).error() };
			}
		}
		_vertexNum = static_cast<uint32_t>(data.vertices.size());
		_indexNum = static_cast<uint32_t>(data.indices.size());
		_changed = false;
		return {};
	}

	glm::vec2 TextRenderConfig::getGlyphAdvanceFactor() const noexcept
	{
		auto factor = direction == Direction::Negative ? -1 : 1;
		if (axis == Axis::Vertical)
		{
			return { 0, factor };

		}
		return { factor, 0 };
	}

	bool TextRenderConfig::fixEndOfLine(glm::vec2& pos, float lineStep) const
	{
		glm::uint lineSize = 0;
		float linePos = 0;
		switch (axis)
		{
		case Axis::Horizontal:
			lineSize = contentSize.x;
			linePos = pos.x;
			break;
		case Axis::Vertical:
			lineSize = contentSize.y;
			linePos = pos.y;
			break;
		}
		if (lineSize == 0 || lineSize > linePos)
		{
			return false;
		}

		auto f = lineDirection == Direction::Negative ? -lineStep : lineStep;
		switch (axis)
		{
		case Axis::Horizontal:
			pos.x = 0;
			pos.y += f;
			break;
		case Axis::Vertical:
			pos.y = 0;
			pos.y += f;
			break;
		}
		return true;

	}

	MeshData Text::createMeshData(std::u32string_view content, const IFont& font, const RenderConfig& config)
	{
		glm::vec2 pos{ 0 };
		MeshData meshData;
		auto lineSize = font.getLineSize();
		auto glyphAdv = config.getGlyphAdvanceFactor();
		for (auto& chr : content)
		{
			auto glyph = font.getGlyph(chr);
			if (!glyph)
			{
				continue;
			}

			config.fixEndOfLine(pos, lineSize);

			auto glyphSize = convert<glm::uvec2>(glyph->size());
			auto glyphTexPos = convert<glm::uvec2>(glyph->texture_position());
			auto glyphOffset = convert<glm::vec2>(glyph->offset());
			auto glyphOriginalSize = convert<glm::uvec2>(glyph->original_size());
			glyphOffset += pos;

			MeshData glyphMesh{ Rectangle{glyphSize, glm::vec2{glyphSize} *0.5F} };

			glyphMesh.scaleTexCoords(glyphSize);
			glyphMesh.translateTexCoords(glyphTexPos);
			glyphMesh.translatePositions({ glyphOffset, 0 });

			meshData += glyphMesh;
			pos += glyphAdv * glm::vec2{ glyphOriginalSize };
		}

		// TODO: maybe add getter of the size to the font interface
		// to not assume that the size is the diffuse texture size
		auto tex = font.getTexture();
		auto texScale = glm::vec2{ 1 };
		if (tex)
		{
			texScale /= glm::vec2{ tex->getSize() };
		}

		meshData.scalePositions({ texScale, 1.F });
		meshData.scaleTexCoords(texScale);

		return meshData;
	}

	TextRenderer::TextRenderer(const std::shared_ptr<Program>& prog) noexcept
		: _prog{ prog }
	{
	}

	expected<void, std::string> TextRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		auto progResult = StandardProgramLoader::load(Program::Standard::Gui);
		if (!progResult)
		{
			return unexpected{ std::move(progResult).error() };
		}
		_prog = progResult.value();
		_scene = scene;
		_cam = cam;
		_colorUniform = { "u_color", bgfx::UniformType::Vec4 };
		_textureUniform = { "s_texColor", bgfx::UniformType::Sampler };
		return {};
	}

	expected<void, std::string> TextRenderer::shutdown() noexcept
	{
		_scene.reset();
		_cam.reset();
		_colorUniform.reset();
		_textureUniform.reset();
		return {};
	}

	expected<void, std::string> TextRenderer::update(float deltaTime) noexcept
	{
		if (!_scene || !_cam)
		{
			return unexpected<std::string>{"camera not loaded"};
		}
		auto entities = _cam->getEntities<Text>();
		std::unordered_map<std::shared_ptr<IFont>, std::unordered_set<char32_t>> fontChars;
		for (auto entity : entities)
		{
			auto& text = _scene->getComponent<Text>(entity).value();
			if (auto font = text.getFont())
			{
				auto& content = text.getContent();
				fontChars[font].insert(content.begin(), content.end());
			}
		}
		for (auto& [font, chars] : fontChars)
		{
			auto updateResult = font->update(chars);
		}
		std::vector<std::string> errors;
		for (auto entity : entities)
		{
			auto& text = _scene->getComponent<Text>(entity).value();
			auto result = text.update(_prog->getVertexLayout());
			if (!result)
			{
				errors.push_back(std::move(result).error());
			}
		}
		return StringUtils::joinExpectedErrors(errors);
	}

	expected<void, std::string> TextRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
	{
		if (!_scene || !_cam)
		{
			return unexpected<std::string>{"camera not loaded"};
		}
		auto entities = _cam->getEntities<Text>();

		static const uint64_t state = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_MSAA
			| BGFX_STATE_BLEND_ALPHA
			;

		for (auto entity : entities)
		{
			auto text = _scene->getComponent<Text>(entity);
			if (!text->getFont() || !text->getFont()->getTexture())
			{
				continue;
			}

			_cam->setEntityTransform(entity, encoder);
			if (!text->render(viewId, encoder))
			{
				continue;
			}
			auto v = Colors::normalize(text->getColor());
			encoder.setUniform(_colorUniform, glm::value_ptr(v));
			encoder.setTexture(0, _textureUniform, text->getFont()->getTexture()->getHandle());

			encoder.setState(state);
			encoder.submit(viewId, _prog->getHandle());
		}

		return {};
	}

	TextureAtlasFont::TextureAtlasFont(const std::shared_ptr<TextureAtlas>& atlas) noexcept
		: _atlas{ atlas }
	{
	}

	std::shared_ptr<Texture> TextureAtlasFont::getTexture() const noexcept
	{
		return _atlas ? _atlas->texture : nullptr;
	}

	std::optional<TextureAtlasFont::Glyph> TextureAtlasFont::getGlyph(char32_t chr) const noexcept
	{
		if (!_atlas)
		{
			return std::nullopt;
		}
		auto key = StringUtils::toUtf8(chr);
		auto elm = _atlas->getElement(key);
		if (!elm)
		{
			return std::nullopt;
		}
		Glyph glyph;
		*glyph.mutable_size() = elm->size();
		*glyph.mutable_texture_position() = elm->texture_position();
		*glyph.mutable_offset() = elm->offset();
		*glyph.mutable_original_size() = elm->original_size();
		return glyph;
	}

	float TextureAtlasFont::getLineSize() const noexcept
	{
		if (!_atlas)
		{
			return 0.0f;
		}
		auto elm = _atlas->getElement("line");
		if (elm)
		{
			auto& size = elm->original_size();
			return std::max(size.x(), size.y());
		}
		float size = 0;
		for (auto& elm : _atlas->elements)
		{
			auto& elmSize = elm.original_size();
			if (elmSize.x() > size)
			{
				size = elmSize.x();
			}
			if (elmSize.y() > size)
			{
				size = elmSize.y();
			}
		}
		return size;
	}

	TextureAtlasFontLoader::TextureAtlasFontLoader(ITextureAtlasLoader& atlasLoader) noexcept
		: _atlasLoader{ atlasLoader }
	{
	}

	TextureAtlasFontLoader::Result TextureAtlasFontLoader::operator()(std::filesystem::path path) noexcept
	{
		auto atlasResult = _atlasLoader(path);
		if (!atlasResult)
		{
			return unexpected{ atlasResult.error() };
		}
		return std::make_shared<TextureAtlasFont>(atlasResult.value());
	}

}
