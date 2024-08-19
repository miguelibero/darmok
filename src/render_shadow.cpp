#include <darmok/render_shadow.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/program_core.hpp>
#include <darmok/texture.hpp>
#include <darmok/light.hpp>
#include <darmok/scene.hpp>
#include <darmok/math.hpp>
#include "generated/shadow.program.h"

namespace darmok
{
    ShadowRenderer::ShadowRenderer(const glm::uvec2& mapSize) noexcept
        : _mapSize(mapSize)
        , _shadowMapUniform{ bgfx::kInvalidHandle }
        , _lightPosUniform{ bgfx::kInvalidHandle }
        , _lightMtxUniform{ bgfx::kInvalidHandle }
        , _depthScaleOffsetUniform{ bgfx::kInvalidHandle }
        , _shadowFb{ bgfx::kInvalidHandle }
        , _depthScaleOffset(0)
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

        _shadowMapUniform = bgfx::createUniform("s_shadowMap", bgfx::UniformType::Sampler);
        _lightPosUniform = bgfx::createUniform("u_lightPos", bgfx::UniformType::Vec4);
        _lightMtxUniform = bgfx::createUniform("u_lightMtx", bgfx::UniformType::Mat4);
        _depthScaleOffsetUniform = bgfx::createUniform("u_depthScaleOffset", bgfx::UniformType::Vec4);

        const bgfx::Caps* caps = bgfx::getCaps();
        _depthScaleOffset = glm::vec4{ 1.0f, 0.0f, 0.0f, 0.0f };
        if (caps->homogeneousDepth)
        {
            _depthScaleOffset[0] = 0.5f;
            _depthScaleOffset[1] = 0.5f;
        }

        TextureConfig texConfig;
        texConfig.size = _mapSize;
        texConfig.format = bgfx::TextureFormat::D16;
        texConfig.type = TextureType::CubeMap;
        _shadowTex = std::make_unique<Texture>(texConfig, BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL);

        _shadowFb = bgfx::createFrameBuffer(1, &_shadowTex->getHandle(), true);

        updatePointLights();
		renderReset();
    }

    bool ShadowRenderer::updatePointLights() noexcept
    {
        auto view = _cam->createEntityView<PointLight>();
        std::vector<Entity> pointLights;
        pointLights.reserve(view.size_hint());
        for (auto entity : view)
        {
            pointLights.push_back(entity);
        }
        if (_pointLights == pointLights)
        {
            return false;
        }
        _pointLights = pointLights;
        return true;
    }

    void ShadowRenderer::update(float deltaTime)
    {
        if (updatePointLights())
        {
            renderReset();
        }
    }

    void ShadowRenderer::renderReset() noexcept
    {
        if (!_cam)
		{
            return;
		}
        _pointLightsByViewId.clear();
        for (size_t i = 0; i < _pointLights.size(); ++i)
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

        bgfx::destroy(_shadowMapUniform);
        bgfx::destroy(_lightPosUniform);
        bgfx::destroy(_lightMtxUniform);
        bgfx::destroy(_depthScaleOffsetUniform);
        bgfx::destroy(_shadowFb);

        _shadowProg.reset();
        _shadowTex.reset();

        _pointLights.clear();
        _pointLightsByViewId.clear();

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
        bgfx::setUniform(_depthScaleOffsetUniform, glm::value_ptr(_depthScaleOffset));
        bgfx::setViewRect(viewId, 0, 0, _mapSize.x, _mapSize.y);
        bgfx::setViewFrameBuffer(viewId, _shadowFb);
        bgfx::setViewClear(viewId
            , BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
            , 0x303030ff, 1.0f, 0
        );

        auto entity = _pointLights[_pointLightsByViewId.size()];
        _pointLightsByViewId[entity] = viewId;
    }

    void ShadowRenderer::renderPassExecute(IRenderGraphContext& context) noexcept
    {
        auto& encoder = context.getEncoder();
        auto viewId = context.getViewId();

        static const uint64_t renderState = BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CCW
            | BGFX_STATE_MSAA
        ;

        auto lightEntity = _pointLightsByViewId[viewId];

        auto pointLight = _scene->getComponent<const PointLight>(lightEntity);
        auto projSize = pointLight->getRadius() * 100.F;
        glm::mat4 lightView(1);
        glm::mat4 lightProj = Math::ortho(-glm::vec2(projSize), glm::vec2(projSize), 100.0f, 0.0f);
        if (auto trans = _scene->getComponent<const Transform>(lightEntity))
        {
            lightView = glm::lookAt(trans->getWorldPosition(), glm::vec3(0), glm::vec3(0, 1, 0));
        }
        bgfx::setViewTransform(viewId, glm::value_ptr(lightView), glm::value_ptr(lightProj));

        auto renderables = _cam->createEntityView<Renderable>();
        for (auto entity : renderables)
        {
            auto renderable = _scene->getComponent<const Renderable>(entity);
            if (!renderable->valid())
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

        context.getResources().setRef<Texture>(*_shadowTex, "shadow");
    }
}