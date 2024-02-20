#pragma once

#include <bgfx/bgfx.h>
#include <bx/bx.h>

#include <memory>
#include <string_view>

namespace darmok
{
	enum class EmbeddedProgramType
	{
		Basic,
		Sprite,
		Debug,
	};

    struct Program final
	{
		Program(const bgfx::ProgramHandle& handle);
        ~Program();
		const bgfx::ProgramHandle& getHandle() const;
	private:
		bgfx::ProgramHandle _handle;
	};

	class EmbeddedProgramLoader final
	{
	public:
		std::shared_ptr<Program> operator()(EmbeddedProgramType type);
	};

    class BX_NO_VTABLE IProgramLoader
	{
	public:
		virtual ~IProgramLoader() = default;
		virtual std::shared_ptr<Program> operator()(std::string_view vertexName, std::string_view fragmentName = "") = 0;
	};
}