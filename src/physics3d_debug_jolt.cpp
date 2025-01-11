
#include <darmok/physics3d_debug.hpp>
#include "physics3d_debug_jolt.hpp"

#ifdef JPH_DEBUG_RENDERER

#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/input.hpp>
#include <darmok/texture.hpp>
#include <darmok/vertex.hpp>
#include <darmok/text.hpp>
#include <darmok/utf.hpp>
#include "physics3d_jolt.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace darmok::physics3d
{
    JoltMeshBatch::JoltMeshBatch(std::unique_ptr<IMesh>&& mesh) noexcept
        : mesh(std::move(mesh))
    {
    }

    void JoltMeshBatch::AddRef() noexcept
    {
        JPH::RefTarget<JoltMeshBatch>::AddRef();
    }

    void JoltMeshBatch::Release() noexcept
    {
        if (--mRefCount == 0 && mesh != nullptr)
        {
            mesh.reset();
        }
    }

    JoltPhysicsDebugRenderer::JoltPhysicsDebugRenderer() noexcept
        : _colorUniform(bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4))
        , _program(StandardProgramLoader::load(StandardProgramType::Unlit))
        , _solidMeshData(MeshType::Transient)
        , _wireMeshData(MeshType::Transient)
    {
        Initialize();
    }

    JoltPhysicsDebugRenderer::~JoltPhysicsDebugRenderer() noexcept
    {
        if (isValid(_colorUniform))
        {
            bgfx::destroy(_colorUniform);
            _colorUniform.idx = bgfx::kInvalidHandle;
        }
    }

    void JoltPhysicsDebugRenderer::shutdown()
    {
        std::lock_guard lock(_instanceLock);
        _instance.reset();
    }

    std::unique_ptr<JoltPhysicsDebugRenderer> JoltPhysicsDebugRenderer::_instance;
    std::mutex JoltPhysicsDebugRenderer::_instanceLock;

    void JoltPhysicsDebugRenderer::render(JPH::PhysicsSystem& joltSystem, const Config& config, bgfx::ViewId viewId, bgfx::Encoder& encoder)
    {
        std::lock_guard lock(_instanceLock);
        if (!_instance)
        {
            _instance = std::unique_ptr<JoltPhysicsDebugRenderer>(new JoltPhysicsDebugRenderer());
        }
        _instance->doRender(joltSystem, config, viewId, encoder);
    }

    void JoltPhysicsDebugRenderer::doRender(JPH::PhysicsSystem& joltSystem, const Config& config, bgfx::ViewId viewId, bgfx::Encoder& encoder)
    {
        _config = config;

        _encoder = encoder;
        _viewId = viewId;

        JPH::BodyManager::DrawSettings settings;
        settings.mDrawShape = true;
        settings.mDrawShapeWireframe = true;
        settings.mDrawWorldTransform = true;
        // settings.mDrawSleepStats = true;

        joltSystem.DrawBodies(settings, this, nullptr);

        renderMesh(_wireMeshData, EDrawMode::Wireframe);
        renderMesh(_solidMeshData, EDrawMode::Solid);
        renderText();

        NextFrame();
    }

    bool JoltPhysicsDebugRenderer::tryRenderMeshBatch(MeshData& meshData, EDrawMode mode)
    {
        if (meshData.vertices.size() < _config.meshBatchSize)
        {
            return false;
        }
        renderMesh(meshData, mode);
        return true;
    }

    void JoltPhysicsDebugRenderer::renderMesh(MeshData& meshData, EDrawMode mode, const Color& color)
    {
        if (meshData.empty())
        {
            return;
        }
        auto mesh = createMesh(meshData);
        meshData.clear();
        renderMesh(*mesh, mode, color);
    }

    void JoltPhysicsDebugRenderer::renderText()
    {
        if (!_config.font)
        {
            return;
        }
        std::unordered_set<UtfChar> chars;
        for (auto& textData : _textData)
        {
            chars.insert(textData.content.begin(), textData.content.end());
        }
        _config.font->update(chars);

        MeshData meshData;
        meshData.type = MeshType::Transient;
        for (auto& textData : _textData)
        {
            auto textMeshData = Text::createMeshData(textData.content, *_config.font);
            textMeshData *= textData.color;
            textMeshData *= glm::translate(glm::mat4(textData.height), textData.position);

            meshData += textMeshData;
            tryRenderMeshBatch(meshData, EDrawMode::Solid);
        }
        _textData.clear();
        renderMesh(meshData, EDrawMode::Solid);
    }

    bool PhysicsDebugRendererImpl::isEnabled() const noexcept
    {
        return _enabled;
    }

    void PhysicsDebugRendererImpl::setEnabled(bool enabled) noexcept
    {
        _enabled = enabled;
    }

    void JoltPhysicsDebugRenderer::renderMesh(const IMesh& mesh, EDrawMode mode, const Color& color)
    {
        if (!_encoder)
        {
            return;
        }
        auto& encoder = _encoder.value();
        if (mesh.render(encoder))
        {
            renderSubmit(mode, color);
        }
    }

    void JoltPhysicsDebugRenderer::renderSubmit(EDrawMode mode, const Color& color)
    {
        if (!_encoder || !_viewId)
        {
            return;
        }
        auto v = Colors::normalize(color);
        _encoder->setUniform(_colorUniform, glm::value_ptr(v));

        uint64_t state = BGFX_STATE_DEFAULT;
        if (mode == EDrawMode::Wireframe)
        {
            state |= BGFX_STATE_PT_LINES;
        }

        _encoder->setState(state);
        _encoder->submit(_viewId.value(), _program->getHandle());
    }

    void JoltPhysicsDebugRenderer::DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg color)
    {
        MeshData data(Line(JoltUtils::convert(from), JoltUtils::convert(to)));
        data *= JoltUtils::convert(color);
        _wireMeshData += data;
        tryRenderMeshBatch(_wireMeshData, EDrawMode::Wireframe);
    }

    void JoltPhysicsDebugRenderer::DrawTriangle(JPH::RVec3Arg v1, JPH::RVec3Arg v2, JPH::RVec3Arg v3, JPH::ColorArg color, ECastShadow castShadow)
    {
        MeshData data(darmok::Triangle(JoltUtils::convert(v1), JoltUtils::convert(v2), JoltUtils::convert(v3)));
        data *= JoltUtils::convert(color);
        if (castShadow == ECastShadow::On)
        {
            _solidMeshData += data;
            tryRenderMeshBatch(_solidMeshData, EDrawMode::Solid);
        }
        else
        {
            _wireMeshData += data;
            tryRenderMeshBatch(_wireMeshData, EDrawMode::Wireframe);
        }
    }

    void JoltPhysicsDebugRenderer::DrawText3D(JPH::RVec3Arg pos, const std::string_view& str, JPH::ColorArg color, float height)
    {
        UtfVector content;
        UtfChar::read(str, content);
        if (content.empty())
        {
            return;
        }

        _textData.push_back(TextData{
            .position = JoltUtils::convert(pos),
            .content = content,
            .color = JoltUtils::convert(color),
            .height = height,
        });
    }

    JPH::DebugRenderer::Batch JoltPhysicsDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Triangle* triangles, int triangleCount)
    {
        MeshData data;
        for (int i = 0; i < triangleCount; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                auto vert = triangles[i].mV[j];
                data.vertices.emplace_back(
                    JoltUtils::convert(vert.mPosition),
                    JoltUtils::convert(vert.mUV),
                    JoltUtils::convert(vert.mNormal),
                    JoltUtils::convert(vert.mColor)
                ); 
            }
        }
        return new JoltMeshBatch(createMesh(data));
    }

    JPH::DebugRenderer::Batch JoltPhysicsDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Vertex* vertices, int vertexCount, const JPH::uint32* indices, int indexCount)
    {
        MeshData data;
        for (int i = 0; i < vertexCount; i++)
        {
            auto vert = vertices[i];
            data.vertices.emplace_back(
                JoltUtils::convert(vert.mPosition),
                JoltUtils::convert(vert.mUV),
                JoltUtils::convert(vert.mNormal),
                JoltUtils::convert(vert.mColor)
            );
        }
        for (int i = 0; i < indexCount; i++)
        {
            data.indices.emplace_back(indices[i]);
        }
        return new JoltMeshBatch(createMesh(data));
    }

    std::unique_ptr<IMesh> JoltPhysicsDebugRenderer::createMesh(const MeshData& meshData)
    {
        return meshData.createMesh(_program->getVertexLayout());
    }

    void JoltPhysicsDebugRenderer::DrawGeometry(JPH::RMat44Arg modelMatrix, const JPH::AABox& worldSpaceBounds, float inLODScaleSq, JPH::ColorArg modelColor, const GeometryRef& geometry, ECullMode cullMode, ECastShadow castShadow, EDrawMode drawMode)
    {
        if (!geometry || geometry->mLODs.empty())
        {
            return;
        }
        // TODO: decide lod
        auto& lod = geometry->mLODs[0];
        auto& batch = *(JoltMeshBatch*)lod.mTriangleBatch.GetPtr();

        auto transMtx = JoltUtils::convert(modelMatrix);
        _encoder->setTransform(glm::value_ptr(transMtx));

        batch.mesh->render(_encoder.value());
        renderMesh(*batch.mesh, drawMode, JoltUtils::convert(modelColor));
    }

    PhysicsDebugRendererImpl::PhysicsDebugRendererImpl(const Config& config) noexcept
        : _config(config)
        , _enabled(false)
    {
    }

    void PhysicsDebugRendererImpl::init(Camera& cam, Scene& scene, App& app)
    {
        _cam = cam;
        _scene = scene;
        _input = app.getInput();
        _input->addListener("enable", _config.enableEvents, *this);
    }

    void PhysicsDebugRendererImpl::shutdown()
    {
        if (_input)
        {
            _input->removeListener(*this);
        }
        _cam.reset();
        _scene.reset();
        _input.reset();
        JoltPhysicsDebugRenderer::shutdown();
    }

    void PhysicsDebugRendererImpl::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder)
    {
        if (!_enabled)
        {
            return;
        }
        auto system = _scene->getSceneComponent<PhysicsSystem>();
        if (!system)
        {
            return;
        }
        auto joltSystem = system->getImpl().getJolt();
        if (!joltSystem)
        {
            return;
        }

        JoltPhysicsDebugRenderer::render(joltSystem.value(), _config.render, viewId, encoder);
    }

    void PhysicsDebugRendererImpl::onInputEvent(const std::string& tag) noexcept
    {
        setEnabled(!_enabled);
    }

    PhysicsDebugRenderer::PhysicsDebugRenderer(const Config& config) noexcept
        : _impl(std::make_unique<PhysicsDebugRendererImpl>(config))
    {
    }

    PhysicsDebugRenderer::~PhysicsDebugRenderer() noexcept
    {
        // empty on purpose
    }

    void PhysicsDebugRenderer::init(Camera& cam, Scene& scene, App& app)
    {
        _impl->init(cam, scene, app);
    }

    void PhysicsDebugRenderer::shutdown()
    {
        _impl->shutdown();
    }

    bool PhysicsDebugRenderer::isEnabled() const noexcept
    {
        return _impl->isEnabled();
    }

    PhysicsDebugRenderer& PhysicsDebugRenderer::setEnabled(bool enabled) noexcept
    {
        _impl->setEnabled(enabled);
        return *this;
    }

    void PhysicsDebugRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder)
    {
        _impl->beforeRenderView(viewId, encoder);
    }
}

#else

namespace darmok::physics3d
{
    PhysicsDebugRenderer::PhysicsDebugRenderer(const Config& config) noexcept
        : _impl(std::make_unique<PhysicsDebugRendererImpl>())
    {
    }

    PhysicsDebugRenderer::~PhysicsDebugRenderer() noexcept
    {
        // empty on purpose
    }

    void PhysicsDebugRenderer::init(Camera& cam, Scene& scene, App& app)
    {
    }

    void PhysicsDebugRenderer::shutdown()
    {
    }

    bool PhysicsDebugRenderer::isEnabled() const noexcept
    {
        return false;
    }

    PhysicsDebugRenderer& PhysicsDebugRenderer::setEnabled(bool enabled) noexcept
    {
        return *this;
    }

    void PhysicsDebugRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder)
    {
    }
}

#endif