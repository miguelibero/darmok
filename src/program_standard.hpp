
#pragma once

#include <darmok/program_fwd.hpp>
#include <darmok/program_standard.hpp>
#include <unordered_map>
#include <string>

namespace bgfx
{
	struct EmbeddedShader;
	struct VertexLayout;
}

namespace darmok
{
	class DataView;
	
    class StandardProgramLoaderImpl final
	{
	public:
		std::shared_ptr<Program> operator()(StandardProgramType type) const;
		static std::optional<StandardProgramType> getType(std::string_view name) noexcept;
		static bgfx::VertexLayout getVertexLayout(StandardProgramType type) noexcept;
		static std::shared_ptr<Program> getProgram(StandardProgramType type);
	private:
		static const bgfx::EmbeddedShader _embeddedShaders[];
		static const std::unordered_map<StandardProgramType, std::string> _embeddedShaderNames;
		static const std::unordered_map<StandardProgramType, DataView> _embeddedShaderVertexLayouts;
	};
}