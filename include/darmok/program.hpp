#pragma once


#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string_view>
#include <rapidjson/document.h>

namespace darmok
{
	class Texture;

	enum class StandardProgramType
	{
		Gui,
		Unlit,
		ForwardPhong,
		ForwardPbr,
	};

	class Data;

	class Program final
	{
	public:
		Program(const bgfx::ProgramHandle& handle, const bgfx::VertexLayout& layout) noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;
		~Program() noexcept;
		[[nodiscard]] const bgfx::ProgramHandle& getHandle() const noexcept;
		const bgfx::VertexLayout& getVertexLayout() const noexcept;

		static void readVertexLayoutJson(const rapidjson::Document& doc, bgfx::VertexLayout& layout) noexcept;
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

	class StandardProgramLoader final
	{
	public:
		using result_type = std::shared_ptr<Program>;
		virtual result_type operator()(StandardProgramType type) noexcept;
	};
}