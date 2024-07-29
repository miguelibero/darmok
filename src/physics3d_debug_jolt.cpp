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

    PhysicsDebugRendererImpl::PhysicsDebugRendererImpl(PhysicsSystemImpl& system, const Config& config) noexcept
        : _config(config)
        , _viewId(-1)
        , _enabled(true)
        , _system(system)
        , _meshBatchSize(32 * 1024)
    {
        _solidMeshData.config.type = MeshType::Transient;
        _wireMeshData.config.type = MeshType::Transient;
    }

    void PhysicsDebugRendererImpl::init(Camera& cam, Scene& scene, App& app)
    {
        _input = app.getInput();
        if (_config.enableEvent)
        {
            _input->addListener("enable", *_config.enableEvent, *this);
        }
        if (_config.material == nullptr)
        {
            _config.material = std::make_shared<Material>();
            auto diffuse = app.getAssets().getColorTextureLoader()(Colors::white());
            _config.material->setTexture(MaterialTextureType::Diffuse, diffuse);
        }
        _vertexLayout = _config.material->getProgram()->getVertexLayout();
        _cam = cam;
        Initialize();

        cam.getRenderGraph().addPass(*this);
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
        _viewId = -1;
    }

    void PhysicsDebugRendererImpl::onInputEvent(const std::string& tag) noexcept
    {
        setEnabled(!_enabled);
    }

    void PhysicsDebugRendererImpl::renderPassDefine(RenderPassDefinition& def) noexcept
    {
        def.setName("Jolt Physics Debug");
    }

    void PhysicsDebugRendererImpl::renderPassConfigure(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
        _cam->configureView(viewId);
    }

    void PhysicsDebugRendererImpl::renderPassExecute(RenderGraphResources& res)
    {
        if (!_enabled)
        {
            return;
        }
        auto joltSystem = _system.getJolt();
        if (!joltSystem)
        {
            return;
        }
        if (!_config.material)
        {
            return;
        }

        _encoder = res.get<bgfx::Encoder>();

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

    void PhysicsDebugRendererImpl::renderMesh(MeshData& meshData, EDrawMode mode)
    {
        if (meshData.empty())
        {
            return;
        }
        auto mesh = meshData.createMesh(_vertexLayout);
        meshData.clear();
        renderMesh(*mesh, EDrawMode::Wireframe);
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
        meshData.config.type = MeshType::Transient;
        for (auto& textData : _textData)
        {
            auto textMeshData = Text::createMeshData(textData.content, *_font);
            textMeshData.config.color = textData.color;
            textMeshData.config.offset = textData.position;
            textMeshData.config.scale = glm::vec3(textData.height);

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

    void PhysicsDebugRendererImpl::renderMesh(const IMesh& mesh, EDrawMode mode)
    {
        if (!_config.material || !_encoder)
        {
            return;
        }
        auto& encoder = _encoder.value();
        Material material = *_config.material;
        auto primType = mode == EDrawMode::Wireframe ? MaterialPrimitiveType::Line : MaterialPrimitiveType::Triangle;
        material.setPrimitiveType(primType);

        if (mesh.render(encoder))
        {
            renderSubmit(material);
        }
    }

    void PhysicsDebugRendererImpl::renderSubmit(const Material& mat)
    {
        static const MaterialColorType colorType = MaterialColorType::Diffuse;
        auto color = _config.material->getColor(colorType).value_or(Colors::white());
        color.a *= _config.alpha;
        Material mat2(mat);
        mat2.setColor(colorType, color);
        mat2.renderSubmit(_encoder.value(), _viewId);
    }

    void PhysicsDebugRendererImpl::DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg color)
    {
        MeshData data(Line(JoltUtils::convert(from), JoltUtils::convert(to)));
        data.config.color = JoltUtils::convert(color);
        _wireMeshData += data;
        tryRenderMeshBatch(_wireMeshData, EDrawMode::Wireframe);
    }

    void PhysicsDebugRendererImpl::DrawTriangle(JPH::RVec3Arg v1, JPH::RVec3Arg v2, JPH::RVec3Arg v3, JPH::ColorArg color, ECastShadow castShadow)
    {
        MeshData data(darmok::Triangle(JoltUtils::convert(v1), JoltUtils::convert(v2), JoltUtils::convert(v3)));
        data.config.color = JoltUtils::convert(color);
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

    JPH::DebugRenderer::Batch PhysicsDebugRendererImpl::CreateTriangleBatch(const JPH::DebugRenderer::Triangle* inTriangles, int inTriangleCount)
    {
        MeshData data;
        for (int i = 0; i < inTriangleCount; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                auto vert = inTriangles[i].mV[j];
                data.vertices.emplace_back(
                    JoltUtils::convert(vert.mPosition),
                    JoltUtils::convert(vert.mNormal),
                    JoltUtils::convert(vert.mUV),
                    JoltUtils::convert(vert.mColor)
                );
            }
        }
        return new JoltMeshBatch(data.createMesh(_vertexLayout));
    }

    JPH::DebugRenderer::Batch PhysicsDebugRendererImpl::CreateTriangleBatch(const JPH::DebugRenderer::Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount)
    {
        MeshData data;
        for (int i = 0; i < inVertexCount; i++)
        {
            auto vert = inVertices[i];
            data.vertices.emplace_back(
                JoltUtils::convert(vert.mPosition),
                JoltUtils::convert(vert.mNormal),
                JoltUtils::convert(vert.mUV),
                JoltUtils::convert(vert.mColor)
            );
        }
        for (int i = 0; i < inIndexCount; i++)
        {
            data.indices.emplace_back(inIndices[i]);
        }
        return new JoltMeshBatch(data.createMesh(_vertexLayout));
    }

    void PhysicsDebugRendererImpl::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode, ECastShadow inCastShadow, EDrawMode inDrawMode)
    {
        if (inGeometry->mLODs.empty())
        {
            return;
        }
        // TODO: decide lod
        auto& lod = inGeometry->mLODs[0];
        auto& batch = *(JoltMeshBatch*)lod.mTriangleBatch.GetPtr();

        auto transMtx = JoltUtils::convert(inModelMatrix);
        _encoder->setTransform(glm::value_ptr(transMtx));

        batch.mesh->render(_encoder.value());
        auto oldColor = _config.material->getColor(MaterialColorType::Diffuse);
        _config.material->setColor(MaterialColorType::Diffuse, JoltUtils::convert(inModelColor));
        renderMesh(*batch.mesh, inDrawMode);
        _config.material->setColor(MaterialColorType::Diffuse, oldColor);
    }

    PhysicsDebugRenderer::PhysicsDebugRenderer(PhysicsSystem& system, const Config& config) noexcept
        : _impl(std::make_unique<PhysicsDebugRendererImpl>(system.getImpl(), config))
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

    PhysicsDebugRenderer& PhysicsDebugRenderer::setFont(const std::shared_ptr<IFont>& font) noexcept
    {
        _impl->setFont(font);
        return *this;
    }
}

#endif