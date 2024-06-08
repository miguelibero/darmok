#include <bx/commandline.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <darmok/model.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/image.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include "assimp.hpp"
#include "assimp_model.hpp"
#include <assimp/Importer.hpp>
#include <bx/allocator.h>

void version(const std::string& name)
{
	std::cout << name << ": darmok model compiler tool." << std::endl;
}

void help(const std::string& name, const char* error = nullptr)
{
	if (error)
	{
		std::cerr << "Error:" << std::endl << error << std::endl << std::endl;
	}

	version(name);

	std::cout << "Usage: " << name << " -i <in> -o <out> --config <config json>" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help           Display this help and exit." << std::endl;
	std::cout << "  -v, --version        Output version information and exit." << std::endl;
	std::cout << "  -i <file path>       Input's file path (can be any format supported by assimp)." << std::endl;
	std::cout << "  -o <file path>       Output's file path (header, binary, json or xml depending on extension)." << std::endl;
}

void writeHeader(std::ostream& os, std::string_view varName, const darmok::Model& model)
{
	darmok::Data data;
	darmok::DataOutputStream::write(data, model);
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

void writeOutput(const darmok::Model& model, const std::string& output, std::string headerVarName)
{
	auto outExt = getPathExtension(output);

	if (headerVarName.empty())
	{
		if (outExt == ".h" || outExt == ".hpp")
		{
			std::string outputStr(output);
			headerVarName = outputStr.substr(0, outputStr.size() - outExt.size());
			headerVarName += "_model";
		}
	}

	if (output.empty())
	{
		if (!headerVarName.empty())
		{
			writeHeader(std::cout, headerVarName, model);
		}
		else
		{
			std::cout << model << std::endl;
		}
		return;
	}
	if (!headerVarName.empty())
	{
		std::ofstream os(output);
		writeHeader(os, headerVarName, model);
	}
	else if (outExt == ".json")
	{
		std::ofstream os(output);
		cereal::JSONOutputArchive archive(os);
		archive(model);
	}
	else if (outExt == ".xml")
	{
		std::ofstream os(output);
		cereal::XMLOutputArchive archive(os);
		archive(model);
	}
	else
	{
		std::ofstream os(output, std::ios::binary);
		cereal::BinaryOutputArchive archive(os);
		archive(model);
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

	auto config = getString(cmdLine.findOption('c', "config"));
	if (!config.empty())
	{
		help(name, "Input file path must be specified.");
		return bx::kExitFailure;
	}

	Assimp::Importer assimpImporter;
	auto data = darmok::Data::fromFile(input);
	darmok::AssimpScene assimpScene(assimpImporter, data.view(), input);

	darmok::Model model;
	bx::DefaultAllocator allocator;
	bgfx::VertexLayout vertexLayout;
	// TODO: load vertex layout from config

	darmok::AssimpModelUpdater updater(assimpScene, allocator, vertexLayout);
	updater.setPath(input);
	updater.run(model);

	auto output = getString(cmdLine.findOption('o', "output"));
	std::string headerVarName = getString(cmdLine.findOption('b', "bin2c"));

	writeOutput(model, output, headerVarName);
	return bx::kExitSuccess;
}