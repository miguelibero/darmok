#pragma once

#include <darmok/text.hpp>
#include <darmok/glm.hpp>
#include <darmok/texture.hpp>
#include <darmok/camera.hpp>
#include <darmok/app.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/string.hpp>
#include <unordered_set>
#include <unordered_map>
#include <optional>
#include <bx/allocator.h>
#include <string_view>
#include <pugixml.hpp>

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
        FreetypeFontLoaderImpl(IDataLoader& dataLoader, const glm::uvec2& defaultFontSize = glm::uvec2(48, 48));
        ~FreetypeFontLoaderImpl();
        void init(App& app);
        void shutdown();
        std::shared_ptr<Font> operator()(std::string_view name);
    private:
        IDataLoader& _dataLoader;
        glm::uvec2 _defaultFontSize;
        FT_Library _library;

    };

    class TextRendererImpl final
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept;
        void shutdown() noexcept;
        bool update();
        bgfx::ViewId afterRender(bgfx::ViewId viewId);
    private:
        OptionalRef<Scene> _scene;
        OptionalRef<bx::AllocatorI> _alloc;
    };

    class FontImpl final
    {
    public:
        FontImpl(FT_Face face, FT_Library library, Data&& data) noexcept;
        ~FontImpl() noexcept;
        void removeContent(std::string_view content);
        void addContent(std::string_view content);
        bool update(bx::AllocatorI& alloc);
        FT_Face getFace() const noexcept;
    private:
        std::optional<Texture> _tex;
        FT_Face _face;
        Data _data;
        FT_Library _library;
        std::unordered_set<Utf8Char> _renderedChars;
        std::unordered_set<Utf8Char> _chars;
    };

    class TextImpl final
    {
    public:
        TextImpl(const std::shared_ptr<Font>& font, const std::string& content = "") noexcept;

        std::shared_ptr<Font> getFont() noexcept;
        void setFont(const std::shared_ptr<Font>& font) noexcept;
        const std::string& getContent() noexcept;
        void setContent(const std::string& str) noexcept;
    private:
        std::shared_ptr<Font> _font;
        std::string _content;
    };

    struct FontAtlas
    {
        Image image;
        std::vector<TextureAtlasElement> elements;
    };

    class FreetypeFontAtlasGenerator final
    {
    public:
        FreetypeFontAtlasGenerator(FT_Face face, FT_Library library, bx::AllocatorI& alloc) noexcept;
        FreetypeFontAtlasGenerator& setSize(const glm::uvec2& size) noexcept;
        glm::uvec2 calcSpace(const std::unordered_set<Utf8Char>& chars) noexcept;
        FontAtlas operator()(const std::unordered_set<Utf8Char>& chars);
        FontAtlas operator()(std::string_view str);
    private:
        FT_Face _face;
        FT_Library _library;
        bx::AllocatorI& _alloc;
        glm::uvec2 _size;

        std::unordered_map<Utf8Char, FT_UInt> getIndices(const std::unordered_set<Utf8Char>& chars) const;
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