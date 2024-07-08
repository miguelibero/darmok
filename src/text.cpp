#include <darmok/text.hpp>
#include <darmok/scene.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>
#include <darmok/program_standard.hpp>

namespace darmok
{
	Text::Text(const std::shared_ptr<IFont>& font, const std::string& content) noexcept
		: _font(font)
	{
		setContent(content);
	}

	Text::~Text()
	{
		if (_font)
		{
			_font->onTextContentChanged(*this, _content, {});
		}
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
		if (_font)
		{
			_font->onTextContentChanged(*this, _content, {});
		}
		_font = font;
		if (_font)
		{
			_font->onTextContentChanged(*this, {}, _content);
		}

		return *this;
	}

	std::string Text::getContentString()
	{
		return Utf8Char::toString(_content);
	}

	Text& Text::setContent(const std::string& str)
	{
		TextContent oldContent = _content;
		Utf8Char::read(str, _content);
		if (_font)
		{
			_font->onTextContentChanged(*this, oldContent, _content);
		}
		return *this;
	}

	Text& Text::setContent(const std::u8string& str)
	{
		TextContent oldContent = _content;
		Utf8Char::read(str, _content);
		if (_font)
		{
			_font->onTextContentChanged(*this, oldContent, _content);
		}
		return *this;
	}

	bool Text::render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const
	{
		if (!_font || !_mesh)
		{
			return false;
		}
		_mesh->render(encoder);
		_font->getMaterial().renderSubmit(encoder, viewId);
		return true;
	}

	bool Text::update()
	{
		if (!_font)
		{
			return false;
		}

		auto prog = _font->getMaterial().getProgram();
		if (!prog)
		{
			return false;
		}
		auto layout = prog->getVertexLayout();

		glm::uvec2 pos(0);
		MeshData data;
		for (auto& chr : _content)
		{
			auto glyph = _font->getGlyph(chr);
			if (!glyph)
			{
				continue;
			}
			MeshData glyphData(Rectangle(glyph->size, glyph->position));
			data.config.offset = glm::vec3(pos.x, pos.y, 0);
			pos.x += glyph->size.x;
			data += glyphData;
		}
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
		return true;
	}

	void TextRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		_scene = scene;
		_cam = cam;
	}

	void TextRenderer::shutdown() noexcept
	{
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
		std::unordered_set<std::shared_ptr<IFont>> fonts;
		for (auto [entity, text] : texts.each())
		{
			fonts.insert(text.getFont());
		}
		for (auto& font : fonts)
		{
			font->update();
		}
		for (auto [entity, text] : texts.each())
		{
			text.update();
		}
	}

	bgfx::ViewId TextRenderer::afterRender(bgfx::ViewId viewId)
	{
		if (!_scene || !_cam)
		{
			return viewId;
		}
		auto texts = _cam->createEntityView<Text>();
		if (texts.size_hint() == 0)
		{
			return viewId;
		}

		auto encoder = bgfx::begin();
		_cam->beforeRenderView(*encoder, viewId);

		for (auto entity : texts)
		{
			auto text = _scene->getComponent<Text>(entity);
			_cam->beforeRenderEntity(entity, *encoder, viewId);
			text->render(*encoder, viewId);
		}
		bgfx::end(encoder);
		return ++viewId;
	}

}