#pragma once

#include <darmok/text.hpp>
#include <darmok/glm.hpp>
#include <darmok/texture.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/material.hpp>
#include <darmok/expected.hpp>
#include <darmok/protobuf/texture_atlas.pb.h>
#include <map>
#include <optional>
#include <string_view>
#include <bimg/bimg.h>

#include <ft2build.h>
#include FT_FREETYPE_H

struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;

namespace darmok
{
    class IDataLoader;
    class Image;

    class FreetypeFontLoaderImpl final
    {
    public:
        using Definition = protobuf::FreetypeFont;
		using Result = expected<std::shared_ptr<IFont>, std::string>;
        FreetypeFontLoaderImpl(bx::AllocatorI& alloc);
        ~FreetypeFontLoaderImpl();
        void init(App& app);
        void shutdown();
        Result create(const std::shared_ptr<Definition>& def);
    private:
        FT_Library _library;
        bx::AllocatorI& _alloc;
    };

    class FreetypeFont final : public IFont
    {
    public:
        using Definition = protobuf::FreetypeFont;

        FreetypeFont(const std::shared_ptr<Definition>& def, FT_Face face, FT_Library library, bx::AllocatorI& alloc) noexcept;
        ~FreetypeFont() noexcept;

        std::optional<Glyph> getGlyph(char32_t chr) const noexcept override;
        float getLineSize() const noexcept override;
        std::shared_ptr<Texture> getTexture() const override;
        [[nodiscard]] expected<void, std::string> update(const std::unordered_set<char32_t>& chars) override;
        FT_Face getFace() const noexcept;
    private:
        std::shared_ptr<Texture> _texture;
        std::map<char32_t, Glyph> _glyphs;
        std::shared_ptr<Definition> _def;
        FT_Face _face;
        FT_Library _library;
        std::unordered_set<char32_t> _renderedChars;
        bx::AllocatorI& _alloc;
    };

    class FreetypeFontAtlasGenerator final
    {
    public:
        using Atlas = protobuf::TextureAtlas;
		using Glyph = protobuf::TextureAtlasElement;

		struct ResultData final
		{
            Atlas atlas;
            Image image;
		};

		using Error = std::string;
		using Result = expected<ResultData, Error>;

        FreetypeFontAtlasGenerator(FT_Face face, FT_Library library, bx::AllocatorI& alloc) noexcept;
        FreetypeFontAtlasGenerator& setSize(const glm::uvec2& size) noexcept;
        FreetypeFontAtlasGenerator& setImageFormat(bimg::TextureFormat::Enum format) noexcept;
        FreetypeFontAtlasGenerator& setRenderMode(FT_Render_Mode mode) noexcept;
        glm::uvec2 calcSpace(std::u32string_view chars) noexcept;
        Result operator()(std::u32string_view chars);
        Result operator()(std::string_view str);
    private:
        FT_Face _face;
        FT_Library _library;
        bx::AllocatorI& _alloc;
        glm::uvec2 _size;
        FT_Render_Mode _renderMode;
        bimg::TextureFormat::Enum _imageFormat;

        std::map<char32_t, FT_UInt> getIndices(std::u32string_view chars) const;
        const FT_Bitmap& renderBitmap(FT_UInt index);
    };

    class FreetypeFontFileImporterImpl final
    {
    public:
        using Input = FileTypeImporterInput;
        FreetypeFontFileImporterImpl();
        ~FreetypeFontFileImporterImpl();
        bool startImport(const Input& input, bool dry = false);
        std::vector<std::filesystem::path> getOutputs(const Input& input);
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        const std::string& getName() const noexcept;
        void endImport(const Input& input);
    private:
        using Atlas = protobuf::TextureAtlas;
        FT_Face _face;
        FT_Library _library;
        bx::DefaultAllocator _alloc;
        FileDataLoader _dataLoader;
        DataTextureDefinitionLoader _texDefLoader;
        std::optional<Atlas> _atlas;
        std::optional<Image> _image;
        std::filesystem::path _imagePath;
        std::filesystem::path _atlasPath;
    };

}