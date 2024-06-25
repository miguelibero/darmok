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
	return CommandLineAssetImporter()(argc, argv);
}