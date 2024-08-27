#include <darmok/environment.hpp>
#include <darmok/camera.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/shape.hpp>
#include <darmok/program.hpp>
#include <darmok/program_core.hpp>

#include "generated/skybox.program.h"

namespace darmok
{
    SkyboxRenderComponent::SkyboxRenderComponent(const std::shared_ptr<Texture>& texture) noexcept
        : _texture(texture)
        , _texUniform{ bgfx::kInvalidHandle }
    {
    }

    void SkyboxRenderComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;

        ProgramDefinition progDef;
        progDef.loadStaticMem(skybox_program);
        _program = std::make_unique<Program>(progDef);

        _texUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

        Cube screen(glm::uvec3(2));
        _mesh = MeshData(screen).createMesh(_program->getVertexLayout());
    }

    void SkyboxRenderComponent::shutdown() noexcept
    {
        if (isValid(_texUniform))
        {
            bgfx::destroy(_texUniform);
            _texUniform.idx = bgfx::kInvalidHandle;
        }
        _mesh.reset();
        _program.reset();
        _cam.reset();
    }

    void SkyboxRenderComponent::beforeRenderView(IRenderGraphContext& context)
    {
        auto& encoder = context.getEncoder();
        auto viewId = context.getViewId();

        if (!_mesh || !_program || !_texture)
        {
            return;
        }

        _mesh->render(encoder);

        encoder.setTexture(0, _texUniform, _texture->getHandle());

        uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            ;
        encoder.setState(state);
        encoder.submit(viewId, _program->getHandle());
    }
}