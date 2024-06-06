#include <darmok/vertex_layout.hpp>
#include <bx/commandline.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cereal/archives/binary.hpp>

void version(const std::string& name)
{
	std::cout << name << " vertex layout compiler tool." << std::endl;
}

void help(const std::string& name, const char* error = nullptr)
{
	if (NULL != error)
	{
		std::cerr << "Error:" << std::endl << error << std::endl << std::endl;
	}

	version(name);

	std::cout << "Usage: " << name << " -i <in> -o <out> --bin2c <array name>" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help           Display this help and exit." << std::endl;
	std::cout << "  -v, --version        Output version information and exit." << std::endl;
	std::cout << "  -i <file path>       Input's file path (can be json or varyingdef)." << std::endl;
	std::cout << "  -o <file path>       Output's file path (if bin2c specified will generate header)." << std::endl;
}

int main(int argc, const char* argv[])
{
    bx::CommandLine cmdLine(argc, argv);
	auto name = std::filesystem::path(cmdLine.get(0)).filename().string();

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

	const char* input = cmdLine.findOption('i', "input");
	if (!input)
	{
		help(name, "Input file path must be specified.");
		return bx::kExitFailure;
	}

	auto ext = std::filesystem::path(input).extension().string();

	std::ifstream ifs(input);
	bgfx::VertexLayout layout;

	if (ext == "json")
	{
		auto json = nlohmann::json::parse(ifs);
		darmok::VertexLayoutUtils::readJson(json, layout);
	}
	else
	{
		std::ostringstream content;
		content << ifs.rdbuf();
		darmok::VertexLayoutUtils::readVaryingDef(content.str(), layout);
	}

	const char* output = cmdLine.findOption('o', "output");
	if (!output)
	{
		std::cout << layout << std::endl;
	}
	else
	{
		std::ofstream os(output, std::ios::binary);
		cereal::BinaryOutputArchive archive(os);
		archive(layout);
	}

	return bx::kExitSuccess;
}