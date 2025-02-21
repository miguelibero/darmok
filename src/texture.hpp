#pragma once

#include <darmok/texture.hpp>

#include <filesystem>

#include <nlohmann/json.hpp>

namespace darmok
{
    struct TextureFileImportConfig final
	{
		TextureConfig config;
		uint64_t flags = defaultTextureLoadFlags;
		bimg::TextureFormat::Enum imageFormat = bimg::TextureFormat::Count;

		void load(const FileTypeImporterInput& input);
	private:
		void read(const nlohmann::json& json, std::filesystem::path basePath);
	};

	class TextureFileImporterImpl final
	{
	public:
		using Input = FileTypeImporterInput;
		using Dependencies = FileTypeImportDependencies;
		using Config = TextureFileImportConfig;

		bool startImport(const Input& input, bool dry = false);
		std::vector<std::filesystem::path> getOutputs(const Input& input);
		Dependencies getDependencies(const Input& input);
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
		void endImport(const Input& input);

		const std::string& getName() const noexcept;
	private:
		std::filesystem::path _outputPath;
		std::optional<Config> _importConfig;
		bx::DefaultAllocator _alloc;
	};

}