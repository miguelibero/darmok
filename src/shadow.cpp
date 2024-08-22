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
#include "generated/shadow.program.h"
#include "render_samplers.hpp"

namespace darmok
{
    ShadowRenderer::ShadowRenderer(const glm::uvec2& mapSize, const glm::vec3& mapMargin) noexcept
        : _mapSize(mapSize)
        , _mapMargin(mapMargin)
        , _shadowFb{ bgfx::kInvalidHandle }
        , _camProjView(1)
    {
    }

    void ShadowRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
		_cam = cam;
		_scene = scene;
		_app = app;

        ProgramDefinition shadowProgDef;
        shadowProgDef.loadStaticMem(shadow_program);
        _shadowProg = std::make_unique<Program>(shadowProgDef);

        TextureConfig texConfig;
        texConfig.size = _mapSize;
        texConfig.format = bgfx::TextureFormat::D16;
        texConfig.layers = 256; // max amount of lights
        // texConfig.type = TextureType::CubeMap;

        _shadowTex = std::make_unique<Texture>(texConfig,
            BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL |
            BGFX_SAMPLER_MAG_POINT | 
            BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP |
            BGFX_SAMPLER_U_BORDER | BGFX_SAMPLER_V_BORDER | BGFX_SAMPLER_BORDER_COLOR(0xFFFFFF)
        );
        _shadowFb = bgfx::createFrameBuffer(1, &_shadowTex->getHandle());

        updateLights();
    }

    bool ShadowRenderer::updateLights() noexcept
    {
        auto dirView = _cam->createEntityView<DirectionalLight>();
        auto pointView = _cam->createEntityView<PointLight>();
        auto spotView = _cam->createEntityView<SpotLight>();
        std::vector<Entity> lights;
        lights.reserve(dirView.size_hint()
            + pointView.size_hint()
            + spotView.size_hint()
        );

        for (auto entity : dirView)
        {
            lights.push_back(entity);
        }
        for (auto entity : pointView)
        {
            lights.push_back(entity);
        }
        for (auto entity : spotView)
        {
            lights.push_back(entity);
        }

        if (_lights == lights)
        {
            return false;
        }

        _lights = lights;
        _cam->renderReset();
        return true;
    }

    void ShadowRenderer::update(float deltaTime)
    {
        updateLights();
        updateCamera();
    }

    void ShadowRenderer::updateCamera() noexcept
    {
        if (!_cam)
        {
            return;
        }

        _camProjView = _cam->getProjectionMatrix();
        if (auto trans = _cam->getTransform())
        {
            _camProjView *= trans->getWorldInverse();
        }
    }

    glm::mat4 ShadowRenderer::getLightProjMatrix(OptionalRef<const Transform> trans) const noexcept
    {
        if (!trans)
        {
            return _camProjView;
        }

        Frustum frust = _camProjView * trans->getWorldMatrix();
        BoundingBox bb = frust.getBoundingBox();
        bb.expand(_mapMargin * bb.size());
        return bb.getOrtho();
    }

    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps

    glm::mat4 ShadowRenderer::getLightMapMatrix(Entity entity) const noexcept
    {
        auto trans = _scene->getComponent<const Transform>(entity);
        auto proj = getLightProjMatrix(trans);
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
        if (!_cam)
		{
            return;
		}
        _lightsByViewId.clear();
        for (size_t i = 0; i < _lights.size(); ++i)
        {
            _cam->getRenderGraph().addPass(*this);
        }
    }

    void ShadowRenderer::shutdown() noexcept
    {
        if (_cam)
		{
			_cam->getRenderGraph().removePass(*this);
		}

        bgfx::destroy(_shadowFb);

        _shadowProg.reset();
        _shadowTex.reset();

        _lights.clear();
        _lightsByViewId.clear();

		_cam.reset();
		_scene.reset();
		_app.reset();
    }

    void ShadowRenderer::renderPassDefine(RenderPassDefinition& def) noexcept
    {
		def.setName("Shadow");
		def.getWriteResources().add<Texture>("shadow");
    }

    void ShadowRenderer::renderPassConfigure(bgfx::ViewId viewId) noexcept
    {
        bgfx::setViewRect(viewId, 0, 0, _mapSize.x, _mapSize.y);
        bgfx::setViewFrameBuffer(viewId, _shadowFb);
        bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH);

        auto entity = _lights[_lightsByViewId.size()];
        _lightsByViewId[viewId] = entity;
    }

    void ShadowRenderer::renderPassExecute(IRenderGraphContext& context) noexcept
    {
        auto& encoder = context.getEncoder();
        auto viewId = context.getViewId();
        auto entity = _lightsByViewId[viewId];      

        const void* viewPtr = nullptr;
        glm::mat4 proj = _camProjView;
        if(auto trans = _scene->getComponent<const Transform>(entity))
        {
            viewPtr = glm::value_ptr(trans->getWorldInverse());
            proj = getLightProjMatrix(trans);
        }
        bgfx::setViewTransform(viewId, viewPtr, glm::value_ptr(proj));

        renderEntities(viewId, encoder);
        context.getResources().setRef<Texture>(*_shadowTex, "shadow");
    }

    void ShadowRenderer::renderEntities(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        static const uint64_t renderState =
            BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_CULL_CCW
            | BGFX_STATE_MSAA
            ;

        auto renderables = _cam->createEntityView<Renderable>();
        for (auto entity : renderables)
        {
            auto renderable = _scene->getComponent<const Renderable>(entity);
            if (!renderable->valid())
            {
                continue;
            }
            if (renderable->getMaterial()->getPrimitiveType() == MaterialPrimitiveType::Line)
            {
                continue;
            }
            _cam->beforeRenderEntity(entity, encoder);
            if (!renderable->render(encoder))
            {
                continue;
            }

            encoder.setState(renderState);
            encoder.submit(viewId, _shadowProg->getHandle());
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
        VertexDataWriter writer(_shadowTransLayout, uint32_t(lights.size_hint()));
        uint32_t index = 0;

        for (auto entity : lights)
        {
            auto mtx = _renderer.getLightMapMatrix(entity);
            // not sure why but the shader reads the data by rows
            mtx = glm::transpose(mtx);
            writer.write(bgfx::Attrib::Color0, index, mtx[0]);
            writer.write(bgfx::Attrib::Color1, index, mtx[1]);
            writer.write(bgfx::Attrib::Color2, index, mtx[2]);
            writer.write(bgfx::Attrib::Color3, index, mtx[3]);
            ++index;
        }
        auto data = writer.finish();
        if (!data.empty())
        {
            bgfx::update(_shadowTransBuffer, 0, data.copyMem());
        }
    }

    void ShadowRenderComponent::renderPassDefine(RenderPassDefinition& def) noexcept
    {
        def.getReadResources().add<Texture>("shadow");
    }

    void ShadowRenderComponent::beforeRenderView(IRenderGraphContext& context) noexcept
    {
    }

    void ShadowRenderComponent::beforeRenderEntity(Entity entity, IRenderGraphContext& context) noexcept
    {
        configureUniforms(context);
    }

    void ShadowRenderComponent::configureUniforms(IRenderGraphContext& context) const noexcept
    {
        auto& encoder = context.getEncoder();

        encoder.setBuffer(RenderSamplers::SHADOW_TRANS, _shadowTransBuffer, bgfx::Access::Read);

        if (auto tex = context.getResources().get<Texture>("shadow"))
        {
            auto texelSize = glm::vec2(1.F) / glm::vec2(tex->getSize());
            glm::vec4 smData(texelSize, 0, 0);
            encoder.setUniform(_shadowDataUniform, glm::value_ptr(smData));
            encoder.setTexture(RenderSamplers::SHADOW_MAP, _shadowMapUniform, tex->getHandle());
        }
    }
}