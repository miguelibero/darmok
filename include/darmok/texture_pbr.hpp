#pragma once

#include <darmok/texture.hpp>
#include <darmok/asset_core.hpp>

namespace darmok
{
    namespace PbrTextureUtils
    {
        using Definition = Texture::Definition;
        [[nodiscard]] Definition createAlbedoLutDefinition(uint16_t size = 256, uint32_t sampleCount = 1024);
        [[nodiscard]] Definition createBrdfLutDefinition(uint16_t size = 256, uint32_t sampleCount = 1024);
    }

    class DARMOK_EXPORT GeneratedTexturesFileImporter final : public IFileTypeImporter
    {
      public:
        expected<Effect, std::string> prepare(const Input& input) noexcept override;
        expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;

        const std::string& getName() const noexcept override;

      private:
        struct TextureConfig final
        {
            std::string outputPath;
            uint16_t size = 256;
            uint16_t sampleCount = 1024;

            void parse(const nlohmann::json& json) noexcept;
        };

        enum class TextureType
        {
            AlbedoLut,
            BrdfLut
        };

        std::unordered_map<TextureType, TextureConfig> _textures;
        using OutputFormat = protobuf::Format;
        OutputFormat _outputFormat = OutputFormat::Binary;
    };
}
