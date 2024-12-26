#pragma once

#include <darmok/texture.hpp>

#include <filesystem>

#include <nlohmann/json.hpp>

namespace darmok
{
    struct TextureImportConfig final
	{
		TextureConfig config;
		std::filesystem::path image;
		uint64_t loadFlags = defaultTextureLoadFlags;

		void load(const FileTypeImporterInput& input);
	private:
		void read(const nlohmann::json& json, std::filesystem::path basePath);
	};

	/*
	class TextureImporterImpl final
	{
	public:
		using Input = AssetTypeImporterInput;
		using Dependencies = AssetImportDependencies;
		using Config = TextureImportConfig;

		TextureImporterImpl();

		bool startImport(const Input& input, bool dry = false);
		std::vector<std::filesystem::path> getOutputs(const Input& input);
		Dependencies getDependencies(const Input& input);
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
		void endImport(const Input& input);

		const std::string& getName() const noexcept;
	private:
	};
	*/
}