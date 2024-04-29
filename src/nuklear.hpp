
#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <nuklear.h>
#include <array>
#include <memory>
#include <bgfx/bgfx.h>

namespace darmok
{
    class DataView;

    class NuklearFont final
    {
    public:
        NuklearFont(struct nk_font_config& cfg, const OptionalRef<nk_draw_null_texture>& nullTex = nullptr) noexcept;
        NuklearFont(struct nk_font_config& cfg, const DataView& data, const OptionalRef<nk_draw_null_texture>& nullTex = nullptr) noexcept;
        ~NuklearFont() noexcept;

        // nk_font_atlas memory needs to be stable
        NuklearFont(const NuklearFont& other) = delete;
        NuklearFont(NuklearFont&& other) = delete;

        OptionalRef<nk_user_font> getHandle() noexcept;
    private:

        nk_font_atlas _atlas;
        bgfx::TextureHandle _texture;

        void atlasBegin() noexcept;
        void atlasEnd(const OptionalRef<nk_draw_null_texture>& nullTex) noexcept;
    };

    using NuklearVertexLayout = std::array<nk_draw_vertex_layout_element, 4>;

    class INuklearRenderer;
    class Program;

    class NuklearAppComponentImpl final
    {
    public:
        NuklearAppComponentImpl(INuklearRenderer& renderer) noexcept;
        ~NuklearAppComponentImpl() noexcept;
        void init(App& app) noexcept;
        void shutdown() noexcept;
        void updateLogic(float deltaTime) noexcept;
        bgfx::ViewId render(bgfx::ViewId viewId) const noexcept;
        nk_context& getContext() noexcept;
        const nk_context& getContext() const noexcept;
        OptionalRef<nk_user_font> loadFont(std::string_view name, float height) noexcept;

    private:
        INuklearRenderer& _renderer;
        mutable nk_context _ctx;
        mutable nk_buffer _cmds;
        nk_draw_null_texture _nullTexture;
        nk_convert_config _convertConfig;
        struct nk_font_config _fontConfig;

        bgfx::UniformHandle _textureUniform;
        std::shared_ptr<Program> _prog;
        std::vector<std::unique_ptr<NuklearFont>> _fonts;
        OptionalRef<App> _app;
        float _ortho[16];
        NuklearVertexLayout _nkLayout;
        glm::uvec2 _viewSize;

        void setupProgram(App& app) noexcept;
        void setupFonts(App& app) noexcept;
        void processInput() noexcept;
        void submitRender(bgfx::ViewId viewId) const noexcept;
    };
}