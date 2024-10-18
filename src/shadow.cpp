#include <darmok/shadow.hpp>
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
        _viewId.reset();
    }

    bgfx::ViewId ShadowRenderPass::renderReset(bgfx::ViewId viewId) noexcept
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
        bgfx::setViewName(viewId, name.c_str());

        auto size = _renderer->getConfig().mapSize;
        bgfx::setViewRect(viewId, 0, 0, size, size);
        bgfx::setViewFrameBuffer(viewId, _fb);
        bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH);

        _viewId = viewId;
        return ++viewId;
    }

    void ShadowRenderPass::render(bgfx::Encoder& encoder) noexcept
    {
        if (!_viewId || !_renderer || !_renderer->isEnabled())
        {
            return;
        }
        if (_lightEntity == entt::null)
        {
            return;
        }
        auto scene = _renderer->getScene();
        if (!scene)
        {
            return;
        }

        auto viewId = _viewId.value();
        auto lightTrans = scene->getComponent<const Transform>(_lightEntity);
        auto proj = _renderer->getLightProjMatrix(lightTrans, _cascade);
        auto view = _renderer->getLightViewMatrix(lightTrans);
        bgfx::setViewTransform(viewId, glm::value_ptr(view), glm::value_ptr(proj));

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

        auto entities = cam->getEntities<Renderable>();
        for (auto entity : entities)
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
            cam->setEntityTransform(entity, encoder);
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
        , _crop(1)
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

    void ShadowRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
		_cam = cam;
		_scene = scene;
		_app = app;

        auto caps = bgfx::getCaps();
        const float sy = caps->originBottomLeft ? 0.5f : -0.5f;
        const float sz = caps->homogeneousDepth ? 0.5f : 1.0f;
        const float tz = caps->homogeneousDepth ? 0.5f : 0.0f;
        _crop = {
            0.5f, 0.0f, 0.0f, 0.0f,
            0.0f,   sy, 0.0f, 0.0f,
            0.0f, 0.0f, sz,   0.0f,
            0.5f, 0.5f, tz,   1.0f,
        };

        ProgramDefinition shadowProgDef;
        shadowProgDef.loadStaticMem(shadow_program);
        _program = std::make_unique<Program>(shadowProgDef);

        TextureConfig texConfig;
        texConfig.size = glm::uvec2(_config.mapSize);
        texConfig.layers = _config.maxLightAmount * _config.cascadeAmount;
        texConfig.format = bgfx::TextureFormat::D16;

        _tex = std::make_unique<Texture>(texConfig,
            BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL |
            BGFX_SAMPLER_MAG_POINT |
            // BGFX_SAMPLER_MAG_ANISOTROPIC |
            BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP |
            BGFX_SAMPLER_U_BORDER | BGFX_SAMPLER_V_BORDER | BGFX_SAMPLER_BORDER_COLOR(2)
        );

        _passes.clear();
        _passes.reserve(_config.maxLightAmount * _config.cascadeAmount);
        for(uint16_t index = 0; index < _config.maxLightAmount; ++index)
        {
            for (uint8_t casc = 0; casc < _config.cascadeAmount; ++casc)
            {
                auto& pass = _passes.emplace_back();
                pass.init(*this, index, casc);
            }
        }

        _shadowMapUniform = bgfx::createUniform("s_shadowMap", bgfx::UniformType::Sampler);
        _shadowDataUniform = bgfx::createUniform("u_shadowData", bgfx::UniformType::Vec4);
        _shadowTransBuffer = bgfx::createDynamicVertexBuffer(1, _shadowTransLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
    }

    void ShadowRenderer::shutdown() noexcept
    {
        for (auto& pass : _passes)
        {
            pass.shutdown();
        }
        _passes.clear();

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

        _program.reset();
        _cam.reset();
        _scene.reset();
        _app.reset();
    }

    void ShadowRenderer::update(float deltaTime)
    {
        updateLights();
        updateCamera();
        updateBuffer();
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
        auto entities = _cam->getEntities<DirectionalLight>();

        size_t i = 0;
        auto changed = false;
        bool finished = false;
        for (auto entity : entities)
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
    }

    void ShadowRenderer::updateCamera() noexcept
    {
        if (!_cam)
        {
            return;
        }

        auto proj = _cam->getProjectionMatrix();
        auto model = _cam->getModelMatrix();
        Frustum frust(proj);

        auto step = 1.F / float(_config.cascadeAmount);
        auto cascSliceDistri = [step](int casc)
        {
            float v = step * casc;
            return Easing::quadraticIn(v, 0.F, 1.F);
        };

        _camProjs.clear();
        auto m = _config.cascadeMargin;
        for (auto casc = 0; casc < _config.cascadeAmount; ++casc)
        {
            auto nearFactor = cascSliceDistri(casc);
            auto farFactor = cascSliceDistri(casc + 1);
            nearFactor = std::max(nearFactor - m, 0.F);
            farFactor = std::min(farFactor + m, 1.F);

            auto cascFrust = frust.getSlice(nearFactor, farFactor);
            auto cascProj = cascFrust.getAlignedProjectionMatrix();
            cascProj *= model;
            _camProjs.emplace_back(cascProj);
        }
    }

    void ShadowRenderer::updateBuffer() noexcept
    {
        auto entities = _cam->getEntities<DirectionalLight>();

        auto cascadeAmount = _config.cascadeAmount;
        uint32_t size = entities.size_hint() * cascadeAmount;

        VertexDataWriter writer(_shadowTransLayout, size);
        uint32_t index = 0;

        for (auto entity : entities)
        {
            for (auto casc = 0; casc < cascadeAmount; ++casc)
            {
                auto lightTrans = _scene->getComponent<const Transform>(entity);
                auto mtx = getLightMapMatrix(lightTrans, casc);
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

    glm::mat4 ShadowRenderer::getProjMatrix(uint8_t cascade) const noexcept
    {
        if (_camProjs.empty())
        {
            return glm::mat4(1);
        }
        return _camProjs[cascade % _camProjs.size()];
    }

    glm::mat4 ShadowRenderer::getLightProjMatrix(OptionalRef<const Transform> lightTrans, uint8_t cascade) const noexcept
    {
        auto proj = getProjMatrix(cascade);
        if (lightTrans)
        {
            proj *= lightTrans->getWorldMatrix();
        }
        auto bb = Frustum(proj).getBoundingBox();
        bb.snap(_config.mapSize);
        return bb.getOrtho();
    }

    glm::mat4 ShadowRenderer::getLightViewMatrix(OptionalRef<const Transform> lightTrans) const noexcept
    {
        if (lightTrans)
        {
            return lightTrans->getWorldInverse();
        }
        return glm::mat4(1);
    }

    glm::mat4 ShadowRenderer::getLightMatrix(OptionalRef<const Transform> lightTrans, uint8_t cascade) const noexcept
    {
        auto proj = getLightProjMatrix(lightTrans, cascade);
        auto view = getLightViewMatrix(lightTrans);
        return proj * view;
    }

    glm::mat4 ShadowRenderer::getLightMapMatrix(OptionalRef<const Transform> lightTrans, uint8_t cascade) const noexcept
    {
        return _crop * getLightMatrix(lightTrans, cascade);
    }

    bgfx::ViewId ShadowRenderer::renderReset(bgfx::ViewId viewId) noexcept
    {
        for (auto& pass : _passes)
        {
            viewId = pass.renderReset(viewId);
        }
        return viewId;
    }

    void ShadowRenderer::render() noexcept
    {
        auto encoder = bgfx::begin();
        for (auto& pass : _passes)
        {
            pass.render(*encoder);
        }
        bgfx::end(encoder);
    }

    void ShadowRenderer::beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        configureUniforms(encoder);
    }

    void ShadowRenderer::configureUniforms(bgfx::Encoder& encoder) const noexcept
    {
        encoder.setBuffer(RenderSamplers::SHADOW_TRANS, _shadowTransBuffer, bgfx::Access::Read);

        auto texelSize = 1.F / _config.mapSize;
        glm::vec4 smData(texelSize, _config.cascadeAmount, _config.bias, _config.normalBias);
        encoder.setUniform(_shadowDataUniform, glm::value_ptr(smData));

        auto texHandle = getTextureHandle();
        encoder.setTexture(RenderSamplers::SHADOW_MAP, _shadowMapUniform, texHandle);
    }

    ShadowDebugRenderer::ShadowDebugRenderer(ShadowRenderer& renderer) noexcept
        : _renderer(renderer)
        , _hasTexturesUniform{ bgfx::kInvalidHandle }
        , _colorUniform{ bgfx::kInvalidHandle }
    {
    }

    void ShadowDebugRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _prog = std::make_shared<Program>(StandardProgramType::Unlit);

        _hasTexturesUniform = bgfx::createUniform("u_hasTextures", bgfx::UniformType::Vec4);
        _colorUniform = bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4);
    }

    void ShadowDebugRenderer::shutdown() noexcept
    {
        std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms = { _hasTexturesUniform, _colorUniform };
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

    void ShadowDebugRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
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
            auto cascProjView = _renderer.getProjMatrix(casc);
            meshData += MeshData(Frustum(cascProjView), RectangleMeshType::Outline);
        }
        renderMesh(meshData, debugColor, viewId, encoder);
        ++debugColor;

        auto entities = cam->getEntities<DirectionalLight>();
        for (auto entity : entities)
        {
            meshData.clear();
            auto lightTrans = _scene->getComponent<Transform>(entity);
            for (auto casc = 0; casc < cascadeAmount; ++casc)
            {
                auto mtx = _renderer.getLightMatrix(lightTrans, casc);
                meshData += MeshData(Frustum(mtx), RectangleMeshType::Outline);
            }

            glm::vec3 lightPos;
            if (lightTrans)
            {
                lightPos = lightTrans->getWorldPosition();
            }
            meshData += MeshData(Sphere(0.01, lightPos), 8);

            renderMesh(meshData, debugColor, viewId, encoder);
            ++debugColor;
        }
    }

    void ShadowDebugRenderer::renderMesh(MeshData& meshData, uint8_t debugColor, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        encoder.setUniform(_hasTexturesUniform, glm::value_ptr(glm::vec4(0)));
        auto color = Colors::normalize(Colors::debug(debugColor));
        encoder.setUniform(_colorUniform, glm::value_ptr(color));
        uint64_t state = BGFX_STATE_DEFAULT | BGFX_STATE_PT_LINES;
        auto mesh = meshData.createMesh(_prog->getVertexLayout());
        mesh->render(encoder);
        encoder.setState(state);
        encoder.submit(viewId, _prog->getHandle());
    }
}