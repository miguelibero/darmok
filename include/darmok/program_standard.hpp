#pragma once

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

	class StandardProgramLoader final
	{
	public:
		using result_type = std::shared_ptr<Program>;

		DLLEXPORT StandardProgramLoader() noexcept;
		DLLEXPORT ~StandardProgramLoader() noexcept;
		DLLEXPORT virtual result_type operator()(StandardProgramType type) noexcept;
		DLLEXPORT static std::optional<StandardProgramType> getType(std::string_view name) noexcept;
		DLLEXPORT static bgfx::VertexLayout getVertexLayout(StandardProgramType type) noexcept;
	private:
		std::unique_ptr<StandardProgramLoaderImpl> _impl;
	};
}