#ifdef _DEBUG

#include "physics3d_debug_jolt.hpp"
#include <darmok/physics3d_debug.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/input.hpp>
#include <darmok/texture.hpp>
#include <darmok/vertex.hpp>
#include <darmok/text.hpp>
#include <darmok/utf8.hpp>
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

    PhysicsDebugRendererImpl::PhysicsDebugRendererImpl(const Config& config) noexcept
        : _config(config)
        , _viewId(-1)
        , _enabled(false)
        , _meshBatchSize(32 * 1024)
    {
        _solidMeshData.type = MeshType::Transient;
        _wireMeshData.type = MeshType::Transient;
    }

    void PhysicsDebugRendererImpl::init(Camera& cam, Scene& scene, App& app)
    {
        if (auto system = scene.getSceneComponent<PhysicsSystem>())
        {
            _system = system->getImpl();
        }
        _input = app.getInput();
        if (_config.enableEvent)
        {
            _input->addListener("enable", *_config.enableEvent, *this);
        }
        if (_config.program == nullptr)
        {
            _config.program = std::make_shared<Program>(StandardProgramType::Unlit);
            _config.programDefines = { "TEXTURE_DISABLED" };
        }
        _vertexLayout = _config.program->getVertexLayout();
        _cam = cam;
        Initialize();

        cam.getRenderGraph().addPass(*this);
        _colorUniform = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
    }

    void PhysicsDebugRendererImpl::renderReset()
    {
        if (_cam)
        {
            _cam->getRenderGraph().addPass(*this);
        }
    }

    void PhysicsDebugRendererImpl::shutdown()
    {
        if (_cam)
        {
            _cam->getRenderGraph().removePass(*this);
        }
        if (_input)
        {
            _input->removeListener(*this);
        }
        _input.reset();
        _cam.reset();
        _system.reset();
        _viewId = -1;
        if (isValid(_colorUniform))
        {
            bgfx::destroy(_colorUniform);
            _colorUniform.idx = bgfx::kInvalidHandle;
        }
    }

    void PhysicsDebugRendererImpl::onInputEvent(const std::string& tag) noexcept
    {
        setEnabled(!_enabled);
    }

    void PhysicsDebugRendererImpl::renderPassDefine(RenderPassDefinition& def) noexcept
    {
        def.setName("Jolt Physics Debug");
        def.getWriteResources().add<Texture>();
    }

    void PhysicsDebugRendererImpl::renderPassConfigure(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
        _cam->configureView(viewId);
    }

    void PhysicsDebugRendererImpl::renderPassExecute(IRenderGraphContext& context)
    {
        if (!_enabled || !_system)
        {
            return;
        }
        auto joltSystem = _system->getJolt();
        if (!joltSystem)
        {
            return;
        }

        auto& encoder = context.getEncoder();
        _encoder = encoder;

        JPH::BodyManager::DrawSettings settings;
        settings.mDrawShape = true;
        settings.mDrawShapeWireframe = true;
        settings.mDrawWorldTransform = true;
        // settings.mDrawSleepStats = true;

        _cam->beforeRenderView(_viewId);

        joltSystem->DrawBodies(settings, this, nullptr);

        renderMesh(_wireMeshData, EDrawMode::Wireframe);
        renderMesh(_solidMeshData, EDrawMode::Solid);
        renderText();
    }

    bool PhysicsDebugRendererImpl::tryRenderMeshBatch(MeshData& meshData, EDrawMode mode)
    {
        if (meshData.vertices.size() < _meshBatchSize)
        {
            return false;
        }
        renderMesh(meshData, mode);
        return true;
    }

    void PhysicsDebugRendererImpl::renderMesh(MeshData& meshData, EDrawMode mode, const Color& color)
    {
        if (meshData.empty())
        {
            return;
        }
        auto mesh = meshData.createMesh(_vertexLayout);
        meshData.clear();
        renderMesh(*mesh, mode, color);
    }

    void PhysicsDebugRendererImpl::renderText()
    {
        if (!_font)
        {
            return;
        }
        std::unordered_set<Utf8Char> chars;
        for (auto& textData : _textData)
        {
            chars.insert(textData.content.begin(), textData.content.end());
        }
        _font->update(chars);

        MeshData meshData;
        meshData.type = MeshType::Transient;
        for (auto& textData : _textData)
        {
            auto textMeshData = Text::createMeshData(textData.content, *_font);
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

    void PhysicsDebugRendererImpl::setFont(const std::shared_ptr<IFont>& font) noexcept
    {
        _font = font;
    }

    void PhysicsDebugRendererImpl::setMeshBatchSize(size_t size) noexcept
    {
        _meshBatchSize = size;
    }

    void PhysicsDebugRendererImpl::renderMesh(const IMesh& mesh, EDrawMode mode, const Color& color)
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

    void PhysicsDebugRendererImpl::renderSubmit(EDrawMode mode, const Color& color)
    {
        auto v = Colors::normalize(color);
        _encoder->setUniform(_colorUniform, glm::value_ptr(v));

        uint64_t state = BGFX_STATE_DEFAULT;
        if (mode == EDrawMode::Wireframe)
        {
            state |= BGFX_STATE_PT_LINES;
        }

        _encoder->setState(state);
        auto prog = _config.program->getHandle(_config.programDefines);
        _encoder->submit(_viewId, prog);
    }

    void PhysicsDebugRendererImpl::DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg color)
    {
        MeshData data(Line(JoltUtils::convert(from), JoltUtils::convert(to)));
        data *= JoltUtils::convert(color);
        _wireMeshData += data;
        tryRenderMeshBatch(_wireMeshData, EDrawMode::Wireframe);
    }

    void PhysicsDebugRendererImpl::DrawTriangle(JPH::RVec3Arg v1, JPH::RVec3Arg v2, JPH::RVec3Arg v3, JPH::ColorArg color, ECastShadow castShadow)
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

    void PhysicsDebugRendererImpl::DrawText3D(JPH::RVec3Arg pos, const std::string_view& str, JPH::ColorArg color, float height)
    {
        Utf8Vector content;
        Utf8Char::read(str, content);
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

    JPH::DebugRenderer::Batch PhysicsDebugRendererImpl::CreateTriangleBatch(const JPH::DebugRenderer::Triangle* triangles, int triangleCount)
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
        return new JoltMeshBatch(data.createMesh(_vertexLayout));
    }

    JPH::DebugRenderer::Batch PhysicsDebugRendererImpl::CreateTriangleBatch(const JPH::DebugRenderer::Vertex* vertices, int vertexCount, const JPH::uint32* indices, int indexCount)
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
        return new JoltMeshBatch(data.createMesh(_vertexLayout));
    }

    void PhysicsDebugRendererImpl::DrawGeometry(JPH::RMat44Arg modelMatrix, const JPH::AABox& worldSpaceBounds, float inLODScaleSq, JPH::ColorArg modelColor, const GeometryRef& geometry, ECullMode cullMode, ECastShadow castShadow, EDrawMode drawMode)
    {
        if (geometry->mLODs.empty())
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

    void PhysicsDebugRenderer::renderReset()
    {
        _impl->renderReset();
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

    PhysicsDebugRenderer& PhysicsDebugRenderer::setFont(const std::shared_ptr<IFont>& font) noexcept
    {
        _impl->setFont(font);
        return *this;
    }
}

#endif