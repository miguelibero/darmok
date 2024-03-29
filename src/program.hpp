#pragma once

#include <darmok/program.hpp>
#include <darmok/asset.hpp>
#include <string>

namespace darmok
{
	class IDataLoader;

    class DataProgramLoader final : public IProgramLoader
	{
	public:
		DataProgramLoader(IDataLoader& dataLoader, const std::string& vertexSuffix = "_vertex", const std::string& fragmentSuffix = "_fragment") noexcept;
		std::shared_ptr<Program> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
		std::string _vertexSuffix;
		std::string _fragmentSuffix;

		bgfx::ShaderHandle loadShader(const std::string& filePath);
	};
}