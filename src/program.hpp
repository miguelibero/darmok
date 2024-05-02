#pragma once

#include <darmok/program.hpp>
#include <darmok/asset.hpp>
#include <string>
#include <unordered_map>

namespace darmok
{
	class IDataLoader;
	class IVertexLayoutLoader;

    class DataProgramLoader final : public IProgramLoader
	{
	public:
		struct Suffixes
		{
			std::string vertex;
			std::string fragment;
			std::string vertexLayout;
		};

		static const Suffixes defaultSuffixes;

		DataProgramLoader(IDataLoader& dataLoader, IVertexLayoutLoader& vertexLayoutLoader, Suffixes suffixes = defaultSuffixes) noexcept;
		std::shared_ptr<Program> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
		IVertexLayoutLoader& _vertexLayoutLoader;
		Suffixes _suffixes;

		bgfx::ShaderHandle loadShader(const std::string& filePath);
	};

	class StandardProgramLoaderImpl final
	{
	public:
		std::shared_ptr<Program> operator()(StandardProgramType type) const noexcept;
	private:
		static const bgfx::EmbeddedShader _embeddedShaders[];
		static const std::unordered_map<StandardProgramType, std::string> _embeddedShaderNames;
		static const std::unordered_map<StandardProgramType, std::string_view> _embeddedShaderVertexLayouts;
	};
}