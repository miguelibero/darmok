#pragma once

#include <darmok/text.hpp>
#include <darmok/glm.hpp>
#include <darmok/texture.hpp>
#include <darmok/camera.hpp>
#include <unordered_set>
#include <optional>
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
        std::shared_ptr<Font> operator()(std::string_view name);
    private:
        IDataLoader& _dataLoader;
        FT_Library _library;
        glm::uvec2 _defaultFontSize;
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
        FontImpl(FT_Face face, Data&& data) noexcept;
        ~FontImpl() noexcept;
        void removeContent(std::string_view content);
        void addContent(std::string_view content);
        bool update(bx::AllocatorI& alloc);
        FT_Face getFace() const noexcept;
    private:
        std::optional<Texture> _tex;
        FT_Face _face;
        Data _data;
        std::unordered_set<FT_ULong> _renderedChars;
        std::unordered_set<FT_ULong> _chars;
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

    class FreetypeFontAtlasGenerator final
    {
    public:
        FreetypeFontAtlasGenerator(FT_Face face, bx::AllocatorI& alloc) noexcept;
        FreetypeFontAtlasGenerator& setSize(const glm::uvec2& size) noexcept;
        glm::uvec2 calcSpace(const std::unordered_set<FT_ULong>& chars) noexcept;
        Image operator()(const std::unordered_set<FT_ULong>& chars);
    private:
        FT_Face _face;
        bx::AllocatorI& _alloc;
        glm::uvec2 _size;

        std::vector<FT_Bitmap> getBitmaps(const std::unordered_set<FT_ULong>& chars);
    };

    class FreetypeFontAtlasImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        std::vector<std::filesystem::path> getOutputs(const Input& input);
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        const std::string& getName() const noexcept;
    };

}