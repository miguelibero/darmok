#include <darmok/asset.hpp>
#include <bx/commandline.h>
#include <filesystem>
#include <iostream>

using namespace darmok;

static void version(const std::string& name)
{
	std::cout << name << ": darmok asset compile tool." << std::endl;
}

static void help(const std::string& name, const char* error = nullptr)
{
	if (error)
	{
		std::cerr << "Error:" << std::endl << error << std::endl << std::endl;
	}

	version(name);

	std::cout << "Usage: " << name << " -i <in> -o <out>" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help				Display this help and exit." << std::endl;
	std::cout << "  -v, --version			Output version information and exit." << std::endl;
	std::cout << "  -i, --input <path>		Input file path (can be a file or a directory)." << std::endl;
	std::cout << "  -o, --output <path>		Output file path (can be a file or a directory)." << std::endl;
	std::cout << "  -d, --dry				Do not process assets, just print output files." << std::endl;
	std::cout << "  -c, --header <prefix>	Output headers with the given prefix in the variable names." << std::endl;
}

static int run(const bx::CommandLine cmdLine, const std::string& name)
{
	if (cmdLine.hasArg('h', "help"))
	{
		help(name);
		return bx::kExitSuccess;
	}

	if (cmdLine.hasArg('v', "version"))
	{
		version(name);
		return bx::kExitSuccess;
	}

	const char* inputPath = nullptr;
	cmdLine.hasArg(inputPath, 'i', "input");
	if (inputPath == nullptr)
	{
		throw std::runtime_error("Input file path must be specified.");
	}

	DarmokAssetProcessor processor(inputPath);

	const char* outputPath = nullptr;
	cmdLine.hasArg(outputPath, 'o', "output");
	if (outputPath != nullptr)
	{
		processor.setOutputPath(outputPath);
	}

	const char* headerVarPrefix = nullptr;
	cmdLine.hasArg(headerVarPrefix, 'c', "header");
	if (headerVarPrefix != nullptr)
	{
		processor.setProduceHeaders(true);
		processor.setHeaderVarPrefix(headerVarPrefix);
	}
	else if (cmdLine.hasArg('c', "header"))
	{
		processor.setProduceHeaders(true);
	}

	if (cmdLine.hasArg('d', "dry"))
	{
		for (auto& output : processor.getOutputs())
		{
			std::cout << output.string() << std::endl;
		}
	}
	else
	{
		processor(std::cout);
	}
	return bx::kExitSuccess;
}

int main(int argc, const char* argv[])
{
	argv = new const char* [] {
		argv[0],
			"-i", "../samples/assimp/assets",
			"-o", "samples/assimp/assets",
			"-d"
		};
	argc = 6;

	bx::CommandLine cmdLine(argc, argv);
	auto path = std::string(cmdLine.get(0));
	auto name = std::filesystem::path(path).filename().string();

	try
	{
		return run(cmdLine, name);
	}
	catch (const std::exception& ex)
	{
		help(name, ex.what());
		return bx::kExitFailure;
	}
}