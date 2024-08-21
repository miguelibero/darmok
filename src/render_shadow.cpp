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
#include "render_samplers.hpp"

namespace darmok
{
    ShadowRenderer::ShadowRenderer(const glm::uvec2& mapSize) noexcept
        : _mapSize(mapSize)
        , _shadowFb{ bgfx::kInvalidHandle }
        , _camOrtho(1)
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
        // texConfig.type = TextureType::CubeMap;

        _shadowTex = std::make_unique<Texture>(texConfig, BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL);

        _shadowFb = bgfx::createFrameBuffer(1, &_shadowTex->getHandle(), true);

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
        if (_lights != lights)
        {
            _lights = lights;
            _cam->renderReset();
            return true;
        }
        return false;
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

        auto viewProj = _cam->getProjectionMatrix();
        if (auto camTrans = _cam->getTransform())
        {
            viewProj = viewProj * camTrans->getWorldInverse();
        }
        BoundingBox bb = Frustum(viewProj);
        _camOrtho = Math::ortho(bb.min.x, bb.max.x, bb.min.y, bb.max.y, bb.min.z, bb.max.z);
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

        auto lightEntity = _lightsByViewId[viewId];      

        if (auto dirLight = _scene->getComponent<const DirectionalLight>(lightEntity))
        {
            const void* viewPtr = nullptr;
            if (auto trans = _scene->getComponent<const Transform>(lightEntity))
            {
                viewPtr = glm::value_ptr(trans->getWorldInverse());
            }
            bgfx::setViewTransform(viewId, viewPtr, glm::value_ptr(_camOrtho));
        }
        else
        {
            return;
        }

        renderEntities(viewId, encoder);
        context.getResources().setRef<Texture>(*_shadowTex, "shadow");
    }

    void ShadowRenderer::renderEntities(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        static const uint64_t renderState =
            BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LESS
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
            _cam->beforeRenderEntity(entity, encoder);
            if (!renderable->render(encoder))
            {
                continue;
            }

            encoder.setState(renderState);
            encoder.submit(viewId, _shadowProg->getHandle());
        }
    }

    ShadowRenderComponent::ShadowRenderComponent() noexcept
        : _depthScaleOffset(0)
        , _shadowMapUniform{ bgfx::kInvalidHandle }
        , _depthScaleOffsetUniform{ bgfx::kInvalidHandle }
    {
    }

    void ShadowRenderComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _shadowMapUniform = bgfx::createUniform("s_shadowMap", bgfx::UniformType::Sampler);
        _depthScaleOffsetUniform = bgfx::createUniform("u_depthScaleOffset", bgfx::UniformType::Vec4);

        auto caps = bgfx::getCaps();
        _depthScaleOffset = glm::vec4{ 1.0f, 0.0f, 0.0f, 0.0f };
        if (caps->homogeneousDepth)
        {
            _depthScaleOffset[0] = 0.5f;
            _depthScaleOffset[1] = 0.5f;
        }

        auto supported = 0 != (caps->supported & BGFX_CAPS_TEXTURE_COMPARE_LEQUAL);
    }

    void ShadowRenderComponent::shutdown() noexcept
    {
        std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms{
            _shadowMapUniform, _depthScaleOffsetUniform
        };
        for (auto& uniform : uniforms)
        {
            if (isValid(uniform))
            {
                bgfx::destroy(uniform);
                uniform.get().idx = bgfx::kInvalidHandle;
            }
        }
    }

    void ShadowRenderComponent::renderPassDefine(RenderPassDefinition& def) noexcept
    {
        def.getReadResources().add<Texture>("shadow");
    }

    void ShadowRenderComponent::beforeRenderView(IRenderGraphContext& context) noexcept
    {
        auto& encoder = context.getEncoder();
        encoder.setUniform(_depthScaleOffsetUniform, glm::value_ptr(_depthScaleOffset));
        if (auto tex = context.getResources().get<Texture>("shadow"))
        {
            encoder.setTexture(RenderSamplers::SHADOW_MAP, _shadowMapUniform, tex->getHandle());
        }
    }
}