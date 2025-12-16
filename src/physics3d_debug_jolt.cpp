
#include <darmok/physics3d_debug.hpp>
#include "detail/physics3d_debug_jolt.hpp"

#ifdef JPH_DEBUG_RENDERER

#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/input.hpp>
#include <darmok/texture.hpp>
#include <darmok/vertex.hpp>
#include <darmok/text.hpp>
#include <darmok/string.hpp>
#include <darmok/stream.hpp>
#include <darmok/scene_serialize.hpp>
#include "detail/physics3d_jolt.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/PhysicsSystem.h>

namespace darmok::physics3d
{
    JoltMeshBatch::JoltMeshBatch(std::unique_ptr<Mesh>&& mesh) noexcept
        : mesh{ std::move(mesh) }
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
        : _colorUniform{ "u_baseColorFactor", bgfx::UniformType::Vec4 }
        , _solidMeshData{Mesh::Definition::Transient}
        , _wireMeshData{Mesh::Definition::Transient}
    {
        if (auto result = StandardProgramLoader::load(Program::Standard::Unlit))
        {
            _program = result.value();
        }
        Initialize();
    }

    void JoltPhysicsDebugRenderer::shutdown() noexcept
    {
        const std::lock_guard lock{ _instanceLock };
        _instance.reset();
    }

    std::unique_ptr<JoltPhysicsDebugRenderer> JoltPhysicsDebugRenderer::_instance;
    std::mutex JoltPhysicsDebugRenderer::_instanceLock;


    void JoltPhysicsDebugRenderer::init(const Definition& def, std::shared_ptr<IFont> font) noexcept
    {
        const std::lock_guard lock{ _instanceLock };
        _instance = std::unique_ptr<JoltPhysicsDebugRenderer>(new JoltPhysicsDebugRenderer());
        _instance->_def = def;
		_instance->_font = font;
    }

    expected<void, std::string> JoltPhysicsDebugRenderer::render(JPH::PhysicsSystem& joltSystem, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        const std::lock_guard lock{ _instanceLock };
        if (!_instance)
        {
			return unexpected<std::string>{ "JoltPhysicsDebugRenderer not initialized" };
        }
        return _instance->doRender(joltSystem, viewId, encoder);
    }

    expected<void, std::string> JoltPhysicsDebugRenderer::doRender(JPH::PhysicsSystem& joltSystem, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        _encoder = encoder;
        _viewId = viewId;

        JPH::BodyManager::DrawSettings settings;
        settings.mDrawShape = true;
        settings.mDrawShapeWireframe = true;
        settings.mDrawWorldTransform = true;
        // settings.mDrawSleepStats = true;

        joltSystem.DrawBodies(settings, this, nullptr);

        std::vector<std::string> errors;

        auto result = renderMesh(_wireMeshData, EDrawMode::Wireframe);
        if(!result)
        {
            errors.push_back(std::move(result).error());
		}
        result = renderMesh(_solidMeshData, EDrawMode::Solid);
        if (!result)
        {
            errors.push_back(std::move(result).error());
        }
        result = renderText();
        if (!result)
        {
            errors.push_back(std::move(result).error());
        }

        NextFrame();

        return StringUtils::joinExpectedErrors(errors);
    }

    expected<void, std::string> JoltPhysicsDebugRenderer::renderMeshBatch(MeshData& meshData, EDrawMode mode) noexcept
    {
        if (meshData.vertices.size() < _def.mesh_batch_size())
        {
            return {};
        }
        return renderMesh(meshData, mode);
    }

    expected<void, std::string> JoltPhysicsDebugRenderer::renderMesh(MeshData& meshData, EDrawMode mode, const Color& color) noexcept
    {
        if (meshData.empty())
        {
            return {};
        }

		auto meshResult = meshData.createMesh(_program->getVertexLayout());
        if(!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
		}

        meshData.clear();
        return renderMesh(meshResult.value(), mode, color);
    }

    expected<void, std::string> JoltPhysicsDebugRenderer::renderText() noexcept
    {
        if (!_font)
        {
            return {};
        }
        std::unordered_set<char32_t> chars;
        for (auto& textData : _textData)
        {
            chars.insert(textData.content.begin(), textData.content.end());
        }
        auto updateResult = _font->update(chars);
        if (!updateResult)
        {
            return updateResult;
		}

        MeshData meshData;
        meshData.type = Mesh::Definition::Transient;
        for (auto& textData : _textData)
        {
            auto textMeshData = Text::createMeshData(textData.content, *_font);
            textMeshData *= textData.color;
            textMeshData *= glm::translate(glm::mat4(textData.height), textData.position);

            meshData += textMeshData;
            auto error = renderMeshBatch(meshData, EDrawMode::Solid);
            if (!error)
            {
                return error;
            }
        }
        _textData.clear();
        return renderMesh(meshData, EDrawMode::Solid);
    }

    bool PhysicsDebugRendererImpl::isEnabled() const noexcept
    {
        return _enabled;
    }

    void PhysicsDebugRendererImpl::setEnabled(bool enabled) noexcept
    {
        _enabled = enabled;
    }

    expected<void, std::string> JoltPhysicsDebugRenderer::renderMesh(const Mesh& mesh, EDrawMode mode, const Color& color) noexcept
    {
        if (!_encoder)
        {
            return {};
        }
        auto& encoder = _encoder.value();
        if (mesh.render(encoder))
        {
            return renderSubmit(mode, color);
        }
        return {};
    }

    expected<void, std::string> JoltPhysicsDebugRenderer::renderSubmit(EDrawMode mode, const Color& color) noexcept
    {
        if (!_encoder)
        {
            return unexpected<std::string>{"missing encoder"};
        }
        if (!_viewId)
        {
            return unexpected<std::string>{"missing view id"};
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
        return {};
    }

    void JoltPhysicsDebugRenderer::DrawLine(JPH::RVec3Arg from, JPH::RVec3Arg to, JPH::ColorArg color) noexcept
    {
        MeshData data(Line(JoltUtils::convert(from), JoltUtils::convert(to)));
        data *= JoltUtils::convert(color);
        _wireMeshData += data;
        auto result = renderMeshBatch(_wireMeshData, EDrawMode::Wireframe);
        if (!result)
        {
			onError("DrawLine", result.error());
        }
    }

    void JoltPhysicsDebugRenderer::onError(std::string_view prefix, std::string_view message) noexcept
    {
        StreamUtils::log(fmt::format("JoltPhysicsDebugRenderer::{}: {}", prefix, message), true);
	}

    void JoltPhysicsDebugRenderer::DrawTriangle(JPH::RVec3Arg v1, JPH::RVec3Arg v2, JPH::RVec3Arg v3, JPH::ColorArg color, ECastShadow castShadow) noexcept
    {
        MeshData data(darmok::Triangle(JoltUtils::convert(v1), JoltUtils::convert(v2), JoltUtils::convert(v3)));
        data *= JoltUtils::convert(color);
        expected<void, std::string> result;
        if (castShadow == ECastShadow::On)
        {
            _solidMeshData += data;
            result = renderMeshBatch(_solidMeshData, EDrawMode::Solid);
        }
        else
        {
            _wireMeshData += data;
            result = renderMeshBatch(_wireMeshData, EDrawMode::Wireframe);
        }
        if (!result)
        {
            onError("DrawTriangle", result.error());
        }
    }

    void JoltPhysicsDebugRenderer::DrawText3D(JPH::RVec3Arg pos, const std::string_view& str, JPH::ColorArg color, float height) noexcept
    {
		auto content = StringUtils::toUtf32(str);
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

    JPH::DebugRenderer::Batch JoltPhysicsDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Triangle* triangles, int triangleCount) noexcept
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
        auto meshResult = data.createMesh(_program->getVertexLayout());
        if (!meshResult)
        {
            return nullptr;
        }

        return new JoltMeshBatch{ std::make_unique<Mesh>(std::move(meshResult).value()) };
    }

    JPH::DebugRenderer::Batch JoltPhysicsDebugRenderer::CreateTriangleBatch(const JPH::DebugRenderer::Vertex* vertices, int vertexCount, const JPH::uint32* indices, int indexCount) noexcept
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
        auto meshResult = data.createMesh(_program->getVertexLayout());
        if (!meshResult)
        {
            return nullptr;
        }

        return new JoltMeshBatch{ std::make_unique<Mesh>(std::move(meshResult).value()) };
    }

    void JoltPhysicsDebugRenderer::DrawGeometry(JPH::RMat44Arg modelMatrix, const JPH::AABox& worldSpaceBounds, float inLODScaleSq, JPH::ColorArg modelColor, const GeometryRef& geometry, ECullMode cullMode, ECastShadow castShadow, EDrawMode drawMode) noexcept
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

        auto result = batch.mesh->render(_encoder.value());
        static constexpr std::string_view logPrefix = "DrawGeometry";
        if (!result)
        {
            onError(logPrefix, result.error());
        }
        result = renderMesh(*batch.mesh, drawMode, JoltUtils::convert(modelColor));
        if (!result)
        {
            onError(logPrefix, result.error());
        }
    }

    PhysicsDebugRendererImpl::PhysicsDebugRendererImpl(const Definition& def) noexcept
        : _def{ def }
        , _enabled{false}
    {
    }

    expected<void, std::string> PhysicsDebugRendererImpl::load(const Definition& def, IComponentLoadContext& context) noexcept
    {
		auto result = shutdown();
        if (!result)
        {
            return result;
        }
		auto& scene = context.getScene();
        if(!_cam)
        { 
			for (auto entity : scene.getComponents<Camera>())
            {
                _cam = scene.getComponent<Camera>(entity);
                break;
            }
        }
        if (!_cam)
        {
            return unexpected<std::string>{"missing camera for physics debug renderer"};
		}
		
        _def = def;
		return init(*_cam, scene, context.getApp());
    }


    expected<void, std::string> PhysicsDebugRendererImpl::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        _input = app.getInput();
        _input->addListener("enable", _def.enable_events(), *this);

        auto fontResult = app.getAssets().getFontLoader()(_def.font_path());
        if (!fontResult)
        {
            return unexpected<std::string>{ fmt::format("failed to load font '{}': {}", _def.font_path(), fontResult.error()) };
		}
        JoltPhysicsDebugRenderer::init(_def, fontResult.value());
        return {};
    }

    expected<void, std::string> PhysicsDebugRendererImpl::shutdown() noexcept
    {
        if (_input)
        {
            _input->removeListener(*this);
        }
        _cam.reset();
        _scene.reset();
        _input.reset();
        JoltPhysicsDebugRenderer::shutdown();
        return {};
    }

    expected<void, std::string> PhysicsDebugRendererImpl::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        if (!_enabled)
        {
            return {};
        }
        auto system = _scene->getSceneComponent<PhysicsSystem>();
        if (!system)
        {
            return unexpected<std::string>{"missing physics system scene component"};
        }
        auto joltSystem = system->getImpl().getJolt();
        if (!joltSystem)
        {
            return unexpected<std::string>{"uninitialized jolt system"};
        }

        return JoltPhysicsDebugRenderer::render(joltSystem.value(), viewId, encoder);
    }

    expected<void, std::string> PhysicsDebugRendererImpl::onInputEvent(const std::string& tag) noexcept
    {
        setEnabled(!_enabled);
        return {};
    }

    PhysicsDebugRenderer::PhysicsDebugRenderer(const Definition& def) noexcept
        : _impl{ std::make_unique<PhysicsDebugRendererImpl>(def) }
    {
    }

    PhysicsDebugRenderer::~PhysicsDebugRenderer() noexcept = default;

    PhysicsDebugRenderer::Definition PhysicsDebugRenderer::createDefinition() noexcept
    {
        Definition def;
        def.mutable_enable_events()->Add(Keyboard::createInputEvent(Keyboard::Definition::F8));
        def.set_mesh_batch_size(32 * 1024);
        def.set_alpha(0.3F);
        return def;
    }

    expected<void, std::string> PhysicsDebugRenderer::load(const Definition& def, IComponentLoadContext& context) noexcept
    {
		return _impl->load(def, context);
    }

    expected<void, std::string> PhysicsDebugRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        return _impl->init(cam, scene, app);
    }

    expected<void, std::string> PhysicsDebugRenderer::shutdown() noexcept
    {
        return _impl->shutdown();
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

    expected<void, std::string> PhysicsDebugRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        return _impl->beforeRenderView(viewId, encoder);
    }
}

#else

namespace darmok::physics3d
{
    PhysicsDebugRenderer::PhysicsDebugRenderer(const Definition& def) noexcept
        : _impl{ std::make_unique<PhysicsDebugRendererImpl>() }
    {
    }

    PhysicsDebugRenderer::~PhysicsDebugRenderer() noexcept = default;

    expected<void, std::string> PhysicsDebugRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        return {};
    }

    expected<void, std::string> PhysicsDebugRenderer::shutdown() noexcept
    {
        return {};
    }

    bool PhysicsDebugRenderer::isEnabled() const noexcept
    {
        return false;
    }

    PhysicsDebugRenderer& PhysicsDebugRenderer::setEnabled(bool enabled) noexcept
    {
        return *this;
    }

    expected<void, std::string> PhysicsDebugRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        return {};
    }
    
    expected<void, std::string> PhysicsDebugRenderer::load(const Definition& def) noexcept
    {
        return {};
    }
}

#endif