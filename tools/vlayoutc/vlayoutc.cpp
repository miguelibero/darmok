#include <darmok/vertex_layout.hpp>
#include <darmok/string.hpp>
#include <bx/commandline.h>
#include <cereal/archives/binary.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

void version(const std::string& name)
{
	std::cout << name << " vertex layout compiler tool." << std::endl;
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
	std::cout << "  -h, --help           Display this help and exit." << std::endl;
	std::cout << "  -v, --version        Output version information and exit." << std::endl;
	std::cout << "  -i <file path>       Input's file path (can be json or varyingdef)." << std::endl;
	std::cout << "  -o <file path>       Output's file path (if bin2c specified will generate header)." << std::endl;
}

void writeHeader(std::ostream& os, std::string arrayName, const bgfx::VertexLayout& layout)
{
	std::stringstream ss;
	cereal::BinaryOutputArchive archive(ss);
	archive(layout);

	ss.seekg(0, std::ios::end);
	size_t size = ss.tellg();
	ss.seekg(0, std::ios::beg);
	std::replace(arrayName.begin(), arrayName.end(), '.', '_');
	os << "static const uint8_t " << arrayName << "[" << size << "] = " << std::endl;
	os << "{" << std::endl;
	size_t i = 0;
	while (ss.peek() != EOF)
	{
		char buf;
		ss.read(&buf, 1);
		os << "0x" << darmok::StringUtils::binToHex(buf);
		if (i < size - 1)
		{
			os << ", ";
		}
		++i;
		if (i % 16 == 0)
		{
			os << std::endl;
		}
	}
	os << "};" << std::endl;
}

std::string getPathExtension(const std::string& path) noexcept
{
	return std::filesystem::path(path).extension().string();
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

	auto inExt = getPathExtension(input);

	std::ifstream ifs(input);
	bgfx::VertexLayout layout;

	if (inExt == ".json")
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
	std::string arrayName = cmdLine.findOption('b', "bin2c");

	if (!output)
	{
		if (!arrayName.empty())
		{
			writeHeader(std::cout, arrayName, layout);
		}
		else
		{
			std::cout << layout << std::endl;
		}
	}
	else
	{
		if (arrayName.empty())
		{
			auto outExt = getPathExtension(output);
			if (outExt == ".h" || outExt == ".hpp")
			{
				std::string outputStr(output);
				arrayName = outputStr.substr(0, outputStr.size() - outExt.size());
				arrayName += "_vlayout";
			}
		}
		if (!arrayName.empty())
		{
			std::ofstream os(output);
			writeHeader(os, arrayName, layout);
		}
		else
		{
			std::ofstream os(output, std::ios::binary);
			cereal::BinaryOutputArchive archive(os);
			archive(layout);
		}
	}

	return bx::kExitSuccess;
}