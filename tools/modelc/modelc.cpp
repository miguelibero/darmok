#include <iostream>
#include <filesystem>
#include <bx/commandline.h>
#include <darmok/model_assimp.hpp>

using namespace darmok;

static void version(const std::string& name)
{
	std::cout << name << ": darmok model compiler tool." << std::endl;
}

static void help(const std::string& name, const char* error = nullptr)
{
	if (error)
	{
		std::cerr << "Error:" << std::endl << error << std::endl << std::endl;
	}

	version(name);

	std::cout << "Usage: " << name << " -i <in> -o <out> --config <config json>" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help					Display this help and exit." << std::endl;
	std::cout << "  -v, --version				Output version information and exit." << std::endl;
	std::cout << "  -i, --input <file path>		Input's file path (can be anything supported by assimp)." << std::endl;
	std::cout << "  -o, --output <file path>	Output's file path (header, binary, json or xml depending on extension)." << std::endl;
	std::cout << "  -c, --config <file path>	Config's file path (json format)." << std::endl;
	std::cout << "  -b, --bin2c <name>			Output's header variable name." << std::endl;
}

static std::string getString(const char* ptr)
{
	return ptr == nullptr ? std::string() : std::string(ptr);
}

static int run(const bx::CommandLine cmdLine)
{
	auto path = getString(cmdLine.get(0));
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

	auto inputPath = getString(cmdLine.findOption('i', "input"));
	if (inputPath.empty())
	{
		help(name, "Input file path must be specified.");
		return bx::kExitFailure;
	}

	AssimpModelProcessor processor(inputPath);

	auto configPath = getString(cmdLine.findOption('c', "config"));
	processor.setHeaderVarName(cmdLine.findOption('b', "bin2c"));
	if (!configPath.empty())
	{
		processor.setConfigFromFile(configPath);
	}

	auto outputPath = getString(cmdLine.findOption('o', "output"));
	if (!outputPath.empty())
	{
		processor.writeFile(outputPath);
	}
	else
	{
		std::cout << processor << std::endl;
	}

	return bx::kExitSuccess;
}

int main(int argc, const char* argv[])
{
	try
	{
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