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
    // -------------------------------------------------------------------------
    // DeferredGBuffer
    // -------------------------------------------------------------------------

    static Texture::Config makeGBufferColorConfig(const glm::uvec2& size, Texture::Format format) noexcept
    {
        Texture::Config cfg;
        *cfg.mutable_size() = convert<protobuf::Uvec2>(size);
        cfg.set_format(format);
        cfg.set_type(Texture::Definition::Texture2D);
        return cfg;
    }

    expected<DeferredGBuffer, std::string> DeferredGBuffer::load(const glm::uvec2& size) noexcept
    {
        DeferredGBuffer gb;
        gb.size = size;

        auto loadTex = [&](Texture::Format fmt) -> expected<std::shared_ptr<Texture>, std::string>
        {
            auto r = Texture::load(makeGBufferColorConfig(size, fmt), BGFX_TEXTURE_RT);
            if (!r) return unexpected{ std::move(r).error() };
            return std::make_shared<Texture>(std::move(r).value());
        };

        auto r0 = loadTex(Texture::Definition::RGBA8);
        if (!r0) return unexpected{ r0.error() };
        gb.albedoMetallicTex = std::move(r0).value();

        auto r1 = loadTex(Texture::Definition::RGBA8);
        if (!r1) return unexpected{ r1.error() };
        gb.normalRoughnessOcclusionTex = std::move(r1).value();

        auto r2 = loadTex(Texture::Definition::RGBA16F);
        if (!r2) return unexpected{ r2.error() };
        gb.emissiveTex = std::move(r2).value();

        auto r3 = loadTex(Texture::Definition::D16F);
        if (!r3) return unexpected{ r3.error() };
        gb.depthTex = std::move(r3).value();

        bgfx::TextureHandle handles[] = {
            gb.albedoMetallicTex->getHandle(),
            gb.normalRoughnessOcclusionTex->getHandle(),
            gb.emissiveTex->getHandle(),
            gb.depthTex->getHandle(),
        };
        gb.handle = bgfx::createFrameBuffer(4, handles);

        return gb;
    }

    void DeferredGBuffer::configureView(bgfx::ViewId viewId) const noexcept
    {
        bgfx::setViewFrameBuffer(viewId, handle);
    }

    // -------------------------------------------------------------------------
    // DeferredLightingRenderStep
    // -------------------------------------------------------------------------

    DeferredLightingRenderStep::DeferredLightingRenderStep(const std::shared_ptr<Program>& prog) noexcept
        : _prog{ prog }
    {
    }

    DeferredLightingRenderStep::~DeferredLightingRenderStep() noexcept = default;

    expected<void, std::string> DeferredLightingRenderStep::init(RenderChain& chain) noexcept
    {
        _chain = chain;

        _albedoMetallicUniform            = { "s_gbufferAlbedoMetallic",           bgfx::UniformType::Sampler };
        _normalRoughnessOcclusionUniform  = { "s_gbufferNormalRoughnessOcclusion", bgfx::UniformType::Sampler };
        _emissiveUniform                  = { "s_gbufferEmissive",                 bgfx::UniformType::Sampler };
        _depthUniform                     = { "s_gbufferDepth",                    bgfx::UniformType::Sampler };

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
        _albedoMetallicTex.reset();
        _normalRoughnessOcclusionTex.reset();
        _emissiveTex.reset();
        _depthTex.reset();
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

    void DeferredLightingRenderStep::setCam(OptionalRef<Camera> cam) noexcept
    {
        _cam = cam;
    }

    void DeferredLightingRenderStep::setGBuffer(const DeferredGBuffer& gbuffer) noexcept
    {
        _albedoMetallicTex           = gbuffer.albedoMetallicTex;
        _normalRoughnessOcclusionTex = gbuffer.normalRoughnessOcclusionTex;
        _emissiveTex                 = gbuffer.emissiveTex;
        _depthTex                    = gbuffer.depthTex;
    }

    expected<void, std::string> DeferredLightingRenderStep::render(bgfx::Encoder& encoder) noexcept
    {
        if (!_viewId)
        {
            return unexpected<std::string>{ "no view id" };
        }
        auto viewId = *_viewId;

        if (!_mesh || !_prog || !_albedoMetallicTex)
        {
            encoder.touch(viewId);
            return {};
        }

        if (_cam)
        {
            auto result = _cam->beforeRenderView(viewId, encoder);
            if (!result) return result;
            // Bind per-camera uniforms (camera pos, light buffers, shadows).
            // entt::null is used because the lighting pass has no entity transform.
            result = _cam->beforeRenderEntity(entt::null, viewId, encoder);
            if (!result) return result;
        }

        auto meshResult = _mesh->render(encoder);
        if (!meshResult) return meshResult;

        // Reuse material sampler slots 0-3 (unused in the lighting pass)
        encoder.setTexture(RenderSamplers::MATERIAL_ALBEDO,
            _albedoMetallicUniform, _albedoMetallicTex->getHandle());
        encoder.setTexture(RenderSamplers::MATERIAL_SPECULAR,
            _normalRoughnessOcclusionUniform, _normalRoughnessOcclusionTex->getHandle());
        encoder.setTexture(RenderSamplers::MATERIAL_METALLIC_ROUGHNESS,
            _emissiveUniform, _emissiveTex->getHandle());
        encoder.setTexture(RenderSamplers::MATERIAL_NORMAL,
            _depthUniform, _depthTex->getHandle());

        _basicUniforms.configure(encoder);

        uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_DEPTH_TEST_MASK;
        state |= BGFX_STATE_DEPTH_TEST_ALWAYS;
        state &= ~BGFX_STATE_WRITE_Z;
        encoder.setState(state);
        encoder.submit(viewId, _prog->getHandle());

        return {};
    }

    // -------------------------------------------------------------------------
    // DeferredRenderer
    // -------------------------------------------------------------------------

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

        auto geomResult = Program::loadStaticMem(darmok_program_darmok_deferred_geo);
        if (!geomResult)
        {
            return unexpected{ "deferred geometry program: " + std::move(geomResult).error() };
        }
        _geometryProg = std::make_shared<Program>(std::move(geomResult).value());

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
        _lightingStep->setCam(cam);

        return {};
    }

    expected<void, std::string> DeferredRenderer::load(const Definition& def) noexcept
    {
        return {};
    }

    expected<void, std::string> DeferredRenderer::recreateGBuffer() noexcept
    {
        if (!_cam)
        {
            return unexpected<std::string>{ "camera not set" };
        }
        auto vp = _cam->getCombinedViewport();
        auto size = vp.origin + vp.size;
        if (size == _gbuffer.size && _gbuffer.valid())
        {
            return {};
        }
        auto result = DeferredGBuffer::load(size);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        _gbuffer = std::move(result).value();
        if (_lightingStep)
        {
            _lightingStep->setGBuffer(_gbuffer);
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

        auto gbResult = recreateGBuffer();
        if (!gbResult)
        {
            return unexpected{ std::move(gbResult).error() };
        }

        bgfx::setViewName(viewId, _cam->getViewName("Deferred Geometry").c_str());
        bgfx::setPaletteColor(0, 0.0f, 0.0f, 0.0f, 0.0f);
        uint16_t clearFlags = BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
        bgfx::setViewClear(viewId, clearFlags, 1.f, 0U, 0, 0, 0);
        _gbuffer.configureView(viewId);
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
        _gbuffer = {};
        return {};
    }
}
