#pragma once

#include <CEGUI/Renderer.h>
#include "render_target.hpp"
#include "shader.hpp"

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class App;
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

    private:
        static const CEGUI::String _rendererId;
        CEGUI::Sizef _displaySize;
        App& _app;
        CeguiRenderTarget _defaultRenderTarget;
        std::unordered_map<CEGUI::String, std::unique_ptr<CeguiTexture>> _textures;
        std::vector<std::unique_ptr<CeguiTextureTarget>> _textureTargets;
        mutable CeguiShaderWrapper _shaderWrapper;

        bx::AllocatorI* getAllocator() const noexcept;
        CeguiTexture& doCreateTexture(const CEGUI::String& name);
    };
}