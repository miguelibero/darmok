#include <darmok/asset_core.hpp>
#include <darmok/asset.hpp>

using namespace darmok;

class CommandLineFileImporter final : public BaseCommandLineFileImporter
{
protected:
	expected<Paths, std::string> getOutputPaths(const Config& config) const noexcept override
	{
		return DarmokAssetFileImporter(config).getOutputPaths();
	}

	void import(const Config& config, std::ostream& log) const noexcept override
	{
		DarmokAssetFileImporter{ config }(log);
	}
};

int main(int argc, const char* argv[])
{
	return CommandLineFileImporter()(CmdArgs(argv, argc));
}