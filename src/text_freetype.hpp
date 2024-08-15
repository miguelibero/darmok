#pragma once

#include <darmok/text.hpp>
#include <darmok/glm.hpp>
#include <darmok/texture.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/material.hpp>
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
        FreetypeFontLoaderImpl(IDataLoader& dataLoader, bx::AllocatorI& alloc, const glm::uvec2& defaultFontSize = glm::uvec2(48, 48));
        ~FreetypeFontLoaderImpl();
        void init(App& app);
        void shutdown();
        std::shared_ptr<IFont> operator()(std::string_view name);
    private:
        IDataLoader& _dataLoader;
        glm::uvec2 _defaultFontSize;
        FT_Library _library;
        bx::AllocatorI& _alloc;
    };

    class FreetypeFont final : public IFont
    {
    public:
        FreetypeFont(FT_Face face, Data&& data, FT_Library library, bx::AllocatorI& alloc) noexcept;
        ~FreetypeFont() noexcept;

        std::optional<Glyph> getGlyph(const Utf8Char& chr) const noexcept override;
        float getLineSize() const noexcept override;
        std::shared_ptr<Texture> getTexture() const override;
        void update(const std::unordered_set<Utf8Char>& chars) override;
        FT_Face getFace() const noexcept;
    private:
        std::shared_ptr<Texture> _texture;
        std::map<Utf8Char, Glyph> _glyphs;
        FT_Face _face;
        Data _data;
        FT_Library _library;
        bx::AllocatorI& _alloc;
        std::unordered_set<Utf8Char> _renderedChars;
    };

    struct FontAtlas
    {
        Image image;
        std::map<Utf8Char, Glyph> glyphs;
    };

    class FreetypeFontAtlasGenerator final
    {
    public:
        FreetypeFontAtlasGenerator(FT_Face face, FT_Library library, bx::AllocatorI& alloc) noexcept;
        FreetypeFontAtlasGenerator& setSize(const glm::uvec2& size) noexcept;
        FreetypeFontAtlasGenerator& setImageFormat(bimg::TextureFormat::Enum format) noexcept;
        FreetypeFontAtlasGenerator& setRenderMode(FT_Render_Mode mode) noexcept;
        glm::uvec2 calcSpace(const Utf8Vector& chars) noexcept;
        FontAtlas operator()(const Utf8Vector& chars);
        FontAtlas operator()(std::string_view str);
    private:
        FT_Face _face;
        FT_Library _library;
        bx::AllocatorI& _alloc;
        glm::uvec2 _size;
        FT_Render_Mode _renderMode;
        bimg::TextureFormat::Enum _imageFormat;

        std::map<Utf8Char, FT_UInt> getIndices(const Utf8Vector& chars) const;
        const FT_Bitmap& renderBitmap(FT_UInt index);
    };

    class FreetypeFontAtlasImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        FreetypeFontAtlasImporterImpl();
        ~FreetypeFontAtlasImporterImpl();
        bool startImport(const Input& input, bool dry = false);
        std::vector<std::filesystem::path> getOutputs(const Input& input);
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        const std::string& getName() const noexcept;
        void endImport(const Input& input);
    private:
        FT_Face _face;
        FT_Library _library;
        bx::DefaultAllocator _alloc;
        std::optional<FontAtlas> _atlas;
        std::filesystem::path _imagePath;
        std::filesystem::path _atlasPath;
    };

}