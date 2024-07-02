#pragma once

#include <darmok/export.h>
#include <memory>
#include <optional>
#include <string_view>
#include <darmok/program_fwd.hpp>

namespace bgfx
{
	struct VertexLayout;
}

namespace darmok
{
    class Program;
    class StandardProgramLoaderImpl;

	class DARMOK_EXPORT StandardProgramLoader final
	{
	public:
		using result_type = std::shared_ptr<Program>;

		StandardProgramLoader() noexcept;
		~StandardProgramLoader() noexcept;
		virtual result_type operator()(StandardProgramType type);
		static std::optional<StandardProgramType> getType(std::string_view name) noexcept;
		static bgfx::VertexLayout getVertexLayout(StandardProgramType type) noexcept;
		static result_type getProgram(StandardProgramType type);
	private:
		std::unique_ptr<StandardProgramLoaderImpl> _impl;
	};
}