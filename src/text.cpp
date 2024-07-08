#include <darmok/text.hpp>
#include <darmok/scene.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>

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
		auto oldContent = _content;
		Utf8Char::read(str, _content);
		if (_font)
		{
			_font->onTextContentChanged(*this, oldContent, _content);
		}
		return *this;
	}

	Text& Text::setContent(const std::u8string& str)
	{
		auto oldContent = _content;
		Utf8Char::read(str, _content);
		if (_font)
		{
			_font->onTextContentChanged(*this, oldContent, _content);
		}
		return *this;
	}

	void Text::render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const
	{
	}

	void TextRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		_scene = scene;
		_alloc = app.getAssets().getAllocator();
	}

	void TextRenderer::shutdown() noexcept
	{
		_scene.reset();
		_alloc.reset();
	}

	void TextRenderer::update(float deltaTime)
	{
		if (!_scene || !_alloc)
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
	}

	bgfx::ViewId TextRenderer::afterRender(bgfx::ViewId viewId)
	{
		auto texts = _scene->getComponentView<Text>();
		if (texts.empty())
		{
			return viewId;
		}
		auto encoder = bgfx::begin();
		for (auto [entity, text] : texts.each())
		{
			text.render(*encoder, viewId);
		}
		bgfx::end(encoder);
		return ++viewId;
	}

}