#include <darmok/asset_core.hpp>

using namespace darmok;

class CommandLineFileImporter final : public BaseCommandLineFileImporter
{
protected:
	std::vector<std::filesystem::path> getOutputs(const Config& config) const override
	{
		return DarmokCoreAssetFileImporter(config).getOutputs();
	}

	void import(const Config& config, std::ostream & log) const override
	{
		return DarmokCoreAssetFileImporter(config)(log);
	}
};

int main(int argc, const char* argv[])
{
	return CommandLineFileImporter()(argc, argv);
}