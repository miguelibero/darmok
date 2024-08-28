#include <darmok/shadow.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/program_core.hpp>
#include <darmok/texture.hpp>
#include <darmok/light.hpp>
#include <darmok/scene.hpp>
#include <darmok/math.hpp>
#include <darmok/shape.hpp>
#include <darmok/material.hpp>
#include <darmok/vertex.hpp>
#include <darmok/easing.hpp>
#include <darmok/mesh.hpp>
#include "generated/shadow.program.h"
#include "render_samplers.hpp"

namespace darmok
{
    ShadowRenderPass::ShadowRenderPass()
        : _fb{ bgfx::kInvalidHandle }
        , _lightEntity(entt::null)
        , _cascade(-1)
    {
    }

    bool ShadowRenderPass::configure(Entity entity) noexcept
    {
        if (_lightEntity == entity)
        {
            return false;
        }
        _lightEntity = entity;
        return true;
    }

    void ShadowRenderPass::init(ShadowRenderer& renderer, uint16_t index, uint8_t cascade) noexcept
    {
        _renderer = renderer;
        _cascade = cascade;

        bgfx::Attachment at;
        auto i = (renderer.getConfig().cascadeAmount * index) + cascade;
        at.init(renderer.getTextureHandle(), bgfx::Access::Write, i);
        _fb = bgfx::createFrameBuffer(1, &at);
    }

    void ShadowRenderPass::shutdown() noexcept
    {
        if (isValid(_fb))
        {
            bgfx::destroy(_fb);
            _fb.idx = bgfx::kInvalidHandle;
        }
        _renderer.reset();
    }

    void ShadowRenderPass::renderPassDefine(RenderPassDefinition& def) noexcept
    {
        std::string name = "Shadow light ";
        if (_lightEntity == entt::null)
        {
            name += "(not used)";
        }
        else
        {
            name += std::to_string(_lightEntity);
            if (_cascade >= 0)
            {
                name += " cascade " + std::to_string(_cascade);
            }
        }
        def.setName(name);
    }

    void ShadowRenderPass::renderPassConfigure(bgfx::ViewId viewId) noexcept
    {
        auto size = _renderer->getConfig().mapSize;
        bgfx::setViewRect(viewId, 0, 0, size, size);
        bgfx::setViewFrameBuffer(viewId, _fb);
        bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH);
    }

    void ShadowRenderPass::renderPassExecute(IRenderGraphContext& context) noexcept
    {
        if (!_renderer || !_renderer->isEnabled())
        {
            return;
        }
        if (_lightEntity == entt::null)
        {
            return;
        }
        auto& encoder = context.getEncoder();
        auto viewId = context.getViewId();

        const void* viewPtr = nullptr;
        auto scene = _renderer->getScene();
        auto trans = scene->getComponent<const Transform>(_lightEntity);
        auto proj = _renderer->getProjViewMatrix(trans, _cascade);
        if (trans)
        {
            viewPtr = glm::value_ptr(trans->getWorldInverse());
        }
        bgfx::setViewTransform(viewId, viewPtr, glm::value_ptr(proj));

        renderEntities(viewId, encoder);
    }

    void ShadowRenderPass::renderEntities(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        static const uint64_t renderState =
            BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            // | BGFX_STATE_CULL_CCW
            | BGFX_STATE_CULL_CW
            | BGFX_STATE_MSAA
            ;

        auto scene = _renderer->getScene();
        auto cam = _renderer->getCamera();

        auto renderables = cam->createEntityView<Renderable>();
        for (auto entity : renderables)
        {
            auto renderable = scene->getComponent<const Renderable>(entity);
            if (!renderable->valid())
            {
                continue;
            }
            if (renderable->getMaterial()->getPrimitiveType() == MaterialPrimitiveType::Line)
            {
                continue;
            }
            cam->beforeRenderEntity(entity, encoder);
            if (!renderable->render(encoder))
            {
                continue;
            }

            encoder.setState(renderState);
            encoder.submit(viewId, _renderer->getProgramHandle());
        }
    }

    ShadowRenderer::ShadowRenderer(const Config& config) noexcept
        : _config(config)
    {
    }

    void ShadowRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
		_cam = cam;
		_scene = scene;
		_app = app;

        ProgramDefinition shadowProgDef;
        shadowProgDef.loadStaticMem(shadow_program);
        _program = std::make_unique<Program>(shadowProgDef);

        TextureConfig texConfig;
        texConfig.size = glm::uvec2(_config.mapSize);
        texConfig.layers = _config.maxLightAmount * _config.cascadeAmount;
        texConfig.format = bgfx::TextureFormat::D16;

        _tex = std::make_unique<Texture>(texConfig,
            BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL |
            // BGFX_SAMPLER_MAG_POINT |
            BGFX_SAMPLER_MAG_ANISOTROPIC |
            BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP |
            BGFX_SAMPLER_U_BORDER | BGFX_SAMPLER_V_BORDER | BGFX_SAMPLER_BORDER_COLOR(2)
        );

        _passes.clear();
        _passes.reserve(_config.maxLightAmount * _config.cascadeAmount);
        for(uint16_t index = 0; index < _config.maxLightAmount; ++index)
        {
            for (uint8_t casc = 0; casc < _config.cascadeAmount; ++casc)
            {
                _passes.emplace_back().init(*this, index, casc);
            }
        }
    }

    void ShadowRenderer::shutdown() noexcept
    {
        for (auto& pass : _passes)
        {
            pass.shutdown();
        }
        if (_cam)
        {
            _cam->getRenderGraph().removeNode(_renderGraph.id());
        }
        _renderGraph.clear();
        _passes.clear();

        _program.reset();
        _cam.reset();
        _scene.reset();
        _app.reset();
    }

    void ShadowRenderer::update(float deltaTime)
    {
        updateLights();
        updateCamera();
    }

    bool ShadowRenderer::isEnabled() const noexcept
    {
        return _cam && _cam->isEnabled();
    }

    void ShadowRenderer::updateLights() noexcept
    {
        if (!_cam)
        {
            return;
        }
        auto view = _cam->createEntityView<DirectionalLight>();

        size_t i = 0;
        auto changed = false;
        bool finished = false;
        for (auto entity : view)
        {
            for (auto casc = 0; casc < _config.cascadeAmount; ++casc)
            {
                if (_passes[i].configure(entity))
                {
                    changed = true;
                }
                ++i;
                if (i >= _passes.size())
                {
                    finished = true;
                    break;
                }
            }
            if (finished)
            {
                break;
            }
        }
        for (; i < _passes.size(); ++i)
        {
            if (_passes[i].configure())
            {
                changed = true;
            }
        }
        if (changed)
        {
            _cam->renderReset();
        }
    }

    void ShadowRenderer::updateCamera() noexcept
    {
        if (!_cam)
        {
            return;
        }

        auto proj = _cam->getProjectionMatrix();
        Frustum frust(proj);

        _camProjViews.clear();
        auto step = 1.F / float(_config.cascadeAmount);

        auto cascSliceDistri = [](float v)
        {
            return Easing::quadraticIn(v, 0.F, 1.F);
        };

        for (auto casc = 0; casc < _config.cascadeAmount; ++casc)
        {
            auto nearFactor = cascSliceDistri(step * casc);
            auto farFactor = cascSliceDistri(step * (casc + 1));
            auto cascFrust = frust.getSlice(nearFactor, farFactor);
            auto cascProj = cascFrust.getAlignedProjectionMatrix();
            cascProj *= _cam->getModelMatrix();
            _camProjViews.emplace_back(cascProj);
        }
    }

    glm::mat4 ShadowRenderer::getProjViewMatrix(OptionalRef<const Transform> trans, uint8_t cascade) const noexcept
    {
        auto projView = _camProjViews[cascade % _camProjViews.size()];
        if (!trans)
        {
            return projView;
        }

        Frustum frust = projView * trans->getWorldMatrix();
        BoundingBox bb = frust.getBoundingBox();
        bb.expand(_config.mapMargin * bb.size());
        bb.snap(glm::uvec2(_config.mapSize));
        return bb.getOrtho();
    }

    const ShadowRenderer::Config& ShadowRenderer::getConfig() const noexcept
    {
        return _config;
    }

    bgfx::ProgramHandle ShadowRenderer::getProgramHandle() const noexcept
    {
        return _program->getHandle();
    }

    bgfx::TextureHandle ShadowRenderer::getTextureHandle() const noexcept
    {
        return _tex->getHandle();
    }

    OptionalRef<Camera> ShadowRenderer::getCamera() noexcept
    {
        return _cam;
    }

    OptionalRef<Scene> ShadowRenderer::getScene() noexcept
    {
        return _scene;
    }

    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps
    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps

    glm::mat4 ShadowRenderer::getMapMatrix(Entity entity, uint8_t cascade) const noexcept
    {
        auto trans = _scene->getComponent<const Transform>(entity);
        auto proj = getProjViewMatrix(trans, cascade);
        if (trans)
        {
            proj *= trans->getWorldInverse();
        }

        auto caps = bgfx::getCaps();
        const float sy = caps->originBottomLeft ? 0.5f : -0.5f;
        const float sz = caps->homogeneousDepth ? 0.5f : 1.0f;
        const float tz = caps->homogeneousDepth ? 0.5f : 0.0f;
        glm::mat4 crop
        {
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f,   sy, 0.0f, 0.0f,
            0.0f, 0.0f, sz,   0.0f,
            0.5f, 0.5f, tz,   1.0f,
        };

        return crop * proj;
    }

    void ShadowRenderer::renderReset() noexcept
    {
        _renderGraph.clear();
        _renderGraph.setName("Shadows");
        _renderGraph.getWriteResources().add<Texture>("shadow");
        for (auto& pass : _passes)
        {
            _renderGraph.addPass(pass);
        }
        if (_cam)
        {
            _cam->getRenderGraph().setChild(_renderGraph);
        }
    }

    ShadowRenderComponent::ShadowRenderComponent(ShadowRenderer& renderer) noexcept
        : _renderer(renderer)
        , _shadowMapUniform{ bgfx::kInvalidHandle }
        , _shadowDataUniform{ bgfx::kInvalidHandle }
        , _shadowTransBuffer{ bgfx::kInvalidHandle }
    {
        _shadowTransLayout.begin()
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color2, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color3, 4, bgfx::AttribType::Float)
        .end();
    }

    void ShadowRenderComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        _shadowMapUniform = bgfx::createUniform("s_shadowMap", bgfx::UniformType::Sampler);
        _shadowDataUniform = bgfx::createUniform("u_shadowData", bgfx::UniformType::Vec4);
        _shadowTransBuffer = bgfx::createDynamicVertexBuffer(1, _shadowTransLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
    }

    void ShadowRenderComponent::shutdown() noexcept
    {
        std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms{
            _shadowMapUniform, _shadowDataUniform
        };
        for (auto& uniform : uniforms)
        {
            if (isValid(uniform))
            {
                bgfx::destroy(uniform);
                uniform.get().idx = bgfx::kInvalidHandle;
            }
        }
        if (isValid(_shadowTransBuffer))
        {
            bgfx::destroy(_shadowTransBuffer);
            _shadowTransBuffer.idx = bgfx::kInvalidHandle;
        }

        _cam.reset();
        _scene.reset();
    }

    void ShadowRenderComponent::update(float deltaTime) noexcept
    {
        auto lights = _cam->createEntityView<DirectionalLight>();

        auto cascadeAmount = _renderer.getConfig().cascadeAmount;
        uint32_t size = lights.size_hint() * cascadeAmount;

        VertexDataWriter writer(_shadowTransLayout, size);
        uint32_t index = 0;

        for (auto entity : lights)
        {
            for (auto casc = 0; casc < cascadeAmount; ++casc)
            {
                auto mtx = _renderer.getMapMatrix(entity, casc);
                // not sure why but the shader reads the data by rows
                mtx = glm::transpose(mtx);
                writer.write(bgfx::Attrib::Color0, index, mtx[0]);
                writer.write(bgfx::Attrib::Color1, index, mtx[1]);
                writer.write(bgfx::Attrib::Color2, index, mtx[2]);
                writer.write(bgfx::Attrib::Color3, index, mtx[3]);
                ++index;
            }

        }
        auto data = writer.finish();
        if (!data.empty())
        {
            bgfx::update(_shadowTransBuffer, 0, data.copyMem());
        }
    }

    void ShadowRenderComponent::beforeRenderEntity(Entity entity, IRenderGraphContext& context) noexcept
    {
        configureUniforms(context);
    }

    void ShadowRenderComponent::configureUniforms(IRenderGraphContext& context) const noexcept
    {
        auto& encoder = context.getEncoder();

        encoder.setBuffer(RenderSamplers::SHADOW_TRANS, _shadowTransBuffer, bgfx::Access::Read);

        auto& config = _renderer.getConfig();
        auto texelSize = 1.F / config.mapSize;
        glm::vec4 smData(texelSize, config.cascadeAmount, config.bias, config.normalBias);
        encoder.setUniform(_shadowDataUniform, glm::value_ptr(smData));

        auto texHandle = _renderer.getTextureHandle();
        encoder.setTexture(RenderSamplers::SHADOW_MAP, _shadowMapUniform, texHandle);
    }

    ShadowDebugRenderComponent::ShadowDebugRenderComponent(ShadowRenderer& renderer) noexcept
        : _renderer(renderer)
        , _colorUniform{ bgfx::kInvalidHandle }
    {
    }

    void ShadowDebugRenderComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _prog = std::make_shared<Program>(StandardProgramType::Unlit);

        _colorUniform = bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4);
    }

    void ShadowDebugRenderComponent::shutdown() noexcept
    {
        std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms = { _colorUniform };
        for (auto& uniform : uniforms)
        {
            if (isValid(uniform.get()))
            {
                bgfx::destroy(uniform);
                uniform.get().idx = bgfx::kInvalidHandle;
            }
        }

        _scene.reset();
        _prog.reset();
    }

    void ShadowDebugRenderComponent::beforeRenderView(IRenderGraphContext& context) noexcept
    {
        auto cam = _renderer.getCamera();
        if (!cam)
        {
            return;
        }
        MeshData meshData;
        meshData.type = MeshType::Transient;

        uint8_t debugColor = 0;
        auto cascadeAmount = _renderer.getConfig().cascadeAmount;

        for (uint8_t casc = 0; casc < cascadeAmount; ++casc)
        {
            auto cascProjView = _renderer.getProjViewMatrix(nullptr, casc);
            meshData += MeshData(Frustum(cascProjView), RectangleMeshType::Outline);
        }
        renderMesh(meshData, debugColor, context);
        ++debugColor;

        auto lights = cam->createEntityView<DirectionalLight>();
        for (auto entity : lights)
        {
            meshData.clear();
            for (auto casc = 0; casc < cascadeAmount; ++casc)
            {
                auto mtx = _renderer.getMapMatrix(entity, casc);
                meshData += MeshData(Frustum(mtx), RectangleMeshType::Outline);
            }
            renderMesh(meshData, debugColor, context);
            ++debugColor;
        }
    }

    void ShadowDebugRenderComponent::renderMesh(MeshData& meshData, uint8_t debugColor, IRenderGraphContext& context) noexcept
    {
        auto& encoder = context.getEncoder();
        auto viewId = context.getViewId();
        auto color = Colors::normalize(Colors::debug(debugColor));
        encoder.setUniform(_colorUniform, glm::value_ptr(color));
        uint64_t state = BGFX_STATE_DEFAULT | BGFX_STATE_PT_LINES;
        auto mesh = meshData.createMesh(_prog->getVertexLayout());
        mesh->render(encoder);
        encoder.setState(state);
        encoder.submit(viewId, _prog->getHandle());
    }
}