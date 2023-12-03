/*
 * based on https://github.com/bkaradzic/bgfx/blob/master/examples/common/entry/entry.h
 */
#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <bx/bx.h>

extern "C" int _main_(int argc, char** argv);

#ifndef DARMOK_CONFIG_IMPLEMENT_MAIN
#	define DARMOK_CONFIG_IMPLEMENT_MAIN 0
#endif // ENTRY_CONFIG_IMPLEMENT_MAIN

#if DARMOK_CONFIG_IMPLEMENT_MAIN
#define DARMOK_IMPLEMENT_MAIN(app, ...)                      \
	int32_t _main_(int32_t argc, char** argv)                \
	{                                                        \
		return darmok::runApp<app>(argc, argv, __VA_ARGS__); \
	}
#else
#define DARMOK_IMPLEMENT_MAIN(app, ...) \
	static app s_app(__VA_ARGS__)
#endif // ENTRY_CONFIG_IMPLEMENT_MAIN

namespace darmok
{
	class InputState;
	
	class BX_NO_VTABLE App
	{
	public:
		App();
		virtual void init(const std::vector<std::string>& args);
		virtual int  shutdown();
		virtual bool update();

	protected:
		virtual void imguiDraw();
		virtual void update(const InputState& input);

		void toggleDebugFlag(uint32_t flag);
		void setDebugFlag(uint32_t flag, bool enabled = true);
	};

	int runApp(App& app, const std::vector<std::string>& args);

	template<typename T, typename... A>
	int runApp(int argc, const char* const* argv, A... constructArgs)
	{
		T app(std::forward(constructArgs)...);
		std::vector<std::string> args(argc);
		for (int i = 0; i < argc; ++i)
		{
			args[i] = argv[i];
		}
		return runApp(app, args);
	};
} // namespace darmok