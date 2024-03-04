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
		DataProgramLoader(IDataLoader& dataLoader);
		std::shared_ptr<Program> operator()(std::string_view vertexName, std::string_view fragmentName = "") override;
	private:
		IDataLoader& _dataLoader;

		bgfx::ShaderHandle loadShader(const std::string& filePath);
		static std::string getShaderExt();
	};
}