#include <darmok/asset_core.hpp>
#include <darmok/asset.hpp>

using namespace darmok;

class CommandLineFileImporter final : public BaseCommandLineFileImporter
{
protected:
	std::vector<std::filesystem::path> getOutputs(const Config& config) const override
	{
		return DarmokAssetFileImporter(config).getOutputs();
	}

	void import(const Config & config, std::ostream& log) const override
	{
		return DarmokAssetFileImporter(config)(log);
	}
};

int main(int argc, const char* argv[])
{
	return CommandLineFileImporter()(CmdArgs(argv, argc));
}