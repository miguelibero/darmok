
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

}