#pragma once

#include <CEGUI/Renderer.h>
#include "render_target.hpp"
#include "shader.hpp"
#include <bgfx/bgfx.h>
#include <memory>

namespace bx
{
    struct AllocatorI;
}

namespace bgfx
{
    struct EmbeddedShader;
}

namespace darmok
{
    class App;
    class Program;
    class CeguiTexture;
    class CeguiTextureTarget;

    class CeguiRenderer final : public CEGUI::Renderer
    {
    public:
        CeguiRenderer(App& app) noexcept;
        ~CeguiRenderer() noexcept;
        CEGUI::RenderTarget& getDefaultRenderTarget() override;
        CEGUI::RefCounted<CEGUI::RenderMaterial> createRenderMaterial(const CEGUI::DefaultShaderType shaderType) const override;
        CEGUI::GeometryBuffer& createGeometryBufferTextured(CEGUI::RefCounted<CEGUI::RenderMaterial> renderMaterial) override;
        CEGUI::GeometryBuffer& createGeometryBufferColoured(CEGUI::RefCounted<CEGUI::RenderMaterial> renderMaterial) override;
        CEGUI::TextureTarget* createTextureTarget(bool addStencilBuffer) override;
        void destroyTextureTarget(CEGUI::TextureTarget* target) override;
        void destroyAllTextureTargets() override;
        CEGUI::Texture& createTexture(const CEGUI::String& name) override;
        CeguiTexture& createTexture(const CEGUI::String& name, uint64_t flags);
        CEGUI::Texture& createTexture(const CEGUI::String& name,
            const CEGUI::String& filename,
            const CEGUI::String& resourceGroup) override;
        CEGUI::Texture& createTexture(const CEGUI::String& name, const CEGUI::Sizef& size) override;
        void destroyTexture(CEGUI::Texture& texture) override;
        void destroyTexture(const CEGUI::String& name) override;
        void destroyAllTextures() override;
        CEGUI::Texture& getTexture(const CEGUI::String& name) const override;
        bool isTextureDefined(const CEGUI::String& name) const override;
        void beginRendering() override;
        void endRendering() override;
        void setDisplaySize(const CEGUI::Sizef& sz) override;
        const CEGUI::Sizef& getDisplaySize() const override;
        unsigned int getMaxTextureSize() const override;
        const CEGUI::String& getIdentifierString() const override;
        bool isTexCoordSystemFlipped() const override;

        bgfx::ViewId getViewId() const noexcept;
        void setViewId(bgfx::ViewId viewId) noexcept;

        bx::AllocatorI* getAllocator() const noexcept;

    private:
        static const CEGUI::String _rendererId;
        static const CEGUI::String _solidWhiteTextureName;
        CEGUI::Sizef _displaySize;
        App& _app;
        CeguiRenderTarget _defaultRenderTarget;
        std::unordered_map<CEGUI::String, std::unique_ptr<CeguiTexture>> _textures;
        std::vector<std::unique_ptr<CeguiTextureTarget>> _textureTargets;
        uint16_t _textureTargetNum;
        std::shared_ptr<Program> _texturedProgram;
        mutable CeguiShaderWrapper _texturedShaderWrapper;
        std::shared_ptr<Program> _solidProgram;
        mutable CeguiShaderWrapper _solidShaderWrapper;
        bgfx::ViewId _viewId;
        static const bgfx::EmbeddedShader _embeddedShaders[];

        CeguiTexture& doCreateTexture(const CEGUI::String& name, uint64_t flags = 0);
        CeguiTexture& doAddTexture(std::unique_ptr<CeguiTexture>&& tex);
    };
}