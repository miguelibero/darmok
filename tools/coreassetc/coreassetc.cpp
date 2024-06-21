#include <darmok/asset_core.hpp>

using namespace darmok;

class CommandLineAssetImporter final : public BaseCommandLineAssetImporter
{
protected:
	void setInputPath(const std::filesystem::path& inputPath) override
	{
		_importer.emplace(inputPath);
	}

	void setOutputPath(const std::filesystem::path& outputPath) override
	{
		_importer->setOutputPath(outputPath);
	}

	void setShadercPath(const std::filesystem::path& path) override
	{
		_importer->setShadercPath(path);
	}

	void addShaderIncludePath(const std::filesystem::path& path) override
	{
		_importer->addShaderIncludePath(path);
	}

	std::vector<std::filesystem::path> getOutputs() const override
	{
		return _importer->getOutputs();
	}

	void import(std::ostream& log) const override
	{
		return _importer.value()(log);
	}
private:
	std::optional<DarmokCoreAssetImporter> _importer;
};

int main(int argc, const char* argv[])
{
	/*
	argv = new const char* [] {
		argv[0],
			"-i", "../assets/shaders",
			"-o", "include/private/generated/shaders",
			"--bgfx-shaderc", "../../vcpkg/buildtrees/bgfx/x64-windows-rel/cmake/bgfx/shaderc.exe",
			"--bgfx-shader-include", "../../vcpkg/installed/x64-windows/include/bgfx/",
			"-d"
		};
	argc = 9;
	*/

	return CommandLineAssetImporter()(argc, argv);
}