#pragma once


#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string_view>

namespace darmok
{
	class Texture;

	enum class StandardProgramType
	{
		Sprite,
		Unlit,
		ForwardPhong,
	};

	class Program final
	{
	public:
		Program(const bgfx::ProgramHandle& handle) noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;
		~Program() noexcept;
		[[nodiscard]] const bgfx::ProgramHandle& getHandle() const noexcept;

		static std::shared_ptr<Program> createStandard(StandardProgramType type) noexcept;

	private:
		bgfx::ProgramHandle _handle;
	};

	class BX_NO_VTABLE IProgramLoader
	{
	public:
		using result_type = std::shared_ptr<Program>;

		virtual ~IProgramLoader() = default;
		virtual result_type operator()(std::string_view name) = 0;
	};
}