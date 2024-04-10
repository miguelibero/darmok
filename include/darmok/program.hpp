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
		Unlit,
		ForwardPhong,
	};

	class Program final
	{
	public:
		Program(const bgfx::ProgramHandle& handle, const bgfx::VertexLayout& layout) noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;
		~Program() noexcept;
		[[nodiscard]] const bgfx::ProgramHandle& getHandle() const noexcept;
		const bgfx::VertexLayout& getVertexLayout() const noexcept;

		static std::shared_ptr<Program> createStandard(StandardProgramType type) noexcept;
		static void readVertexLayoutJson(std::string_view json, bgfx::VertexLayout& layout) noexcept;
		static bgfx::Attrib::Enum getBgfxAttrib(const std::string_view name) noexcept;
		static bgfx::AttribType::Enum getBgfxAttribType(const std::string_view name) noexcept;

	private:
		bgfx::ProgramHandle _handle;
		bgfx::VertexLayout _layout;
	};

	class BX_NO_VTABLE IProgramLoader
	{
	public:
		using result_type = std::shared_ptr<Program>;

		virtual ~IProgramLoader() = default;
		virtual result_type operator()(std::string_view name) = 0;
	};
}