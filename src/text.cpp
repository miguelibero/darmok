#include <darmok/text.hpp>
#include <darmok/scene.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
	Text::Text(const std::shared_ptr<IFont>& font, const std::string& content) noexcept
		: _font(font)
		, _color(Colors::black())
		, _changed(false)
		, _vertexNum(0)
		, _indexNum(0)
	{
		setContent(content);
	}

	Text::~Text()
	{
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
		return Utf8Char::toString(_content);
	}

	const Utf8Vector& Text::getContent() const noexcept
	{
		return _content;
	}

	Text& Text::setContent(const std::string& str)
	{
		_content.clear();
		Utf8Char::read(str, _content);
		_changed = true;
		return *this;
	}

	Text& Text::setContent(const std::u8string& str)
	{
		_content.clear();
		Utf8Char::read(str, _content);
		_changed = true;
		return *this;
	}

	Text& Text::setContent(const Utf8Vector& content)
	{
		_content = content;
		_changed = true;
		return *this;
	}

	bool Text::render(bgfx::ViewId viewId, bgfx::Encoder& encoder) const
	{
		if (!_font || !_mesh)
		{
			return false;
		}
		if (_vertexNum == 0 || _indexNum == 0)
		{
			return false;
		}
		if (!_mesh->render(encoder, {
			.numVertices = _vertexNum,
			.numIndices = _indexNum,
			}))
		{
			return false;
		}
		_font->getMaterial().renderSubmit(viewId, encoder);
		return true;
	}

	bool Text::update()
	{
		if (!_font || !_changed)
		{
			return false;
		}

		auto& mat = _font->getMaterial();
		auto prog = mat.getProgram();
		if (!prog)
		{
			return false;
		}

		auto data = createMeshData(_content, *_font, _renderConfig);
		if (data.empty())
		{
			_vertexNum = 0;
			_indexNum = 0;
			return false;
		}

		auto& layout = prog->getVertexLayout();
		data.config.color = _color;

		Data vertexData;
		Data indexData;
		data.exportData(layout, vertexData, indexData);
		if (!_mesh)
		{
			_mesh.emplace(layout, vertexData, indexData);
		}
		else
		{
			_mesh->updateVertices(vertexData);
			_mesh->updateIndices(indexData);
		}
		_vertexNum = data.vertices.size();
		_indexNum = data.indices.size();
		_changed = false;
		return true;
	}

	glm::vec2 TextRenderConfig::getGlyphAdvanceFactor() const noexcept
	{
		auto f = direction == Direction::Negative ? -1 : 1;
		if (axis == Axis::Vertical)
		{
			return glm::vec2(0, f);

		}
		return glm::vec2(f, 0);
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

	MeshData Text::createMeshData(const Utf8Vector& content, const IFont& font, const RenderConfig& config)
	{
		auto& mat = font.getMaterial();

		glm::vec2 pos(0);
		MeshData data;
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

			MeshData glyphData(Rectangle(glyph->size));
			glyphData.config.textureScale = glyph->size;
			glyphData.config.offset = glm::vec3(0);
			glyphData.normalize();
			glyphData.config.textureOffset = glyph->texturePosition;
			glyphData.config.offset = glm::vec3(pos, 0) + glm::vec3(glyph->offset, 0);
			data += glyphData;

			pos += glyphAdv * glm::vec2(glyph->originalSize);
		}

		// TODO: maybe add getter of the size to the font interface
		// to not assume that the size is the diffuse texture size
		auto tex = mat.getTexture(MaterialTextureType::Base);
		auto texScale = glm::vec2(1);
		if (tex)
		{
			texScale /= glm::vec2(tex->getSize());
		}

		data.config.textureScale = texScale;
		data.config.scale = glm::vec3(texScale, 0);

		return data;
	}

	void TextRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		_scene = scene;
		_cam = cam;
		cam.getRenderGraph().addPass(*this);
	}

	void TextRenderer::shutdown() noexcept
	{
		if (_cam)
		{
			_cam->getRenderGraph().removePass(*this);
		}
		_scene.reset();
		_cam.reset();
	}

	void TextRenderer::update(float deltaTime)
	{
		if (!_scene || !_cam)
		{
			return;
		}
		auto texts = _scene->getComponentView<Text>();
		std::unordered_map<std::shared_ptr<IFont>, std::unordered_set<Utf8Char>> fontChars;
		for (auto [entity, text] : texts.each())
		{
			auto& content = text.getContent();
			fontChars[text.getFont()].insert(content.begin(), content.end());
		}
		for (auto& [font, chars] : fontChars)
		{
			font->update(chars);
		}
		for (auto [entity, text] : texts.each())
		{
			text.update();
		}
	}

	void TextRenderer::renderReset() noexcept
	{
		if (_cam)
		{
			_cam->getRenderGraph().addPass(*this);
		}
	}

	void TextRenderer::renderPassDefine(RenderPassDefinition& def) noexcept
	{
		def.setName("Text Renderer");
		def.getWriteResources().add<Texture>();
	}

	void TextRenderer::renderPassConfigure(bgfx::ViewId viewId) noexcept
	{
		_viewId = viewId;
		_cam->configureView(viewId);
	}

	void TextRenderer::renderPassExecute(IRenderGraphContext& context) noexcept
	{
		if (!_scene || !_cam)
		{
			return;
		}
		auto texts = _cam->createEntityView<Text>();
		if (texts.size_hint() == 0)
		{
			return;
		}

		auto& encoder = context.getEncoder();
		_cam->beforeRenderView(_viewId, encoder);

		for (auto entity : texts)
		{
			auto text = _scene->getComponent<Text>(entity);
			_cam->beforeRenderEntity(entity, encoder);
			text->render(_viewId, encoder);
		}
	}

	TextureAtlasFont::TextureAtlasFont(const std::shared_ptr<TextureAtlas>& atlas, const std::shared_ptr<Program>& prog) noexcept
		: _atlas(atlas)
		, _material(prog, _atlas->texture)
	{
	}

	std::optional<Glyph> TextureAtlasFont::getGlyph(const Utf8Char& chr) const noexcept
	{
		auto elm = _atlas->getElement(chr.toString());
		if (!elm)
		{
			return std::nullopt;
		}
		return Glyph{
			.size = elm->size,
			.texturePosition = elm->texturePosition,
			.offset = elm->offset,
			.originalSize = elm->originalSize,
		};
	}

	const Material& TextureAtlasFont::getMaterial() const noexcept
	{
		return _material;
	}

	Material& TextureAtlasFont::getMaterial() noexcept
	{
		return _material;
	}

	float TextureAtlasFont::getLineSize() const noexcept
	{
		auto elm = _atlas->getElement("line");
		if (elm)
		{
			return std::max(elm->originalSize.x, elm->originalSize.y);
		}
		float size = 0;
		for (auto& elm : _atlas->elements)
		{
			if (elm.originalSize.x > size)
			{
				size = elm.originalSize.x;
			}
			if (elm.originalSize.y > size)
			{
				size = elm.originalSize.y;
			}
		}
		return size;
	}

	TextureAtlasFontLoader::TextureAtlasFontLoader(ITextureAtlasLoader& atlasLoader) noexcept
		: _atlasLoader(atlasLoader)
	{
	}

	void TextureAtlasFontLoader::init(App& app)
	{
		_program = std::make_unique<Program>(StandardProgramType::Gui);
	}

	void TextureAtlasFontLoader::shutdown()
	{
		_program.reset();
	}

	std::shared_ptr<IFont> TextureAtlasFontLoader::operator()(std::string_view name)
	{
		auto atlas = _atlasLoader(name);
		if (!atlas)
		{
			return nullptr;
		}
		return std::make_shared<TextureAtlasFont>(atlas, _program);
	}

}