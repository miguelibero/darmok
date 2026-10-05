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
#include "detail/render_samplers.hpp"
#include "generated/shaders/darmok_deferred_geo.h"
#include "generated/shaders/darmok_deferred_light.h"

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

    void DeferredGeoBuffer::configureView(bgfx::ViewId viewId) const noexcept
    {
        bgfx::setViewFrameBuffer(viewId, _handle);
    }

    expected<void, std::string> DeferredGeoBuffer::render(bgfx::Encoder& encoder) const noexcept
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

    DeferredLightingRenderStep::DeferredLightingRenderStep(const std::shared_ptr<Program>& prog) noexcept
        : _prog{ prog }
    {
    }

    DeferredLightingRenderStep::~DeferredLightingRenderStep() noexcept = default;

    expected<void, std::string> DeferredLightingRenderStep::init(RenderChain& chain) noexcept
    {
        _chain = chain;

        static const Rectangle screen{ glm::uvec2{2} };
        auto meshResult = MeshData{ screen }.createMesh(_prog->getVertexLayout());
        if (!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
        }
        _mesh = std::make_unique<Mesh>(std::move(meshResult).value());
        return {};
    }

    expected<void, std::string> DeferredLightingRenderStep::shutdown() noexcept
    {
        _viewId.reset();
        _mesh.reset();
        _chain.reset();
        _cam.reset();
        _gbuffer.reset();
        return {};
    }

    expected<void, std::string> DeferredLightingRenderStep::updateRenderChain(FrameBuffer& read, OptionalRef<FrameBuffer> write) noexcept
    {
        _writeBuffer = write;
        if (write && _viewId)
        {
            write->configureView(*_viewId);
        }
        return {};
    }

    expected<bgfx::ViewId, std::string> DeferredLightingRenderStep::renderReset(bgfx::ViewId viewId) noexcept
    {
        if (_chain)
        {
            _chain->configureView(viewId, "Deferred Lighting", _writeBuffer);
        }
        _viewId = viewId;
        return ++viewId;
    }

    void DeferredLightingRenderStep::setCamera(OptionalRef<Camera> cam) noexcept
    {
        _cam = cam;
    }

    void DeferredLightingRenderStep::setGeoBuffer(const DeferredGeoBuffer& gbuffer) noexcept
    {
        _gbuffer = gbuffer;
    }

    expected<void, std::string> DeferredLightingRenderStep::render(bgfx::Encoder& encoder) noexcept
    {
        if (!_viewId)
        {
            return unexpected<std::string>{ "no view id" };
        }
        auto viewId = *_viewId;

        if (!_mesh || !_prog || !_gbuffer)
        {
            encoder.touch(viewId);
            return {};
        }

        if (_cam)
        {
            auto result = _cam->beforeRenderView(viewId, encoder);
            if(!result)
            {
                return result;
            }
            result = _cam->beforeRenderEntity(entt::null, viewId, encoder);
            if(!result)
            {
                return result;
            }
        }

        _basicUniforms.configure(encoder);

        auto gbufferResult = _gbuffer->render(encoder);
        if(!gbufferResult)
        {
            return gbufferResult;
        }

        auto meshResult = _mesh->render(encoder);
        if(!meshResult)
        {
            return meshResult;
        }

        uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_DEPTH_TEST_MASK;
        state |= BGFX_STATE_DEPTH_TEST_ALWAYS;
        state &= ~BGFX_STATE_WRITE_Z;
        encoder.setState(state);
        encoder.submit(viewId, _prog->getHandle());

        return {};
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

        auto geoResult = Program::loadStaticMem(darmok_program_darmok_deferred_geo);
        if (!geoResult)
        {
            return unexpected{ "deferred geometry program: " + std::move(geoResult).error() };
        }
        _geometryProg = std::make_shared<Program>(std::move(geoResult).value());

        auto lightResult = Program::loadStaticMem(darmok_program_darmok_deferred_light);
        if (!lightResult)
        {
            return unexpected{ "deferred lighting program: " + std::move(lightResult).error() };
        }
        auto lightProg = std::make_shared<Program>(std::move(lightResult).value());

        auto& chain = cam.getRenderChain();
        auto stepResult = chain.addStep<DeferredLightingRenderStep>(lightProg);
        if (!stepResult)
        {
            return unexpected{ std::move(stepResult).error() };
        }
        _lightingStep = stepResult.value().get();
        _lightingStep->setCamera(cam);

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
        if (_lightingStep)
        {
            _lightingStep->setGeoBuffer(*_gbuffer);
        }
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

        bgfx::setViewName(viewId, _cam->getViewName("Deferred Geometry").c_str());
        bgfx::setPaletteColor(0, 0.0f, 0.0f, 0.0f, 0.0f);
        uint16_t clearFlags = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
        bgfx::setViewClear(viewId, clearFlags, 1.f, 0U, 0, 0, 0);
        _gbuffer->configureView(viewId);
        auto vp = _cam->getCombinedViewport();
        vp.configureView(viewId);
        _cam->setViewTransform(viewId);

        _viewId = viewId;
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
            auto& material = *renderable->getMaterial();
            if (_materials)
            {
                _materials->renderBind(encoder, material);
            }
            else
            {
                material.renderBind(encoder);
            }
            encoder.submit(viewId, _geometryProg->getHandle());
        }

        bgfx::end(&encoder);
        return StringUtils::joinExpectedErrors(errors);
    }

    expected<void, std::string> DeferredRenderer::update(float deltaTime) noexcept
    {
        return {};
    }

    expected<void, std::string> DeferredRenderer::shutdown() noexcept
    {
        if (_lightingStep && _cam)
        {
            (void)_cam->getRenderChain().removeStep(*_lightingStep);
        }
        _lightingStep.reset();
        _cam.reset();
        _scene.reset();
        _app.reset();
        _materials.reset();
        _geometryProg.reset();
        _gbuffer.reset();
        return {};
    }
}
