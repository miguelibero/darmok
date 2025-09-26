#include <darmok/asset_core.hpp>

using namespace darmok;

class CommandLineFileImporter final : public BaseCommandLineFileImporter
{
protected:
	expected<Paths, std::string> getOutputPaths(const Config& config) const noexcept override
	{
		return DarmokCoreAssetFileImporter(config).getOutputPaths();
	}

	void import(const Config& config, std::ostream& log) const noexcept override
	{
		return DarmokCoreAssetFileImporter(config)(log);
	}
};

int main(int argc, const char* argv[])
{
	return CommandLineFileImporter()(CmdArgs(argv, argc));
}