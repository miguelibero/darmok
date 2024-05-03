#include "cegui.hpp"
#include "cegui/renderer.hpp"
#include "cegui/resource.hpp"
#include <darmok/cegui.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/window.hpp>
#include <darmok/input.hpp>
#include <CEGUI/System.h>
#include <CEGUI/ImageCodecModules/STB/ImageCodec.h>
#include <CEGUI/GUIContext.h>
#include <CEGUI/Rectf.h>
#include <CEGUI/Sizef.h>


namespace darmok
{
	CeguiAppComponentImpl::CeguiAppComponentImpl() noexcept
		: _viewSize()
		, _viewOrigin(0)
		, _projFovy(0)
		, _fontScale(0)
	{
	}

	void CeguiAppComponentImpl::init(App& app)
	{
		_app = app;
		_renderer = std::make_unique<CeguiRenderer>(app);
		updateRenderer();
		_resourceProvider = std::make_unique<CeguiResourceProvider>(app.getAssets().getDataLoader());
		CEGUI::System::create(
			*_renderer, _resourceProvider.get()
		);
		auto& renderTarget = _renderer->getDefaultRenderTarget();
		updateRenderTarget(renderTarget);

		_guiContext = CEGUI::System::getSingleton().createGUIContext(renderTarget);
		_guiContext->initDefaultInputSemantics();

		auto& input = app.getInput();
		input.getMouse().addListener(*this);
		input.getKeyboard().addListener(*this);
		// TODO: add gamepad support
	}

	void CeguiAppComponentImpl::shutdown()
	{
		if (_app)
		{
			auto& input = _app->getInput();
			input.getMouse().removeListener(*this);
			input.getKeyboard().removeListener(*this);
		}
		CEGUI::System::destroy();
		_app.reset();
		_renderer.reset();
		_resourceProvider.reset();
		_guiContext.reset();
	}

	void CeguiAppComponentImpl::updateRenderer() const noexcept
	{
		if (_renderer == nullptr)
		{
			return;
		}
		if (_fontScale > 0.F)
		{
			_renderer->setFontScale(_fontScale);
		}
		if (_app)
		{
			auto& winSize = _app->getWindow().getPixelSize();
			auto ceguiSize = CEGUI::Sizef(winSize.x, winSize.y);
			if (_renderer->getDisplaySize() != ceguiSize)
			{
				_renderer->setDisplaySize(ceguiSize);
			}
		}
	}

	void CeguiAppComponentImpl::setFontScale(float scale) noexcept
	{
		_fontScale = scale;
		updateRenderer();
	}

	void CeguiAppComponentImpl::updateRenderTarget(CEGUI::RenderTarget& renderTarget) const noexcept
	{
		CEGUI::Rectf area;
		if (_viewSize)
		{
			area = CEGUI::Rectf(_viewOrigin, CEGUI::Sizef(_viewSize->x, _viewSize->y));
			renderTarget.setArea(area);
		}
		if (_projFovy != 0.F)
		{
			renderTarget.setFovY(_projFovy);
		}
	}

	void CeguiAppComponentImpl::setViewRect(const glm::uvec2& size, const glm::uvec2& origin) noexcept
	{
		_viewSize = size;
		_viewOrigin = origin;
		if (_renderer != nullptr)
		{
			updateRenderTarget(_renderer->getDefaultRenderTarget());
		}
	}

	void CeguiAppComponentImpl::setProjectionFovy(float fovy) noexcept
	{
		_projFovy = fovy;
		if (_renderer != nullptr)
		{
			updateRenderTarget(_renderer->getDefaultRenderTarget());
		}
	}

	OptionalRef<CEGUI::GUIContext> CeguiAppComponentImpl::getGuiContext() noexcept
	{
		return _guiContext;
	}

	OptionalRef<const CEGUI::GUIContext> CeguiAppComponentImpl::getGuiContext() const noexcept
	{
		return _guiContext;
	}

	static CEGUI::Key::Scan convertCeguiKeyboardKey(KeyboardKey key) noexcept
	{
		switch (key)
		{
		case KeyboardKey::Esc:
			return CEGUI::Key::Scan::Esc;
		case KeyboardKey::Return:
			return CEGUI::Key::Scan::Return;
		case KeyboardKey::Tab:
			return CEGUI::Key::Scan::Tab;
		case KeyboardKey::Space:
			return CEGUI::Key::Scan::Space;
		case KeyboardKey::Backspace:
			return CEGUI::Key::Scan::Backspace;
		case KeyboardKey::Up:
			return CEGUI::Key::Scan::ArrowUp;
		case KeyboardKey::Down:
			return CEGUI::Key::Scan::ArrowDown;
		case KeyboardKey::Left:
			return CEGUI::Key::Scan::ArrowLeft;
		case KeyboardKey::Right:
			return CEGUI::Key::Scan::ArrowRight;
		case KeyboardKey::Insert:
			return CEGUI::Key::Scan::Insert;
		case KeyboardKey::Delete:
			return CEGUI::Key::Scan::DeleteKey;
		case KeyboardKey::Home:
			return CEGUI::Key::Scan::Home;
		case KeyboardKey::End:
			return CEGUI::Key::Scan::End;
		case KeyboardKey::PageUp:
			return CEGUI::Key::Scan::PageUp;
		case KeyboardKey::PageDown:
			return CEGUI::Key::Scan::PageDown;
		case KeyboardKey::Plus:
			return CEGUI::Key::Scan::Add;
		case KeyboardKey::Minus:
			return CEGUI::Key::Scan::Subtract;
		case KeyboardKey::LeftBracket:
			return CEGUI::Key::Scan::LeftBracket;
		case KeyboardKey::RightBracket:
			return CEGUI::Key::Scan::RightBracket;
		case KeyboardKey::Semicolon:
			return CEGUI::Key::Scan::Semicolon;
		case KeyboardKey::Quote:
			return CEGUI::Key::Scan::Colon;
		case KeyboardKey::Comma:
			return CEGUI::Key::Scan::Comma;
		case KeyboardKey::Period:
			return CEGUI::Key::Scan::Period;
		case KeyboardKey::Slash:
			return CEGUI::Key::Scan::ForwardSlash;
		case KeyboardKey::Backslash:
			return CEGUI::Key::Scan::Backslash;
		case KeyboardKey::F1:
			return CEGUI::Key::Scan::F1;
		case KeyboardKey::F2:
			return CEGUI::Key::Scan::F2;
		case KeyboardKey::F3:
			return CEGUI::Key::Scan::F3;
		case KeyboardKey::F4:
			return CEGUI::Key::Scan::F4;
		case KeyboardKey::F5:
			return CEGUI::Key::Scan::F5;
		case KeyboardKey::F6:
			return CEGUI::Key::Scan::F6;
		case KeyboardKey::F7:
			return CEGUI::Key::Scan::F7;
		case KeyboardKey::F8:
			return CEGUI::Key::Scan::F8;
		case KeyboardKey::F9:
			return CEGUI::Key::Scan::F9;
		case KeyboardKey::F10:
			return CEGUI::Key::Scan::F10;
		case KeyboardKey::F11:
			return CEGUI::Key::Scan::F11;
		case KeyboardKey::F12:
			return CEGUI::Key::Scan::F12;
		case KeyboardKey::NumPad0:
			return CEGUI::Key::Scan::Numpad_0;
		case KeyboardKey::NumPad1:
			return CEGUI::Key::Scan::Numpad_1;
		case KeyboardKey::NumPad2:
			return CEGUI::Key::Scan::Numpad_2;
		case KeyboardKey::NumPad3:
			return CEGUI::Key::Scan::Numpad_3;
		case KeyboardKey::NumPad4:
			return CEGUI::Key::Scan::Numpad_4;
		case KeyboardKey::NumPad5:
			return CEGUI::Key::Scan::Numpad_5;
		case KeyboardKey::NumPad6:
			return CEGUI::Key::Scan::Numpad_6;
		case KeyboardKey::NumPad7:
			return CEGUI::Key::Scan::Numpad_7;
		case KeyboardKey::NumPad8:
			return CEGUI::Key::Scan::Numpad_8;
		case KeyboardKey::NumPad9:
			return CEGUI::Key::Scan::Numpad_9;
		case KeyboardKey::Key0:
			return CEGUI::Key::Scan::Zero;
		case KeyboardKey::Key1:
			return CEGUI::Key::Scan::One;
		case KeyboardKey::Key2:
			return CEGUI::Key::Scan::Two;
		case KeyboardKey::Key3:
			return CEGUI::Key::Scan::Three;
		case KeyboardKey::Key4:
			return CEGUI::Key::Scan::Four;
		case KeyboardKey::Key5:
			return CEGUI::Key::Scan::Five;
		case KeyboardKey::Key6:
			return CEGUI::Key::Scan::Six;
		case KeyboardKey::Key7:
			return CEGUI::Key::Scan::Seven;
		case KeyboardKey::Key8:
			return CEGUI::Key::Scan::Eight;
		case KeyboardKey::Key9:
			return CEGUI::Key::Scan::Nine;
		case KeyboardKey::KeyA:
			return CEGUI::Key::Scan::A;
		case KeyboardKey::KeyB:
			return CEGUI::Key::Scan::B;
		case KeyboardKey::KeyC:
			return CEGUI::Key::Scan::C;
		case KeyboardKey::KeyD:
			return CEGUI::Key::Scan::D;
		case KeyboardKey::KeyE:
			return CEGUI::Key::Scan::E;
		case KeyboardKey::KeyF:
			return CEGUI::Key::Scan::F;
		case KeyboardKey::KeyG:
			return CEGUI::Key::Scan::G;
		case KeyboardKey::KeyH:
			return CEGUI::Key::Scan::H;
		case KeyboardKey::KeyI:
			return CEGUI::Key::Scan::I;
		case KeyboardKey::KeyJ:
			return CEGUI::Key::Scan::J;
		case KeyboardKey::KeyK:
			return CEGUI::Key::Scan::K;
		case KeyboardKey::KeyL:
			return CEGUI::Key::Scan::L;
		case KeyboardKey::KeyM:
			return CEGUI::Key::Scan::M;
		case KeyboardKey::KeyN:
			return CEGUI::Key::Scan::N;
		case KeyboardKey::KeyO:
			return CEGUI::Key::Scan::O;
		case KeyboardKey::KeyP:
			return CEGUI::Key::Scan::P;
		case KeyboardKey::KeyQ:
			return CEGUI::Key::Scan::Q;
		case KeyboardKey::KeyR:
			return CEGUI::Key::Scan::R;
		case KeyboardKey::KeyS:
			return CEGUI::Key::Scan::S;
		case KeyboardKey::KeyT:
			return CEGUI::Key::Scan::T;
		case KeyboardKey::KeyU:
			return CEGUI::Key::Scan::U;
		case KeyboardKey::KeyV:
			return CEGUI::Key::Scan::V;
		case KeyboardKey::KeyW:
			return CEGUI::Key::Scan::W;
		case KeyboardKey::KeyX:
			return CEGUI::Key::Scan::X;
		case KeyboardKey::KeyY:
			return CEGUI::Key::Scan::Y;
		case KeyboardKey::KeyZ:
			return CEGUI::Key::Scan::Z;
		}
		return CEGUI::Key::Scan::Unknown;
	}

	const static std::unordered_map<KeyboardModifier, CEGUI::Key::Scan> s_ceguiScanModifiers =
	{
		{KeyboardModifier::LeftAlt, CEGUI::Key::Scan::LeftAlt},
		{KeyboardModifier::RightAlt, CEGUI::Key::Scan::RightAlt},
		{KeyboardModifier::LeftCtrl, CEGUI::Key::Scan::LeftControl},
		{KeyboardModifier::RightCtrl, CEGUI::Key::Scan::RightControl},
		{KeyboardModifier::LeftShift, CEGUI::Key::Scan::LeftShift},
		{KeyboardModifier::RightShift, CEGUI::Key::Scan::RightShift},
		{KeyboardModifier::LeftMeta, CEGUI::Key::Scan::LeftWindows},
		{KeyboardModifier::RightMeta, CEGUI::Key::Scan::RightWindows},
	};

	std::vector<CEGUI::Key::Scan> convertCeguiKeyModifiers(uint8_t modifiers) noexcept
	{
		std::vector<CEGUI::Key::Scan> scans;
		for(auto& elm : s_ceguiScanModifiers)
		{
			if (modifiers | to_underlying(elm.first))
			{
				scans.push_back(elm.second);
			}
		}
		return scans;
	}

	void CeguiAppComponentImpl::onKeyboardKey(KeyboardKey key, uint8_t modifiers, bool down)
	{
		auto scans = convertCeguiKeyModifiers(modifiers);
		scans.push_back(convertCeguiKeyboardKey(key));

		for (auto& scan : scans)
		{
			if (down)
			{
				_guiContext->injectKeyDown(scan);
			}
			else
			{
				_guiContext->injectKeyUp(scan);
			}
		}
	}

	void CeguiAppComponentImpl::onKeyboardChar(const Utf8Char& chr)
	{
		_guiContext->injectChar(chr.data);
	}

	void CeguiAppComponentImpl::onMouseActive(bool active)
	{
		if (!active)
		{
			_guiContext->injectMouseLeaves();
		}
	}

	void CeguiAppComponentImpl::onMousePositionChange(const glm::vec2& delta)
	{
		// better in update
	}

	void CeguiAppComponentImpl::onMouseScrollChange(const glm::vec2& delta)
	{
		// better in update
	}

	void CeguiAppComponentImpl::onMouseButton(MouseButton button, bool down)
	{
		auto ceguiButton = CEGUI::MouseButton::Invalid;
		switch (button)
		{
		case MouseButton::Left:
			ceguiButton = CEGUI::MouseButton::Left;
			break;
		case MouseButton::Middle:
			ceguiButton = CEGUI::MouseButton::Middle;
			break;
		case MouseButton::Right:
			ceguiButton = CEGUI::MouseButton::Right;
			break;
		}
		if (down)
		{
			_guiContext->injectMouseButtonDown(ceguiButton);
		}
		else
		{
			_guiContext->injectMouseButtonUp(ceguiButton);
		}
	}

	void CeguiAppComponentImpl::updateLogic(float deltaTime)
	{
		CEGUI::System::getSingleton().injectTimePulse(deltaTime);
		updateRenderer();
		updateGuiContext();
	}

	void CeguiAppComponentImpl::updateGuiContext() noexcept
	{
		if (!_guiContext)
		{
			return;
		}
		if (!_app)
		{
			return;
		}
		auto& mouse = _app->getInput().getMouse();
		auto pos = mouse.getPosition();
		auto& winSize = _app->getWindow().getPixelSize();
		pos.y = winSize.y - pos.y;
		_guiContext->injectMousePosition(pos.x, pos.y);
		auto scroll = mouse.getScrollDelta();
		_guiContext->injectMouseWheelChange(scroll.x);
	}

	bgfx::ViewId CeguiAppComponentImpl::render(bgfx::ViewId viewId) const
	{
		_renderer->setViewId(viewId);
		CEGUI::System::getSingleton().renderAllGUIContexts();
		return _renderer->getViewId();
	}

	void CeguiAppComponentImpl::setResourceGroupDirectory(std::string_view resourceGroup, std::string_view directory) noexcept
	{
		_resourceProvider->setResourceGroupDirectory(
			CEGUI::String(resourceGroup.data(), resourceGroup.size()),
			CEGUI::String(directory.data(), directory.size()));
	}

	CeguiAppComponent::CeguiAppComponent() noexcept
		: _impl(std::make_unique<CeguiAppComponentImpl>())
	{
	}

	CeguiAppComponent::~CeguiAppComponent() noexcept
	{
	}

	void CeguiAppComponent::init(App& app)
	{
		_impl->init(app);
	}

	void CeguiAppComponent::shutdown()
	{
		_impl->shutdown();
	}

	void CeguiAppComponent::updateLogic(float deltaTime)
	{
		_impl->updateLogic(deltaTime);
	}

	bgfx::ViewId CeguiAppComponent::render(bgfx::ViewId viewId) const
	{
		return _impl->render(viewId);
	}

	void CeguiAppComponent::setResourceGroupDirectory(std::string_view resourceGroup, std::string_view directory) noexcept
	{
		_impl->setResourceGroupDirectory(resourceGroup, directory);
	}

	OptionalRef<CEGUI::GUIContext> CeguiAppComponent::getGuiContext() noexcept
	{
		return _impl->getGuiContext();
	}

	OptionalRef<const CEGUI::GUIContext> CeguiAppComponent::getGuiContext() const noexcept
	{
		return _impl->getGuiContext();
	}

	void CeguiAppComponent::setViewRect(const glm::uvec2& size, const glm::uvec2& origin) noexcept
	{
		_impl->setViewRect(size, origin);
	}

	void CeguiAppComponent::setProjectionFovy(float fovy) noexcept
	{
		_impl->setProjectionFovy(fovy);
	}

	void CeguiAppComponent::setFontScale(float scale) noexcept
	{
		_impl->setFontScale(scale);
	}
}