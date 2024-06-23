#include <darmok/asset_core.hpp>

using namespace darmok;

class CommandLineAssetImporter final : public BaseCommandLineAssetImporter
{
protected:
	std::vector<std::filesystem::path> getOutputs(const Config& config) const override
	{
		return DarmokCoreAssetImporter(config).getOutputs();
	}

	void import(const Config& config, std::ostream & log) const override
	{
		return DarmokCoreAssetImporter(config)(log);
	}
};

int main(int argc, const char* argv[])
{
	/*
	argv = new const char* [] {
		argv[0],
			"-i", "../assets/shaders",
			"-o", "include/private/generated/shaders",
			"-c", "darmok-assetc-cache",
			"--bgfx-shaderc", "../../vcpkg/buildtrees/bgfx/x64-windows-rel/cmake/bgfx/shaderc.exe",
			"--bgfx-shader-include", "../../vcpkg/installed/x64-windows/include/bgfx/",
			"-d"
		};
	argc = 12;
	*/
	return CommandLineAssetImporter()(argc, argv);
}