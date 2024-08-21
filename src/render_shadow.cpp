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

        _lightTransforms.clear();
        for (auto entity : lights)
        {
            if (auto trans = _scene->getComponent<const Transform>(entity))
            {
                _lightTransforms[entity] = trans->getWorldInverse();
            }
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

        auto bb = _cam->getFrustum().getBoundingBox();
        _camOrtho = bb.getOrtho();
    }

    glm::mat4 ShadowRenderer::getLightMatrix(Entity entity) const noexcept
    {
        glm::mat4 mat = _camOrtho;
        auto itr = _lightTransforms.find(entity);
        if (itr != _lightTransforms.end())
        {
            mat = mat * itr->second;
        }
        return mat;
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
        auto itr = _lightTransforms.find(entity);
        if (itr != _lightTransforms.end())
        {
            viewPtr = glm::value_ptr(itr->second);
        }
        bgfx::setViewTransform(viewId, viewPtr, glm::value_ptr(_camOrtho));

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

    ShadowRenderComponent::ShadowRenderComponent(ShadowRenderer& renderer) noexcept
        : _renderer(renderer)
        , _shadowMapUniform{ bgfx::kInvalidHandle }
        , _lightTransUniform{ bgfx::kInvalidHandle }
        , _lightTrans(1)
    {
    }

    void ShadowRenderComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        _shadowMapUniform = bgfx::createUniform("s_shadowMap", bgfx::UniformType::Sampler);
        _lightTransUniform = bgfx::createUniform("u_dirLightTrans", bgfx::UniformType::Mat4);
    }

    void ShadowRenderComponent::shutdown() noexcept
    {
        std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms{
            _shadowMapUniform, _lightTransUniform
        };
        for (auto& uniform : uniforms)
        {
            if (isValid(uniform))
            {
                bgfx::destroy(uniform);
                uniform.get().idx = bgfx::kInvalidHandle;
            }
        }
        _cam.reset();
        _scene.reset();
    }

    void ShadowRenderComponent::update(float deltaTime) noexcept
    {
        auto view = _cam->createEntityView<DirectionalLight>();
        for (auto& entity : view)
        {
            _lightTrans = _renderer.getLightMatrix(entity);
            break;
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
        encoder.setUniform(_lightTransUniform, glm::value_ptr(_lightTrans));
        if (auto tex = context.getResources().get<Texture>("shadow"))
        {
            encoder.setTexture(RenderSamplers::SHADOW_MAP, _shadowMapUniform, tex->getHandle());
        }
    }
}