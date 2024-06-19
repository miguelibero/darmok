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

static int run(const bx::CommandLine cmdLine)
{
	auto path = std::string(cmdLine.get(0));
	auto name = std::filesystem::path(path).filename().string();

	if (cmdLine.hasArg('h', "help"))
	{
		help(name);
		return bx::kExitFailure;
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
		help(name, "Input file path must be specified.");
		return bx::kExitFailure;
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
	try
	{
		argv = new const char* [] {
			argv[0],
				"-i", "../assets/shaders",
				"-o", "include/private/generated/shaders",
				"-c"
			};
		argc = 6;
		bx::CommandLine cmdLine(argc, argv);
		return run(cmdLine);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "exception thrown:" << std::endl;
		std::cerr << ex.what() << std::endl;
		return bx::kExitFailure;
	}
}