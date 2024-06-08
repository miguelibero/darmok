#pragma once

#include <darmok/program.hpp>
#include <darmok/asset.hpp>
#include <string>

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
}