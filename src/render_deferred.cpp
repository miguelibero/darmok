#include <darmok/render_deferred.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/shape.hpp>
#include <darmok/texture.hpp>
#include <darmok/material.hpp>
#include <darmok/scene.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/string.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/light.hpp>
#include <darmok/shadow.hpp>
#include "detail/render_samplers.hpp"
#include "generated/shaders/deferred_geo.h"
#include "generated/shaders/deferred_light.h"

namespace darmok
{
    DeferredGeoBuffer::DeferredGeoBuffer(
        Texture albedoMetallicTex,
        Texture normalRoughnessOcclusionTex,
        Texture emissiveTex,
        Texture depthTex
    ) noexcept
        : _albedoMetallicTex{std::move(albedoMetallicTex)}
        , _normalRoughnessOcclusionTex{std::move(normalRoughnessOcclusionTex)}
        , _emissiveTex{std::move(emissiveTex)}
        , _depthTex{std::move(depthTex)}
        , _albedoMetallicUniform{"s_gbufferAlbedoMetallic",bgfx::UniformType::Sampler}
        , _normalRoughnessOcclusionUniform{"s_gbufferNormalRoughnessOcclusion", bgfx::UniformType::Sampler}
        , _emissiveUniform{"s_gbufferEmissive", bgfx::UniformType::Sampler}
        , _depthUniform{"s_gbufferDepth", bgfx::UniformType::Sampler}
        , _sceneDepthUniform{"s_sceneDepth", bgfx::UniformType::Sampler}
    {
        bgfx::TextureHandle handles[] = {_albedoMetallicTex.getHandle(),
                                         _normalRoughnessOcclusionTex.getHandle(),
                                         _emissiveTex.getHandle(),
                                         _depthTex.getHandle()};

        _handle = bgfx::createFrameBuffer(4, handles);
    }

    glm::uvec2 DeferredGeoBuffer::getSize() const noexcept
    {
        return _albedoMetallicTex.getSize();
    }

    expected<DeferredGeoBuffer, std::string> DeferredGeoBuffer::load(const glm::uvec2& size) noexcept
    {
        Texture::Config cfg;
        *cfg.mutable_size() = convert<protobuf::Uvec2>(size);
        cfg.set_type(Texture::Definition::Texture2D);

        auto loadTexture = [&](Texture::Format format)
        {
            cfg.set_format(format);
            return Texture::load(cfg, BGFX_TEXTURE_RT);
        };

        auto albedoResult = loadTexture(Texture::Definition::RGBA8);
        if(!albedoResult)
        {
            return unexpected{std::move(albedoResult).error()};
        }
        auto normalTesult = loadTexture(Texture::Definition::RGBA8);
        if(!normalTesult)
        {
            return unexpected{std::move(normalTesult).error()};
        }
        auto emissiveResult = loadTexture(Texture::Definition::RGBA16F);
        if(!emissiveResult)
        {
            return unexpected{std::move(emissiveResult).error()};
        }
        auto depthResult = loadTexture(Texture::Definition::D16F);
        if(!depthResult)
        {
            return unexpected{std::move(depthResult).error()};
        }

        return DeferredGeoBuffer{
            std::move(*albedoResult),
            std::move(*normalTesult),
            std::move(*emissiveResult),
            std::move(*depthResult)};
    }

    void DeferredGeoBuffer::configureGeoView(bgfx::ViewId viewId) const noexcept
    {
        bgfx::setViewFrameBuffer(viewId, _handle);
    }

    expected<void, std::string> DeferredGeoBuffer::beforeRenderLight(bgfx::Encoder& encoder) const noexcept
    {
        // Reuse material sampler slots 0-3 (unused in the lighting pass)
        encoder.setTexture(RenderSamplers::DEFERRED_ALBEDO_METALLIC,
                           _albedoMetallicUniform, _albedoMetallicTex.getHandle());
        encoder.setTexture(RenderSamplers::DEFERRED_NORMAL_ROUGHNESS_OCCLUSION,
                           _normalRoughnessOcclusionUniform, _normalRoughnessOcclusionTex.getHandle());
        encoder.setTexture(RenderSamplers::DEFERRED_EMISSIVE,
                           _emissiveUniform, _emissiveTex.getHandle());
        encoder.setTexture(RenderSamplers::DEFERRED_DEPTH,
                           _depthUniform, _depthTex.getHandle());

        return {};
    }

    void DeferredGeoBuffer::beforeRenderTransparent(bgfx::Encoder& encoder) const noexcept
    {
        encoder.setTexture(RenderSamplers::DEFERRED_SCENE_DEPTH,
                           _sceneDepthUniform, _depthTex.getHandle());
    }

    DeferredRenderer::Definition DeferredRenderer::createDefinition() noexcept
    {
        Definition def;
        return def;
    }

    expected<void, std::string> DeferredRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam   = cam;
        _scene = scene;
        _app   = app;

        auto matResult = app.getOrAddComponent<MaterialAppComponent>();
        if (!matResult)
        {
            return unexpected{ std::move(matResult).error() };
        }
        _materials = matResult.value().get();

        auto geoResult = Program::loadStaticMem(darmok_program_deferred_geo);
        if (!geoResult)
        {
            return unexpected{ "deferred geometry program: " + std::move(geoResult).error() };
        }
        _geoProg = std::move(geoResult).value();

        auto lightResult = Program::loadStaticMem(darmok_program_deferred_light);
        if (!lightResult)
        {
            return unexpected{ "deferred lighting program: " + std::move(lightResult).error() };
        }
        _lightProg = std::move(lightResult).value();

        // transparent objects are forward shaded over the lit deferred scene
        auto forwardResult = StandardProgramLoader::load(Program::Standard::Forward);
        if (!forwardResult)
        {
            return unexpected{ "forward program: " + std::move(forwardResult).error() };
        }
        _forwardProg = std::move(forwardResult).value();
        auto unlitResult = StandardProgramLoader::load(Program::Standard::Unlit);
        if (!unlitResult)
        {
            return unexpected{ "unlit program: " + std::move(unlitResult).error() };
        }
        _unlitProg = std::move(unlitResult).value();

        static const Rectangle screen{glm::uvec2{2}};
        auto meshResult = MeshData{screen}.createMesh(_lightProg->getVertexLayout());
        if(!meshResult)
        {
            return unexpected{std::move(meshResult).error()};
        }
        _screenMesh = std::move(meshResult).value();

        return {};
    }

    expected<void, std::string> DeferredRenderer::load(const Definition& def) noexcept
    {
        return {};
    }

    expected<void, std::string> DeferredRenderer::recreateGeoBuffer() noexcept
    {
        if (!_cam)
        {
            return unexpected<std::string>{ "camera not set" };
        }
        auto vp = _cam->getCombinedViewport();
        auto size = vp.origin + vp.size;
        if(_gbuffer && size == _gbuffer->getSize())
        {
            return {};
        }
        auto result = DeferredGeoBuffer::load(size);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        _gbuffer = std::move(result).value();
        return {};
    }

    expected<bgfx::ViewId, std::string> DeferredRenderer::renderReset(bgfx::ViewId viewId) noexcept
    {
        _viewId.reset();
        if (!_cam)
        {
            return unexpected<std::string>{ "camera not loaded" };
        }

        auto gbResult = recreateGeoBuffer();
        if (!gbResult)
        {
            return unexpected{ std::move(gbResult).error() };
        }

        _viewId = viewId;

        uint16_t clearFlags = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
        bgfx::setViewName(viewId, _cam->getViewName("Deferred Geometry").c_str());
        bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);

        auto vp = _cam->getCombinedViewport();
        vp.configureView(viewId);
        if (_gbuffer)
        {
            _gbuffer->configureGeoView(viewId);
        }

        ++viewId;

        _cam->configureView(viewId, "Deferred Lighting", clearFlags);

        ++viewId;

        // keep the lit color and depth, draw transparent objects back to front
        _cam->configureView(viewId, "Deferred Transparent", BGFX_CLEAR_NONE);
        bgfx::setViewMode(viewId, bgfx::ViewMode::DepthDescending);

        return ++viewId;
    }

    expected<void, std::string> DeferredRenderer::render() noexcept
    {
        if (!_viewId)
        {
            return {};
        }
        if (!_scene || !_cam)
        {
            return unexpected<std::string>{ "scene or camera not loaded" };
        }
        if (!_cam->isEnabled())
        {
            return {};
        }

        auto viewId = *_viewId;
        auto& encoder = *bgfx::begin();

        auto result = _cam->beforeRenderView(viewId, encoder);
        if (!result)
        {
            bgfx::end(&encoder);
            return result;
        }

        std::vector<std::string> errors;
        std::vector<Entity> transparentEntities;
        auto entities = _cam->getEntities<Renderable>();
        for (auto entity : entities)
        {
            auto renderable = _scene->getComponent<const Renderable>(entity);
            if (!renderable->valid())
            {
                continue;
            }
            if (_cam->shouldEntityBeCulled(entity))
            {
                continue;
            }
            auto& material = *renderable->getMaterial();
            if (material.opacityType == Material::Definition::Transparent)
            {
                // drawn after lighting, they can't be stored in the G-buffer
                transparentEntities.push_back(entity);
                continue;
            }
            auto entityResult = _cam->beforeRenderEntity(entity, viewId, encoder);
            if (!entityResult)
            {
                errors.push_back(std::move(entityResult).error());
                continue;
            }
            if (!renderable->render(encoder))
            {
                continue;
            }
            if (_materials)
            {
                _materials->renderBind(encoder, material);
            }
            else
            {
                material.renderBind(encoder);
            }

            encoder.setState(material.getRenderState());
            encoder.submit(viewId, _geoProg->getHandle());
        }

        ++viewId;

        if(!_screenMesh || !_lightProg || !_gbuffer)
        {
            encoder.touch(viewId);
            bgfx::end(&encoder);
            return unexpected<std::string>{"lighting not initialized"};
        }

        result = _gbuffer->beforeRenderLight(encoder);
        if(!result)
        {
            bgfx::end(&encoder);
            return result;
        }

        result = _cam->beforeRenderLight(viewId, encoder);
        if(!result)
        {
            bgfx::end(&encoder);
            return result;
        }

        result = _materials->beforeRenderLight(viewId, encoder);
        if(!result)
        {
            bgfx::end(&encoder);
            return result;
        }

        result = _screenMesh->render(encoder);
        if(!result)
        {
            bgfx::end(&encoder);
            return result;
        }

        uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_DEPTH_TEST_MASK;
        state |= BGFX_STATE_DEPTH_TEST_ALWAYS;
        state &= ~BGFX_STATE_WRITE_Z;
        encoder.setState(state);

        // the shadow code is only compiled into the SHADOW_ENABLED program variant
        ProgramDefines defines;
        if (_cam->getComponent<ShadowRenderer>())
        {
            defines.insert("SHADOW_ENABLED");
        }
        encoder.submit(viewId, _lightProg->getHandle(defines));

        ++viewId;
        auto transResult = renderTransparent(viewId, encoder, transparentEntities, defines);
        if (!transResult)
        {
            errors.push_back(std::move(transResult).error());
        }

        bgfx::end(&encoder);
        return StringUtils::joinExpectedErrors(errors);
    }

    expected<void, std::string> DeferredRenderer::renderTransparent(bgfx::ViewId viewId, bgfx::Encoder& encoder,
        const std::vector<Entity>& entities, const ProgramDefines& lightDefines) noexcept
    {
        if (entities.empty())
        {
            encoder.touch(viewId);
            return {};
        }
        auto result = _cam->beforeRenderView(viewId, encoder);
        if (!result)
        {
            return result;
        }

        // opaque depth lives in the G-buffer, the shader discards fragments behind it
        ProgramDefines defines = lightDefines;
        defines.insert("SCENE_DEPTH_ENABLED");

        std::vector<std::string> errors;
        for (auto entity : entities)
        {
            auto renderable = _scene->getComponent<const Renderable>(entity);
            auto entityResult = _cam->beforeRenderEntity(entity, viewId, encoder);
            if (!entityResult)
            {
                errors.push_back(std::move(entityResult).error());
                continue;
            }
            auto lightResult = _cam->beforeRenderLight(viewId, encoder);
            if (!lightResult)
            {
                errors.push_back(std::move(lightResult).error());
                continue;
            }
            _gbuffer->beforeRenderTransparent(encoder);
            if (!renderable->render(encoder))
            {
                continue;
            }
            auto& material = *renderable->getMaterial();
            auto& prog = material.shadingModel == Material::Definition::Unlit ? _unlitProg : _forwardProg;
            auto matDefines = material.programDefines;
            _materials->renderBind(encoder, material);
            encoder.setState(material.getRenderState());
            encoder.submit(viewId, prog->getHandle(material.programDefines));
        }
        return StringUtils::joinExpectedErrors(errors);
    }

    expected<void, std::string> DeferredRenderer::update(float deltaTime) noexcept
    {
        return {};
    }

    expected<void, std::string> DeferredRenderer::shutdown() noexcept
    {
        _cam.reset();
        _scene.reset();
        _app.reset();
        _materials.reset();
        _geoProg.reset();
        _lightProg.reset();
        _forwardProg.reset();
        _unlitProg.reset();
        _screenMesh.reset();
        _gbuffer.reset();
        return {};
    }
}
