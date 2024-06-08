#include <darmok/vertex_layout.hpp>
#include <darmok/string.hpp>
#include <darmok/data_stream.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <bx/commandline.h>
#include <filesystem>
#include <fstream>
#include <iostream>

void version(const std::string& name)
{
	std::cout << name << ": darmok vertex layout compiler tool." << std::endl;
}

void help(const std::string& name, const char* error = nullptr)
{
	if (error)
	{
		std::cerr << "Error:" << std::endl << error << std::endl << std::endl;
	}

	version(name);

	std::cout << "Usage: " << name << " -i <in> -o <out> --bin2c <array name>" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help					Display this help and exit." << std::endl;
	std::cout << "  -v, --version				Output version information and exit." << std::endl;
	std::cout << "  -i, --input <file path>		Input's file path (can be json or varyingdef)." << std::endl;
	std::cout << "  -o, --output <file path>	Output's file path (header, binary, json or xml depending on extension)." << std::endl;
	std::cout << "  -b, --bin2c <name>			Output's header variable name." << std::endl;
}

void writeHeader(std::ostream& os, std::string_view varName, const bgfx::VertexLayout& layout)
{
	darmok::Data data;
	darmok::DataOutputStream::write(data, layout);
	os << data.view().toHeader(varName);
}

std::string getPathExtension(const std::string& path) noexcept
{
	return std::filesystem::path(path).extension().string();
}

std::string getString(const char* ptr)
{
	return ptr == nullptr ? std::string() : std::string(ptr);
}

void writeOutput(const bgfx::VertexLayout& layout, const std::string& output, std::string headerVarName)
{
	auto outExt = getPathExtension(output);

	if (headerVarName.empty())
	{
		if (outExt == ".h" || outExt == ".hpp")
		{
			std::string outputStr(output);
			headerVarName = outputStr.substr(0, outputStr.size() - outExt.size());
			headerVarName += "_vlayout";
		}
	}

	if (output.empty())
	{
		if (!headerVarName.empty())
		{
			writeHeader(std::cout, headerVarName, layout);
		}
		else
		{
			std::cout << layout << std::endl;
		}
		return;
	}
	if (!headerVarName.empty())
	{
		std::ofstream os(output);
		writeHeader(os, headerVarName, layout);
	}
	else if (outExt == ".json")
	{
		std::ofstream os(output);
		cereal::JSONOutputArchive archive(os);
		archive(layout);
	}
	else if (outExt == ".xml")
	{
		std::ofstream os(output);
		cereal::XMLOutputArchive archive(os);
		archive(layout);
	}
	else
	{
		std::ofstream os(output, std::ios::binary);
		cereal::BinaryOutputArchive archive(os);
		archive(layout);
	}
}

int main(int argc, const char* argv[])
{
    bx::CommandLine cmdLine(argc, argv);
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

	auto input = getString(cmdLine.findOption('i', "input"));
	if (input.empty())
	{
		help(name, "Input file path must be specified.");
		return bx::kExitFailure;
	}

	auto inExt = getPathExtension(input);

	std::ifstream ifs(input);
	bgfx::VertexLayout layout;
	layout.begin().end();

	if (inExt == ".json")
	{
		auto json = nlohmann::ordered_json::parse(ifs);
		darmok::VertexLayoutUtils::readJson(json, layout);
	}
	else
	{
		std::ostringstream content;
		content << ifs.rdbuf();
		darmok::VertexLayoutUtils::readVaryingDef(content.str(), layout);
	}

	auto output = getString(cmdLine.findOption('o', "output"));
	auto headerVarName = getString(cmdLine.findOption('b', "bin2c"));

	writeOutput(layout, output, headerVarName);
	return bx::kExitSuccess;
}