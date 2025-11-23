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
#include <darmok/scene_filter.hpp>
#include <darmok/string.hpp>
#include <darmok/transform.hpp>
#include <darmok/glm_serialize.hpp>
#include "generated/shaders/shadow.program.h"
#include "detail/render_samplers.hpp"

namespace darmok
{
    ShadowRenderPass::ShadowRenderPass()
        : _fb{ bgfx::kInvalidHandle }
        , _lightEntity{ entt::null }
        , _part{ 0 }
    {
    }

    void ShadowRenderPass::configure(Entity entity, uint8_t part) noexcept
    {
        if (_lightEntity == entity && _part == part)
        {
            return;
        }
        _lightEntity = entity;
        _part = part;
        configureView();
    }

    void ShadowRenderPass::init(ShadowRenderer& renderer, uint16_t index) noexcept
    {
        _renderer = renderer;

        bgfx::Attachment attach;
        attach.init(renderer.getTextureHandle(), bgfx::Access::Write, index);
        _fb = bgfx::createFrameBuffer(1, &attach);
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
        if (!_renderer)
        {
            return viewId;
        }
        _viewId = viewId;
        configureView();
        return ++viewId;
    }

    void ShadowRenderPass::configureView() noexcept
    {
        if (!_viewId)
        {
            return;
        }
        auto viewId = _viewId.value();
        std::string name = "Shadow light ";
        if (_lightEntity == entt::null)
        {
            name += "(not used)";
        }
        else
        {
            name += std::to_string(_lightEntity);
            if (_part >= 0)
            {
                name += " part " + std::to_string(_part);
            }
        }
        if (auto cam = _renderer->getCamera())
        {
            name = cam->getViewName(name);
        }
        bgfx::setViewName(viewId, name.c_str());

        auto size = _renderer->getConfig().mapSize;
        bgfx::setViewRect(viewId, 0, 0, size, size);
        bgfx::setViewFrameBuffer(viewId, _fb);
        bgfx::setViewClear(viewId, BGFX_CLEAR_DEPTH);
    }

    void ShadowRenderPass::render(bgfx::Encoder& encoder) noexcept
    {
        if (!_viewId)
        {
            return;
        }
        auto viewId = _viewId.value();
        encoder.touch(viewId);

        if (_lightEntity == entt::null || !_renderer || !_renderer->isEnabled())
        {
            return;
        }
        auto scene = _renderer->getScene();
        if (!scene)
        {
            return;
        }

        auto lightTrans = scene->getComponent<const Transform>(_lightEntity);
        auto view = _renderer->getLightViewMatrix(lightTrans);
        glm::mat4 proj(1.F);

        if (scene->hasComponent<DirectionalLight>(_lightEntity))
        {
            proj = _renderer->getDirLightProjMatrix(lightTrans, _part);
        }
        else if (auto spotLight = scene->getComponent<SpotLight>(_lightEntity))
        {
            proj = _renderer->getSpotLightProjMatrix(spotLight.value());
        }
        else if (auto pointLight = scene->getComponent<PointLight>(_lightEntity))
        {
            proj = _renderer->getPointLightProjMatrix(pointLight.value(), _part);
        }
        bgfx::setViewTransform(viewId, glm::value_ptr(view), glm::value_ptr(proj));

        renderEntities(viewId, encoder);
    }

    void ShadowRenderPass::renderEntities(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        static const uint64_t renderState =
            BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CCW
            ;

        auto scene = _renderer->getScene();
        auto cam = _renderer->getCamera();

        auto entities = cam->getEntities<Renderable>();
        for (auto entity : entities)
        {
            if (entity == _lightEntity)
            {
                continue;
            }
            auto renderable = scene->getComponent<const Renderable>(entity);
            if (!renderable->valid())
            {
                continue;
            }
            if (renderable->getMaterial()->primitiveType == Material::Definition::Line)
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
        : _config{ config }
        , _crop{ 1 }
        , _shadowMapUniform{ bgfx::kInvalidHandle }
        , _shadowData1Uniform{ bgfx::kInvalidHandle }
        , _shadowData2Uniform{ bgfx::kInvalidHandle }
        , _shadowTransBuffer{ bgfx::kInvalidHandle }
        , _shadowLightDataBuffer{ bgfx::kInvalidHandle }
        , _dirAmount{ 0 }
        , _spotAmount{0}
        , _pointAmount{0}
    {
        _shadowTransLayout.begin()
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color2, 4, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color3, 4, bgfx::AttribType::Float)
        .end();
        _shadowLightDataLayout.begin()
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float)
        .end();
    }

    expected<void, std::string> ShadowRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
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

        Program::Definition shadowProgDef;
        auto result = protobuf::readStaticMem(shadowProgDef, shadow_program);
        if (result)
        {
            _program = std::make_unique<Program>(shadowProgDef);
        }
        
        Texture::Config texConfig;
        *texConfig.mutable_size() = convert<protobuf::Uvec2>(glm::uvec2{ _config.mapSize });
        texConfig.set_layers(_config.maxPassAmount);
        texConfig.set_format(Texture::Definition::D16);
        texConfig.set_type(Texture::Definition::Texture2D);

        _tex = std::make_unique<Texture>(texConfig,
            BGFX_TEXTURE_RT | BGFX_SAMPLER_COMPARE_LEQUAL |
            BGFX_SAMPLER_MAG_POINT |
            // BGFX_SAMPLER_MAG_ANISOTROPIC |
            BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP |
            BGFX_SAMPLER_U_BORDER | BGFX_SAMPLER_V_BORDER | BGFX_SAMPLER_BORDER_COLOR(2)
        );

        _passes.clear();
        _passes.reserve(_config.maxPassAmount);
        for(uint16_t index = 0; index < _config.maxPassAmount; ++index)
        {
            auto& pass = _passes.emplace_back();
            pass.init(*this, index);
        }

        _shadowMapUniform = bgfx::createUniform("s_shadowMap", bgfx::UniformType::Sampler);
        _shadowData1Uniform = bgfx::createUniform("u_shadowData1", bgfx::UniformType::Vec4);
        _shadowData2Uniform = bgfx::createUniform("u_shadowData2", bgfx::UniformType::Vec4);
        _shadowTransBuffer = bgfx::createDynamicVertexBuffer(1, _shadowTransLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
        _shadowLightDataBuffer = bgfx::createDynamicVertexBuffer(1, _shadowLightDataLayout, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
    }

    expected<void, std::string> ShadowRenderer::shutdown() noexcept
    {
        for (auto& pass : _passes)
        {
            pass.shutdown();
        }
        _passes.clear();

        const std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms{
            _shadowMapUniform, _shadowData1Uniform, _shadowData2Uniform
        };
        for (auto& uniform : uniforms)
        {
            if (isValid(uniform))
            {
                bgfx::destroy(uniform);
                uniform.get().idx = bgfx::kInvalidHandle;
            }
        }
        const std::vector<std::reference_wrapper<bgfx::DynamicVertexBufferHandle>> buffers{
            _shadowTransBuffer, _shadowLightDataBuffer
        };
        for (auto& buffer : buffers)
        {
            if (isValid(buffer))
            {
                bgfx::destroy(buffer);
                buffer.get().idx = bgfx::kInvalidHandle;
            }
        }

        _program.reset();
        _cam.reset();
        _scene.reset();
        _app.reset();
    }

    expected<void, std::string> ShadowRenderer::update(float deltaTime) noexcept
    {
        updateLights();
        updateCamera();
        updateBuffers();
    }

    bool ShadowRenderer::isEnabled() const noexcept
    {
        return _cam && _cam->isEnabled();
    }

    const size_t ShadowRenderer::_pointLightFaceAmount = 6;

    void ShadowRenderer::updateLights() noexcept
    {
        if (!_cam || !_scene)
        {
            return;
        }

        size_t passIdx = 0;
        _dirAmount = 0;
        _spotAmount = 0;
        _pointAmount = 0;

        auto configurePasses = [this, &passIdx](Entity entity, size_t passAmount) -> bool
        {
            if (passIdx > _passes.size() - passAmount)
            {
                return false;
            }
            for (auto i = 0; i < passAmount; ++i)
            {
                _passes[passIdx].configure(entity, i);
                ++passIdx;
            }
            return true;
        };

        for (auto entity : _cam->getEntities<DirectionalLight>())
        {
            auto light = _scene->getComponent<const DirectionalLight>(entity);
            if (light->getShadowType() == LightDefinition::NoShadow)
            {
                continue;
            }
            if (!configurePasses(entity, _config.cascadeAmount))
            {
                break;
            }
            ++_dirAmount;
        }
        for (auto entity : _cam->getEntities<SpotLight>())
        {
            auto light = _scene->getComponent<const SpotLight>(entity);
            if (light->getShadowType() == LightDefinition::NoShadow)
            {
                continue;
            }
            if (!configurePasses(entity, 1))
            {
                break;
            }
            ++_spotAmount;
        }
        for (auto entity : _cam->getEntities<PointLight>())
        {
            auto light = _scene->getComponent<const PointLight>(entity);
            if (light->getShadowType() == LightDefinition::NoShadow)
            {
                continue;
            }
            if (!configurePasses(entity, _pointLightFaceAmount))
            {
                break;
            }
            ++_pointAmount;
        }
        for (; passIdx < _passes.size(); ++passIdx)
        {
            _passes[passIdx].configure();
        }
    }

    void ShadowRenderer::updateCamera() noexcept
    {
        if (!_cam)
        {
            return;
        }

        auto view = _cam->getViewMatrix();
        Frustum frust(_cam->getProjectionInverse(), true);

        auto step = 1.F / float(_config.cascadeAmount);
        auto easing = _config.cascadeEasing;
        auto cascSliceDistri = [step, easing](int casc)
        {
            float v = step * casc;
            return Easing::apply(easing, v, 0.F, 1.F);
        };

        _camProjs.clear();
        auto margin = _config.cascadeMargin;
        for (auto casc = 0; casc < _config.cascadeAmount; ++casc)
        {
            auto nearFactor = cascSliceDistri(casc);
            auto farFactor = cascSliceDistri(casc + 1);
            nearFactor = std::max(nearFactor - margin, 0.F);
            farFactor = std::min(farFactor + margin, 1.F);

            auto cascFrust = frust.getSlice(nearFactor, farFactor);
            auto cascProj = cascFrust.getAlignedProjectionMatrix();
            cascProj *= view;
            _camProjs.emplace_back(cascProj);
        }
    }

    size_t ShadowRenderer::getShadowMapAmount() const noexcept
    {
        return (_dirAmount * _config.cascadeAmount) + _spotAmount + (_pointAmount * _pointLightFaceAmount);
    }

    enum class ShadowLightType
    {
        Dir,
        Spot,
        Point
    };

    void ShadowRenderer::updateBuffers() noexcept
    {
        auto amount = static_cast<uint32_t>(getShadowMapAmount());
        VertexDataWriter transWriter{_shadowTransLayout, amount };
        VertexDataWriter lightDataWriter{_shadowLightDataLayout, amount};

        uint32_t index = 0;

        auto addElement = [&transWriter, &lightDataWriter, &index]
            (Entity entity, const glm::mat4& mtx, ShadowLightType lightType, ShadowType shadowType)
        {
            // not sure why but the shader reads the data by rows
            // TODO: try to set trans buffer type to mat4
            auto tmtx = glm::transpose(mtx);
            transWriter.write(bgfx::Attrib::Color0, index, tmtx[0]);
            transWriter.write(bgfx::Attrib::Color1, index, tmtx[1]);
            transWriter.write(bgfx::Attrib::Color2, index, tmtx[2]);
            transWriter.write(bgfx::Attrib::Color3, index, tmtx[3]);

            const glm::vec4 lightData{ entity, lightType, toUnderlying(shadowType), 0.f };
            lightDataWriter.write(bgfx::Attrib::Color0, index, lightData);

            ++index;
        };

        for (auto entity : _cam->getEntities<DirectionalLight>())
        {
            auto light = _scene->getComponent<const DirectionalLight>(entity);
            auto shadowType = light->getShadowType();
            if (shadowType == LightDefinition::NoShadow)
            {
                continue;
            }
            auto lightTrans = _scene->getComponent<const Transform>(entity);
            for (auto casc = 0; casc < _config.cascadeAmount; ++casc)
            {
                auto mtx = getDirLightMapMatrix(lightTrans, casc);
                addElement(entity, mtx, ShadowLightType::Dir, shadowType);
            }
        }

        for (auto entity : _cam->getEntities<SpotLight>())
        {
            auto light = _scene->getComponent<const SpotLight>(entity);
            auto shadowType = light->getShadowType();
            if (shadowType == LightDefinition::NoShadow)
            {
                continue;
            }
            auto lightTrans = _scene->getComponent<const Transform>(entity);
            auto mtx = getSpotLightMapMatrix(light.value(), lightTrans);
            addElement(entity, mtx, ShadowLightType::Spot, shadowType);
        }

        for (auto entity : _cam->getEntities<PointLight>())
        {
            auto light = _scene->getComponent<const PointLight>(entity);
            auto shadowType = light->getShadowType();
            if (shadowType == LightDefinition::NoShadow)
            {
                continue;
            }
            auto lightTrans = _scene->getComponent<const Transform>(entity);
            for (auto face = 0; face < _pointLightFaceAmount; ++face)
            {
                auto mtx = getPointLightMapMatrix(light.value(), lightTrans, face);
                addElement(entity, mtx, ShadowLightType::Point, shadowType);
            }
        }

        auto data = transWriter.finish();
        if (!data.empty())
        {
            bgfx::update(_shadowTransBuffer, 0, data.copyMem());
        }
        data = lightDataWriter.finish();
        if (!data.empty())
        {
            bgfx::update(_shadowLightDataBuffer, 0, data.copyMem());
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

    glm::mat4 ShadowRenderer::getLightViewMatrix(const OptionalRef<const Transform>& lightTrans) const noexcept
    {
        if (lightTrans)
        {
            return lightTrans->getWorldInverse();
        }
        return glm::mat4{ 1.f };
    }

    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps
    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/cascaded-shadow-maps

    glm::mat4 ShadowRenderer::getCameraProjMatrix(uint8_t cascade) const noexcept
    {
        if (_camProjs.empty())
        {
            return glm::mat4{ 1.f };
        }
        return _camProjs[cascade % _camProjs.size()];
    }

    glm::mat4 ShadowRenderer::getDirLightProjMatrix(const OptionalRef<const Transform>& lightTrans, uint8_t cascade) const noexcept
    {
        auto mtx = getCameraProjMatrix(cascade);
        if (lightTrans)
        {
            mtx *= lightTrans->getWorldMatrix();
        }
        auto bb = Frustum{ mtx }.getBoundingBox();
        bb.snap(_config.mapSize);
        return bb.getOrtho();
    }

    glm::mat4 ShadowRenderer::getDirLightMatrix(const OptionalRef<const Transform>& lightTrans, uint8_t cascade) const noexcept
    {
        auto proj = getDirLightProjMatrix(lightTrans, cascade);
        auto view = getLightViewMatrix(lightTrans);
        return proj * view;
    }

    glm::mat4 ShadowRenderer::getDirLightMapMatrix(const OptionalRef<const Transform>& lightTrans, uint8_t cascade) const noexcept
    {
        return _crop * getDirLightMatrix(lightTrans, cascade);
    }

    glm::mat4 ShadowRenderer::getSpotLightMatrix(const SpotLight& light, const OptionalRef<const Transform>& lightTrans) const noexcept
    {
        auto proj = getSpotLightProjMatrix(light);
        auto view = getLightViewMatrix(lightTrans);
        return proj * view;
    }

    glm::mat4 ShadowRenderer::getSpotLightProjMatrix(const SpotLight& light) const noexcept
    {
        return Math::perspective(light.getConeAngle() * 2.f, 1.f, _config.nearPlane, light.getRange());
    }

    glm::mat4 ShadowRenderer::getSpotLightMapMatrix(const SpotLight& light, const OptionalRef<const Transform>& lightTrans) const noexcept
    {
        return _crop * getSpotLightMatrix(light, lightTrans);
    }

    glm::mat4 ShadowRenderer::getPointLightMatrix(const PointLight& light, const OptionalRef<const Transform>& lightTrans, uint8_t face) const noexcept
    {
        auto proj = getPointLightProjMatrix(light, face);
        auto view = getLightViewMatrix(lightTrans);
        return proj * view;
    }

    glm::mat4 ShadowRenderer::getPointLightProjMatrix(const PointLight& light, uint8_t face) const noexcept
    {
        static const std::array<glm::quat, _pointLightFaceAmount> rots
        {
            glm::vec3{0.f, 0.f, 0.f},
            glm::vec3{0.f, glm::pi<float>(), 0.f},
            glm::vec3{0.f, glm::half_pi<float>(), 0.f},
            glm::vec3{0.f, -glm::half_pi<float>(), 0.f},
            glm::vec3{glm::half_pi<float>(), 0.f, 0.f},
            glm::vec3{-glm::half_pi<float>(), 0.f, 0.f},
        };
        auto proj = Math::perspective(glm::half_pi<float>(), 1.f, _config.nearPlane, light.getRange());
        proj *= glm::mat4_cast(rots[face]);
        return proj;
    }

    glm::mat4 ShadowRenderer::getPointLightMapMatrix(const PointLight& light, const OptionalRef<const Transform>& lightTrans, uint8_t face) const noexcept
    {
        return _crop * getPointLightMatrix(light, lightTrans, face);
    }

    expected<bgfx::ViewId, std::string> ShadowRenderer::renderReset(bgfx::ViewId viewId) noexcept
    {
        for (auto& pass : _passes)
        {
            viewId = pass.renderReset(viewId);
        }
        return viewId;
    }

    expected<void, std::string> ShadowRenderer::render() noexcept
    {
        auto encoder = bgfx::begin();
        for (auto& pass : _passes)
        {
            pass.render(*encoder);
        }
        bgfx::end(encoder);
    }

    expected<void, std::string> ShadowRenderer::beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        configureUniforms(encoder);
    }

    void ShadowRenderer::configureUniforms(bgfx::Encoder& encoder) const noexcept
    {
        encoder.setBuffer(RenderSamplers::SHADOW_TRANS, _shadowTransBuffer, bgfx::Access::Read);
        encoder.setBuffer(RenderSamplers::SHADOW_LIGHT_DATA, _shadowLightDataBuffer, bgfx::Access::Read);
        encoder.setTexture(RenderSamplers::SHADOW_MAP, _shadowMapUniform, getTextureHandle());

        auto texelSize = 1.f / _config.mapSize;
        auto shadowData = glm::vec4{ texelSize, _config.cascadeAmount, _config.bias, _config.normalBias };
        encoder.setUniform(_shadowData1Uniform, glm::value_ptr(shadowData));
        shadowData = glm::vec4{ _dirAmount, _spotAmount, _pointAmount, getShadowMapAmount()};
        encoder.setUniform(_shadowData2Uniform, glm::value_ptr(shadowData));
    }

    ShadowDebugRenderer::ShadowDebugRenderer(ShadowRenderer& renderer) noexcept
        : _renderer(renderer)
    {
    }

    expected<void, std::string> ShadowDebugRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _debugRender.init(app);
        return {};
    }

    expected<void, std::string> ShadowDebugRenderer::shutdown() noexcept
    {
        _debugRender.shutdown();
        _scene.reset();
        return {};
    }

    expected<void, std::string> ShadowDebugRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        if (!_scene)
        {
            return unexpected<std::string>{"renderer not initialized"};
        }
        auto cam = _renderer.getCamera();
        if (!cam)
        {
            return unexpected<std::string>{"camera not found"};
        }
        MeshData meshData;
        meshData.type = Mesh::Definition::Transient;
        uint8_t debugColor = 0;

        auto cascadeAmount = _renderer.getConfig().cascadeAmount;

        for (uint8_t casc = 0; casc < cascadeAmount; ++casc)
        {
            auto cascProjView = _renderer.getCameraProjMatrix(casc);
            meshData += MeshData{ Frustum{cascProjView}, Mesh::Definition::OutlineRectangle };
        }
        _debugRender.renderMesh(meshData, viewId, encoder, debugColor, true);
        ++debugColor;

        auto entities = cam->getEntities<DirectionalLight>();
        for (auto entity : entities)
        {
            meshData.clear();
            auto lightTrans = _scene->getComponent<const Transform>(entity);
            for (auto casc = 0; casc < cascadeAmount; ++casc)
            {
                auto mtx = _renderer.getDirLightMatrix(lightTrans, casc);
                meshData += MeshData{ Frustum{mtx}, Mesh::Definition::OutlineRectangle };
            }

            glm::vec3 lightPos;
            if (lightTrans)
            {
                lightPos = lightTrans->getWorldPosition();
            }
            meshData += MeshData{ Sphere{0.01f, lightPos}, 8 };

            _debugRender.renderMesh(meshData, viewId, encoder, debugColor, true);
            ++debugColor;
        }
        return {};
    }
}
