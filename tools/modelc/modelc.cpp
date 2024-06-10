#include <iostream>
#include <fstream>
#include <filesystem>
#include <bx/commandline.h>
#include <bx/allocator.h>
#include <bx/file.h>
#include <darmok/string.hpp>
#include <darmok/model.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/image.hpp>
#include <darmok/vertex_layout.hpp>
#include <darmok/program.hpp>
#include <darmok/program_standard.hpp>
#include <darmok/model_assimp.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <nlohmann/json.hpp>

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

static void writeHeader(std::ostream& os, std::string_view varName, const Model& model)
{
	Data data;
	DataOutputStream::write(data, model);
	os << data.view().toHeader(varName);
}

static std::string getString(const char* ptr)
{
	return ptr == nullptr ? std::string() : std::string(ptr);
}

static void writeOutput(const Model& model, const std::string& output, std::string headerVarName)
{
	auto outExt = StringUtils::getPathExtension(output);

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

struct Config final
{
	const char* vertexLayoutJsonKey = "vertex_layout";
	const char* embedTexturesJsonKey = "embed_textures";

	bgfx::VertexLayout vertexLayout;
	bool embedTextures = true;

	Config()
	{
		vertexLayout = getDefaultVertexLayout();
	}

	void load(const std::string& path)
	{
		std::ifstream ifs(path);
		auto json = nlohmann::json::parse(ifs);
		vertexLayout = loadVertexLayout(json[vertexLayoutJsonKey]);
		if (json.contains(embedTexturesJsonKey))
		{
			embedTextures = json[embedTexturesJsonKey];
		}
		if (vertexLayout.getStride() == 0)
		{
			vertexLayout = getDefaultVertexLayout();
		}
	}
private:

	static bgfx::VertexLayout getDefaultVertexLayout()
	{
		return StandardProgramLoader::getVertexLayout(StandardProgramType::ForwardPhong);
	}

	static bgfx::VertexLayout loadVertexLayout(const nlohmann::ordered_json& json)
	{
		bgfx::VertexLayout layout;
		if (json.is_string())
		{
			std::string str = json;
			auto standard = StandardProgramLoader::getType(str);
			if (standard)
			{
				return StandardProgramLoader::getVertexLayout(standard.value());
			}
			VertexLayoutUtils::readFile(str, layout);
			return layout;
		}
		VertexLayoutUtils::readJson(json, layout);
		return layout;
	}
};

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
	//inputPath = "D:/Projects/darmok/samples/ozz/assets/BasicMotionsDummyModel.fbx";
	if (inputPath.empty())
	{
		help(name, "Input file path must be specified.");
		return bx::kExitFailure;
	}

	Config config;
	auto configPath = getString(cmdLine.findOption('c', "config"));
	//configPath = "D:/Projects/darmok/samples/ozz/assets/BasicMotionsDummyModel.model.json";
	if (!configPath.empty())
	{
		config.load(configPath);
	}

	bx::DefaultAllocator allocator;
	bx::FileReader fileReader;
	FileDataLoader dataLoader(fileReader, allocator);
	DataImageLoader imgLoader(dataLoader, allocator);
	auto optImgLoader = config.embedTextures ? OptionalRef<IImageLoader>(imgLoader) : nullptr;
	AssimpModelLoader assimpLoader(dataLoader, allocator, optImgLoader);
	assimpLoader.setVertexLayout(config.vertexLayout);

	auto model = assimpLoader(inputPath);

	auto outputPath = getString(cmdLine.findOption('o', "output"));
	//outputPath = "D:/Projects/darmok/build/samples/ozz/assets/BasicMotionsDummyModel.dml";
	std::string headerVarName = getString(cmdLine.findOption('b', "bin2c"));

	writeOutput(*model, outputPath, headerVarName);
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