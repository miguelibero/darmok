

#include <bx/uint32_t.h>
#include <darmok/entry.hpp>

namespace
{

class ExampleEmpty : public darmok::SimpleApp
{
public:
	void init(const std::vector<std::string>& args) override
	{
		SimpleApp::init(args);

		// Enable debug text.
		darmok::setDebugFlag(BGFX_DEBUG_TEXT);

		// Set view 0 clear state.
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
			);
	}

	void draw() override
	{
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
	}
};

}

DARMOK_IMPLEMENT_MAIN(ExampleEmpty);
