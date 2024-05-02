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
		auto& winSize = _app->getWindow().getPixelSize();
		_renderer->setDisplaySize(CEGUI::Sizef(winSize.x, winSize.y));
		if (_fontScale > 0.F)
		{
			_renderer->setFontScale(_fontScale);
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
		}
		else if(_app)
		{
			auto& winSize = _app->getWindow().getPixelSize();
			area.setSize(CEGUI::Sizef(winSize.x, winSize.y));
		}
		renderTarget.setArea(area);
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

	void CeguiAppComponentImpl::onKeyboardKey(KeyboardKey key, uint8_t modifiers, bool down)
	{
		auto ceguiScan = CEGUI::Key::Scan::Unknown;
		switch (key)
		{
			case KeyboardKey::Esc:
				ceguiScan = CEGUI::Key::Scan::Esc;
			    break;
			case KeyboardKey::Return:
				ceguiScan = CEGUI::Key::Scan::Return;
			    break;
			case KeyboardKey::Tab:
				ceguiScan = CEGUI::Key::Scan::Tab;
			    break;
			case KeyboardKey::Space:
				ceguiScan = CEGUI::Key::Scan::Space;
			    break;
			case KeyboardKey::Backspace:
				ceguiScan = CEGUI::Key::Scan::Backspace;
			    break;
			case KeyboardKey::Up:
				ceguiScan = CEGUI::Key::Scan::ArrowUp;
			    break;
			case KeyboardKey::Down:
				ceguiScan = CEGUI::Key::Scan::ArrowDown;
			    break;
			case KeyboardKey::Left:
				ceguiScan = CEGUI::Key::Scan::ArrowLeft;
			    break;
			case KeyboardKey::Right:
				ceguiScan = CEGUI::Key::Scan::ArrowRight;
			    break;
			case KeyboardKey::Insert:
				ceguiScan = CEGUI::Key::Scan::Insert;
			    break;
			case KeyboardKey::Delete:
				ceguiScan = CEGUI::Key::Scan::DeleteKey;
			    break;
			case KeyboardKey::Home:
				ceguiScan = CEGUI::Key::Scan::Home;
			    break;
			case KeyboardKey::End:
				ceguiScan = CEGUI::Key::Scan::End;
			    break;
			case KeyboardKey::PageUp:
				ceguiScan = CEGUI::Key::Scan::PageUp;
			    break;
			case KeyboardKey::PageDown:
				ceguiScan = CEGUI::Key::Scan::PageDown;
			    break;
			case KeyboardKey::Print:
			    break;
			case KeyboardKey::Plus:
				ceguiScan = CEGUI::Key::Scan::Add;
			    break;
			case KeyboardKey::Minus:
				ceguiScan = CEGUI::Key::Scan::Subtract;
			    break;
			case KeyboardKey::LeftBracket:
				ceguiScan = CEGUI::Key::Scan::LeftBracket;
			    break;
			case KeyboardKey::RightBracket:
				ceguiScan = CEGUI::Key::Scan::RightBracket;
			    break;
			case KeyboardKey::Semicolon:
				ceguiScan = CEGUI::Key::Scan::Semicolon;
			    break;
			case KeyboardKey::Quote:
				ceguiScan = CEGUI::Key::Scan::Colon;
			    break;
			case KeyboardKey::Comma:
				ceguiScan = CEGUI::Key::Scan::Comma;
			    break;
			case KeyboardKey::Period:
				ceguiScan = CEGUI::Key::Scan::Period;
			    break;
			case KeyboardKey::Slash:
				ceguiScan = CEGUI::Key::Scan::ForwardSlash;
			    break;
			case KeyboardKey::Backslash:
				ceguiScan = CEGUI::Key::Scan::Backslash;
			    break;
			case KeyboardKey::Tilde:
			    break;
			case KeyboardKey::F1:
				ceguiScan = CEGUI::Key::Scan::F1;
			    break;
			case KeyboardKey::F2:
			    ceguiScan = CEGUI::Key::Scan::F2;
			    break;
			case KeyboardKey::F3:
			    ceguiScan = CEGUI::Key::Scan::F3;
			    break;
			case KeyboardKey::F4:
			    ceguiScan = CEGUI::Key::Scan::F4;
			    break;
			case KeyboardKey::F5:
			    ceguiScan = CEGUI::Key::Scan::F5;
			    break;
			case KeyboardKey::F6:
				ceguiScan = CEGUI::Key::Scan::F6;
			    break;
			case KeyboardKey::F7:
				ceguiScan = CEGUI::Key::Scan::F7;
			    break;
			case KeyboardKey::F8:
				ceguiScan = CEGUI::Key::Scan::F8;
			    break;
			case KeyboardKey::F9:
				ceguiScan = CEGUI::Key::Scan::F9;
			    break;
			case KeyboardKey::F10:
				ceguiScan = CEGUI::Key::Scan::F10;
			    break;
			case KeyboardKey::F11:
				ceguiScan = CEGUI::Key::Scan::F11;
			    break;
			case KeyboardKey::F12:
				ceguiScan = CEGUI::Key::Scan::F12;
			    break;
			case KeyboardKey::NumPad0:
				ceguiScan = CEGUI::Key::Scan::Numpad_0;
			    break;
			case KeyboardKey::NumPad1:
			    ceguiScan = CEGUI::Key::Scan::Numpad_1;
			    break;
			case KeyboardKey::NumPad2:
			    ceguiScan = CEGUI::Key::Scan::Numpad_2;
			    break;
			case KeyboardKey::NumPad3:
			    ceguiScan = CEGUI::Key::Scan::Numpad_3;
			    break;
			case KeyboardKey::NumPad4:
			    ceguiScan = CEGUI::Key::Scan::Numpad_4;
			    break;
			case KeyboardKey::NumPad5:
			    ceguiScan = CEGUI::Key::Scan::Numpad_5;
			    break;
			case KeyboardKey::NumPad6:
			    ceguiScan = CEGUI::Key::Scan::Numpad_6;
			    break;
			case KeyboardKey::NumPad7:
			    ceguiScan = CEGUI::Key::Scan::Numpad_7;
			    break;
			case KeyboardKey::NumPad8:
			    ceguiScan = CEGUI::Key::Scan::Numpad_8;
			    break;
			case KeyboardKey::NumPad9:
			    ceguiScan = CEGUI::Key::Scan::Numpad_9;
			    break;
			case KeyboardKey::Key0:
				ceguiScan = CEGUI::Key::Scan::Zero;
			    break;
			case KeyboardKey::Key1:
				ceguiScan = CEGUI::Key::Scan::One;
			    break;
			case KeyboardKey::Key2:
				ceguiScan = CEGUI::Key::Scan::Two;
			    break;
			case KeyboardKey::Key3:
				ceguiScan = CEGUI::Key::Scan::Three;
			    break;
			case KeyboardKey::Key4:
				ceguiScan = CEGUI::Key::Scan::Four;
			    break;
			case KeyboardKey::Key5:
				ceguiScan = CEGUI::Key::Scan::Five;
			    break;
			case KeyboardKey::Key6:
				ceguiScan = CEGUI::Key::Scan::Six;
			    break;
			case KeyboardKey::Key7:
				ceguiScan = CEGUI::Key::Scan::Seven;
			    break;
			case KeyboardKey::Key8:
				ceguiScan = CEGUI::Key::Scan::Eight;
			    break;
			case KeyboardKey::Key9:
				ceguiScan = CEGUI::Key::Scan::Nine;
			    break;
			case KeyboardKey::KeyA:
				ceguiScan = CEGUI::Key::Scan::A;
			    break;
			case KeyboardKey::KeyB:
				ceguiScan = CEGUI::Key::Scan::B;
			    break;
			case KeyboardKey::KeyC:
				ceguiScan = CEGUI::Key::Scan::C;
			    break;
			case KeyboardKey::KeyD:
				ceguiScan = CEGUI::Key::Scan::D;
			    break;
			case KeyboardKey::KeyE:
				ceguiScan = CEGUI::Key::Scan::E;
			    break;
			case KeyboardKey::KeyF:
				ceguiScan = CEGUI::Key::Scan::F;
			    break;
			case KeyboardKey::KeyG:
				ceguiScan = CEGUI::Key::Scan::G;
			    break;
			case KeyboardKey::KeyH:
				ceguiScan = CEGUI::Key::Scan::H;
			    break;
			case KeyboardKey::KeyI:
				ceguiScan = CEGUI::Key::Scan::I;
			    break;
			case KeyboardKey::KeyJ:
				ceguiScan = CEGUI::Key::Scan::J;
			    break;
			case KeyboardKey::KeyK:
				ceguiScan = CEGUI::Key::Scan::K;
			    break;
			case KeyboardKey::KeyL:
				ceguiScan = CEGUI::Key::Scan::L;
			    break;
			case KeyboardKey::KeyM:
				ceguiScan = CEGUI::Key::Scan::M;
			    break;
			case KeyboardKey::KeyN:
				ceguiScan = CEGUI::Key::Scan::N;
			    break;
			case KeyboardKey::KeyO:
				ceguiScan = CEGUI::Key::Scan::O;
			    break;
			case KeyboardKey::KeyP:
				ceguiScan = CEGUI::Key::Scan::P;
			    break;
			case KeyboardKey::KeyQ:
				ceguiScan = CEGUI::Key::Scan::Q;
			    break;
			case KeyboardKey::KeyR:
				ceguiScan = CEGUI::Key::Scan::R;
			    break;
			case KeyboardKey::KeyS:
				ceguiScan = CEGUI::Key::Scan::S;
			    break;
			case KeyboardKey::KeyT:
				ceguiScan = CEGUI::Key::Scan::T;
			    break;
			case KeyboardKey::KeyU:
				ceguiScan = CEGUI::Key::Scan::U;
			    break;
			case KeyboardKey::KeyV:
				ceguiScan = CEGUI::Key::Scan::V;
			    break;
			case KeyboardKey::KeyW:
				ceguiScan = CEGUI::Key::Scan::W;
			    break;
			case KeyboardKey::KeyX:
				ceguiScan = CEGUI::Key::Scan::X;
			    break;
			case KeyboardKey::KeyY:
				ceguiScan = CEGUI::Key::Scan::Y;
			    break;
			case KeyboardKey::KeyZ:
				ceguiScan = CEGUI::Key::Scan::Z;
			    break;
		}
		if (down)
		{
			_guiContext->injectKeyDown(ceguiScan);
		}
		else
		{
			_guiContext->injectKeyUp(ceguiScan);
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
		updateRenderer();

		if (_guiContext)
		{
			_guiContext->injectTimePulse(deltaTime);
			if (_app)
			{
				auto& mouse = _app->getInput().getMouse();
				auto delta = mouse.getPositionDelta();
				_guiContext->injectMouseMove(delta.x, delta.y);
				delta = mouse.getScrollDelta();
				_guiContext->injectMouseWheelChange(delta.x);
			}
		}
	}

	bgfx::ViewId CeguiAppComponentImpl::render(bgfx::ViewId viewId) const
	{
		_renderer->setViewId(viewId);
		CEGUI::System::getSingleton().renderAllGUIContexts();
		return ++viewId;
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