

#include <bx/uint32_t.h>
#include <darmok/entry.hpp>
#include <imgui/imgui.h>

namespace
{

class ExampleEmpty : public darmok::IApp
{
public:
	void init(int32_t argc, const char* const* argv, uint32_t width, uint32_t height) override
	{
		_width = width;
		_height = height;

		bgfx::Init init;
		init.platformData.nwh  = darmok::getNativeWindowHandle(darmok::kDefaultWindowHandle);
		init.platformData.ndt  = darmok::getNativeDisplayHandle();
		init.platformData.type = darmok::getNativeWindowHandleType(darmok::kDefaultWindowHandle);
		init.resolution.width  = _width;
		init.resolution.height = _height;
		init.resolution.reset  = BGFX_RESET_VSYNC;
		bgfx::init(init);

		// Enable debug text.
		//bgfx::setDebug(m_debug);

		// Set view 0 clear state.
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
			);

		imguiCreate();
	}

	virtual int shutdown() override
	{
		imguiDestroy();

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		if (!darmok::processEvents(_width, _height, _debug, _reset, &_mouseState) )
		{
			imguiBeginFrame(m_mouseState.m_mx
				,  _mouseState.y
				, (_mouseState.buttons[darmok::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (_mouseState.buttons[darmok::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (_mouseState.buttons[darmok::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  _mouseState.z
				, uint16_t(_width)
				, uint16_t(_height)
				);

			showExampleDialog(this);

			imguiEndFrame();

			// Set view 0 default viewport.
			bgfx::setViewRect(0, 0, 0, uint16_t(_width), uint16_t(_height) );

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);

			// Use debug font to print information about this example.
			bgfx::dbgTextClear();

			const bgfx::Stats* stats = bgfx::getStats();

			/*
			bgfx::dbgTextImage(
				  bx::max<uint16_t>(uint16_t(stats->textWidth/2), 20)-20
				, bx::max<uint16_t>(uint16_t(stats->textHeight/2),  6)-6
				, 40
				, 12
				, s_logo
				, 160
				);
			*/
			bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");

			bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
			bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

			bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters."
				, stats->width
				, stats->height
				, stats->textWidth
				, stats->textHeight
				);

			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			return true;
		}

		return false;
	}

	darmok::MouseState _mouseState;

	uint32_t _width;
	uint32_t _height;
	uint32_t _debug;
	uint32_t _reset;
};

}

ENTRY_IMPLEMENT_MAIN(ExampleEmpty);
