#pragma once

#include <memory>
#include <darmok/program_fwd.hpp>

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
	private:
		std::unique_ptr<StandardProgramLoaderImpl> _impl;
	};
}