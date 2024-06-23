#include <darmok/asset_core.hpp>
#include <darmok/asset.hpp>

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
	std::optional<DarmokAssetImporter> _importer;
};

int main(int argc, const char* argv[])
{
	/*
	argv = new const char* [] {
		argv[0],
			"-i", "D:/Projects/darmok/samples/scene/assets",
			"-o", "D:/Projects/darmok/build/samples/scene/assets",
			"-d"
		};

	argc = 6;
	*/

	return CommandLineAssetImporter()(argc, argv);
}