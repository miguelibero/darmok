
#define NK_IMPLEMENTATION
#include <nuklear.h>
#undef NK_IMPLEMENTATION

#include "nuklear.hpp"
#include <darmok/nuklear.hpp>
#include <darmok/asset.hpp>
#include <darmok/window.hpp>
#include <darmok/input.hpp>
#include <darmok/program.hpp>
#include <bx/math.h>
#include <array>
#include <memory>


namespace darmok
{
    NuklearFont::NuklearFont(struct nk_font_config& cfg, const OptionalRef<nk_draw_null_texture>& nullTex) noexcept
    {
        atlasBegin();
        nk_font_atlas_add_default(&_atlas, cfg.size, &cfg);
        atlasEnd(nullTex);
    }

    NuklearFont::NuklearFont(struct nk_font_config& cfg, const std::shared_ptr<Data>& data, const OptionalRef<nk_draw_null_texture>& nullTex) noexcept
    {
        atlasBegin();
        _ttf = data;
        nk_font_atlas_add_from_memory(&_atlas, data->ptr(), data->size(), 18, &cfg);
        nk_font_atlas_add_default(&_atlas, cfg.size, &cfg);
        atlasEnd(nullTex);
    }

    void NuklearFont::atlasBegin() noexcept
    {
        nk_font_atlas_init_default(&_atlas);
        nk_font_atlas_begin(&_atlas);
    }

    void NuklearFont::atlasEnd(const OptionalRef<nk_draw_null_texture>& nullTex) noexcept
    {
        int w, h;
        auto imagePtr = nk_font_atlas_bake(&_atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
        // TODO: check if the memory needs to be copied
        _image = Data(imagePtr, w * h * 4);
        _texture = bgfx::createTexture2D(
            w, h, false, 1, bgfx::TextureFormat::RGBA8, 0,
            _image.makeRef()
        );

        nk_font_atlas_end(&_atlas, nk_handle_id(_texture.idx), nullTex.ptr());
    }

    NuklearFont::~NuklearFont()
    {
        nk_font_atlas_clear(&_atlas);
        if (isValid(_texture))
        {
            bgfx::destroy(_texture);
        }
    }

    OptionalRef<nk_user_font> NuklearFont::getHandle() noexcept
    {
        if (_atlas.fonts != nullptr)
        {
            return _atlas.fonts->handle;
        }
        if (_atlas.default_font != nullptr)
        {
            return _atlas.default_font->handle;
        }
        return nullptr;
    }
    
    NuklearAppComponentImpl::NuklearAppComponentImpl(INuklearRenderer& renderer) noexcept
        : _renderer(renderer)
        , _ctx{}
        , _cmds{}
        , _nullTexture{}
        , _convertConfig{}
        , _fontConfig{}
        , _textureUniform{ bgfx::kInvalidHandle }
        , _ortho{}
        , _nkLayout{}
        , _viewport{}
    {
    }

    NuklearAppComponentImpl::~NuklearAppComponentImpl() noexcept
    {
    }

    void NuklearAppComponentImpl::setupProgram(App& app) noexcept
    {
        _textureUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
        _prog = app.getAssets().getStandardProgramLoader()(StandardProgramType::Gui);
        auto& layout = _prog->getVertexLayout();
        _nkLayout[0] = { NK_VERTEX_POSITION, NK_FORMAT_FLOAT, layout.getOffset(bgfx::Attrib::Position) };
        _nkLayout[1] = { NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, layout.getOffset(bgfx::Attrib::TexCoord0) };
        _nkLayout[2] = { NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, layout.getOffset(bgfx::Attrib::Color0) };
        _nkLayout[3] = { NK_VERTEX_LAYOUT_END };
    }

    nk_context& NuklearAppComponentImpl::getContext() noexcept
    {
        return _ctx;
    }

    const nk_context& NuklearAppComponentImpl::getContext() const noexcept
    {
        return _ctx;
    }

    void NuklearAppComponentImpl::setupFonts(App& app) noexcept
    {
        _fontConfig = nk_font_config(18);
        _fontConfig.oversample_h = 3;
        _fontConfig.oversample_v = 2;

        auto font = std::make_unique<NuklearFont>(_fontConfig, _nullTexture);
        auto handle = font->getHandle().ptr();
        if (handle != nullptr)
        {
            nk_style_set_font(&_ctx, handle);
        }
        _fonts.push_back(std::move(font));
    }

    OptionalRef<nk_user_font> NuklearAppComponentImpl::loadFont(std::string_view name, float height) noexcept
    {
        if (!_app)
        {
            return nullptr;
        }
        auto data = _app->getAssets().getDataLoader()(name);
        if (data == nullptr)
        {
            return nullptr;
        }

        auto font = std::make_unique<NuklearFont>(_fontConfig, data, _nullTexture);
        auto handle = font->getHandle();
        _fonts.push_back(std::move(font));
        return handle;
    }

    void NuklearAppComponentImpl::init(App& app) noexcept
    {
        _app = app;
        nk_init_default(&_ctx, 0);
        nk_buffer_init_default(&_cmds);

        setupProgram(app);
        setupFonts(app);

        NK_MEMSET(&_convertConfig, 0, sizeof(_convertConfig));
        _convertConfig.vertex_layout = &_nkLayout.front();
        _convertConfig.vertex_size = _prog->getVertexLayout().getStride();
        _convertConfig.vertex_alignment = 4;
        _convertConfig.tex_null = _nullTexture;
        _convertConfig.circle_segment_count = 22;
        _convertConfig.curve_segment_count = 22;
        _convertConfig.arc_segment_count = 22;
        _convertConfig.global_alpha = 1.0f;
        _convertConfig.shape_AA = NK_ANTI_ALIASING_ON;
        _convertConfig.line_AA = NK_ANTI_ALIASING_ON;
    }

    void NuklearAppComponentImpl::shutdown() noexcept
    {
        _fonts.clear();
        nk_free(&_ctx);
        nk_buffer_free(&_cmds);
        if (isValid(_textureUniform))
        {
            bgfx::destroy(_textureUniform);
        }
        _app = nullptr;
    }

    static const std::unordered_map<KeyboardKey, nk_keys> s_nuklearKeyMap = {
        { KeyboardKey::Delete, NK_KEY_DEL },
        { KeyboardKey::Return, NK_KEY_ENTER },
        { KeyboardKey::Tab, NK_KEY_TAB },
        { KeyboardKey::Backspace, NK_KEY_BACKSPACE },
        { KeyboardKey::Up, NK_KEY_UP },
        { KeyboardKey::Down, NK_KEY_DOWN },
        { KeyboardKey::Left, NK_KEY_LEFT },
        { KeyboardKey::Right, NK_KEY_RIGHT }
    };

    static const std::unordered_map<KeyboardModifierGroup, nk_keys> s_nuklearKeyboardModifierGroupMap = {
        { KeyboardModifierGroup::Ctrl, NK_KEY_CTRL },
        { KeyboardModifierGroup::Shift, NK_KEY_SHIFT },
    };

    void NuklearAppComponentImpl::processInput() noexcept
    {
        if (!_app)
        {
            return;
        }
        auto& input = _app->getInput();
        auto& win = _app->getWindow();

        nk_input_begin(&_ctx);

        {
            auto& kb = input.getKeyboard();

            for (auto& chr : kb.getUpdateChars())
            {
                nk_input_char(&_ctx, chr.data);
            }
            for (auto& elm : s_nuklearKeyMap)
            {
                nk_input_key(&_ctx, elm.second, kb.getKey(elm.first));
            }
            auto mods = kb.getModifiers();
            for (auto& elm : s_nuklearKeyboardModifierGroupMap)
            {
                nk_input_key(&_ctx, elm.second, mods | to_underlying(elm.first));
            }

            if (mods | to_underlying(KeyboardModifierGroup::Ctrl))
            {
                nk_input_key(&_ctx, NK_KEY_COPY, kb.getKey(KeyboardKey::KeyC));
                nk_input_key(&_ctx, NK_KEY_PASTE, kb.getKey(KeyboardKey::KeyV));
                nk_input_key(&_ctx, NK_KEY_CUT, kb.getKey(KeyboardKey::KeyX));
            }
        } 
        {
            auto& mouse = input.getMouse();
            auto p = win.screenPointToWindow(mouse.getPosition());

            nk_input_motion(&_ctx, p.x, p.y);
            nk_input_button(&_ctx, NK_BUTTON_LEFT, p.x, p.y, mouse.getButton(MouseButton::Left));
            nk_input_button(&_ctx, NK_BUTTON_MIDDLE, p.x, p.y, mouse.getButton(MouseButton::Middle));
            nk_input_button(&_ctx, NK_BUTTON_RIGHT, p.x, p.y, mouse.getButton(MouseButton::Right));
            auto& scroll = mouse.getScroll();
            nk_input_scroll(&_ctx, { scroll.x, scroll. y });
        }

        nk_input_end(&_ctx);
    }

    void NuklearAppComponentImpl::updateLogic(float deltaTime) noexcept
    {
        if (!_app)
        {
            return;
        }

        // update ortho matrix
        auto& vp = _app->getWindow().getViewport();
        if (vp != _viewport)
        {
            auto caps = bgfx::getCaps();
            bx::mtxOrtho(_ortho, vp[0], vp[2], vp[3], vp[1], 0.0f, 1000.0f, 0.0f, caps->homogeneousDepth);
            _viewport = vp;
        }

        processInput();
    }

    bgfx::ViewId NuklearAppComponentImpl::render(bgfx::ViewId viewId) noexcept
    {
        if (!_app)
        {
            return viewId;
        }

        _renderer.nuklearRender(_ctx);
        submitRender(viewId);

        return ++viewId;
    }

    void NuklearAppComponentImpl::submitRender(bgfx::ViewId viewId) noexcept
    {
        bgfx::setViewTransform(viewId, nullptr, _ortho);
        auto winSize = _app->getWindow().getPixelSize();
        bgfx::setViewRect(viewId, 0, 0, winSize.x, winSize.y);

        /* convert from command queue into draw list and draw to screen */
        struct nk_buffer vbuf, ebuf;

        static const nk_size maxVertexCount = 65536;
        static const nk_size maxElementCount = maxVertexCount * 2;
        static const nk_size maxElementMem = maxElementCount * sizeof(uint16_t);
        static const uint64_t bgfxStateFlags = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
            | BGFX_STATE_MSAA
            ;

        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer tib;

        auto& layout = _prog->getVertexLayout();

        bgfx::allocTransientVertexBuffer(&tvb, maxVertexCount, layout);
        bgfx::allocTransientIndexBuffer(&tib, maxElementCount);

        nk_buffer_init_fixed(&vbuf, tvb.data, layout.getSize(maxVertexCount));
        nk_buffer_init_fixed(&ebuf, tib.data, maxElementMem);
        nk_convert(&_ctx, &_cmds, &vbuf, &ebuf, &_convertConfig);

        /* iterate over and execute each draw command */
        uint32_t offset = 0;
        const struct nk_draw_command* cmd;
        nk_draw_foreach(cmd, &_ctx, &_cmds) {

            if (!cmd->elem_count) {
                continue;
            }

            const uint16_t xx = uint16_t(bx::max(cmd->clip_rect.x, 0.0f));
            const uint16_t yy = uint16_t(bx::max(cmd->clip_rect.y, 0.0f));
            const uint16_t cw = uint16_t(bx::max(cmd->clip_rect.w, 0.0f));
            const uint16_t ch = uint16_t(bx::max(cmd->clip_rect.h, 0.0f));
            bgfx::setScissor(xx, yy, cw, ch);

            bgfx::setState(bgfxStateFlags);
            bgfx::setTexture(0, _textureUniform, { (uint16_t)cmd->texture.id });
            bgfx::setVertexBuffer(0, &tvb, 0, maxVertexCount);
            bgfx::setIndexBuffer(&tib, offset, cmd->elem_count);
            bgfx::submit(viewId, _prog->getHandle());

            offset += cmd->elem_count;
        }
        nk_buffer_clear(&_cmds);
        nk_clear(&_ctx);
    }

    NuklearAppComponent::NuklearAppComponent(INuklearRenderer& renderer) noexcept
        : _impl(std::make_unique<NuklearAppComponentImpl>(renderer))
    {
    }

    NuklearAppComponent::~NuklearAppComponent() noexcept
    {
    }

    void NuklearAppComponent::init(App& app)
    {
        _impl->init(app);
    }

    void NuklearAppComponent::shutdown()
    {
        _impl->shutdown();
    }

    void NuklearAppComponent::updateLogic(float deltaTime)
    {
        _impl->updateLogic(deltaTime);
    }

    bgfx::ViewId NuklearAppComponent::render(bgfx::ViewId viewId)
    {
        return _impl->render(viewId);
    }

    nk_context& NuklearAppComponent::getContext() noexcept
    {
        return _impl->getContext();
    }

    const nk_context& NuklearAppComponent::getContext() const noexcept
    {
        return _impl->getContext();
    }

    OptionalRef<nk_user_font> NuklearAppComponent::loadFont(std::string_view name, float height) noexcept
    {
        return _impl->loadFont(name, height);
    }

}