/*
 * based on https://github.com/bkaradzic/bgfx/blob/master/examples/common/entry/entry.h
 */
#pragma once

#include <darmok/input.hpp>

#include <string>
#include <memory>
#include <vector>
#include <bx/bx.h>

namespace bx { struct FileReaderI; struct FileWriterI; struct AllocatorI; }

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
	///
	bool processEvents();

	///
	bx::FileReaderI& getFileReader();

	///
	bx::FileWriterI& getFileWriter();

	///
	bx::AllocatorI& getAllocator();

	///
	void setCurrentDir(const std::string& dir);

	///
	void setDebugFlag(uint32_t flag, bool enabled = true);

	///
	bool getDebugFlag(uint32_t flag);

	/// 
	void setResetFlag(uint32_t flag, bool enabled = true);

	///
	bool getResetFlag(uint32_t flag);

	///
	uint32_t getResetFlags();

	///
	class BX_NO_VTABLE App
	{
	public:

		///
		virtual ~App() = 0;

		///
		virtual void init(const std::vector<std::string>& args) = 0;

		///
		virtual int  shutdown() = 0;

		///
		virtual bool update() = 0;

	};

	class BX_NO_VTABLE SimpleApp : public App
	{
	public:

		///
		void init(const std::vector<std::string>& args) override;

		///
		int  shutdown() override;

		///
		bool update() override;

	protected:
		NormMousePosition _lastMousePos;
		Utf8Char _lastChar;

		virtual void imguiDraw();
		virtual void draw();
	};

	///
	int runApp(std::unique_ptr<App>&& app, const std::vector<std::string>& args);

	template<typename T, typename... A>
	int runApp(int argc, const char* const* argv, A... constructArgs)
	{
		auto app = std::make_unique<T>(std::move(constructArgs)...);
		std::vector<std::string> args(argc);
		for (int i = 0; i < argc; ++i)
		{
			args[i] = argv[i];
		}
		return runApp(std::move(app), args);
	};
} // namespace darmok