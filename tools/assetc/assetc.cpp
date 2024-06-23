#include <darmok/asset_core.hpp>
#include <darmok/asset.hpp>

using namespace darmok;

class CommandLineAssetImporter final : public BaseCommandLineAssetImporter
{
protected:
	std::vector<std::filesystem::path> getOutputs(const Config& config) const override
	{
		return DarmokAssetImporter(config).getOutputs();
	}

	void import(const Config & config, std::ostream& log) const override
	{
		return DarmokAssetImporter(config)(log);
	}
};

int main(int argc, const char* argv[])
{
	/*
	argv = new const char* [] {
		argv[0],
			"-i", "D:/Projects/darmok/samples/assimp/assets",
			"-o", "D:/Projects/darmok/build/samples/assimp/assets",
			"-c", "D:/Projects/darmok/build/darmok-assetc-cache"
		};
	argc = 7;
	*/

	return CommandLineAssetImporter()(argc, argv);
}