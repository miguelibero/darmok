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
	return CommandLineAssetImporter()(argc, argv);
}