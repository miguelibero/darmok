#include <darmok/environment.hpp>
#include <darmok/camera.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/shape.hpp>
#include <darmok/program.hpp>
#include <darmok/program_core.hpp>
#include <darmok/color.hpp>

#include "generated/skybox.program.h"

namespace darmok
{
    SkyboxRenderer::SkyboxRenderer(const std::shared_ptr<Texture>& texture) noexcept
        : _texture(texture)
        , _texUniform{ bgfx::kInvalidHandle }
    {
    }

    void SkyboxRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;

        ProgramDefinition progDef;
        progDef.loadStaticMem(skybox_program);
        _program = std::make_unique<Program>(progDef);

        _texUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

        Cube screen(glm::uvec3(2));
        _mesh = MeshData(screen).createMesh(_program->getVertexLayout());
    }

    void SkyboxRenderer::shutdown() noexcept
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

    void SkyboxRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder)
    {
        if (!_mesh || !_program || !_texture)
        {
            return;
        }

        _mesh->render(encoder);

        encoder.setTexture(0, _texUniform, _texture->getHandle());

        uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            ;
        encoder.setState(state);
        encoder.submit(viewId, _program->getHandle());
    }

    const GridRenderer::Config GridRenderer::defaultConfig = {
        GridConfig{ glm::vec2(0.1F), Colors::fromNumber(0x808080FF) },
        GridConfig{ glm::vec2(1.F), Colors::fromNumber(0xBEBEBEFF) },
    };

    GridRenderer::GridRenderer(const Config& config) noexcept
        : _config(config)
    {
    }

    void GridRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _debugRender.init(app);
        updateMesh();
    }

    void GridRenderer::updateMesh() noexcept
    {
        auto prog = _debugRender.getProgram();
        if (!prog)
        {
            return;
        }
        MeshData meshData;
        for (auto& config : _config)
        {
            Grid grid;
            grid.separation = config.separation;
            // TODO: calculate amount & origin based on camera
            MeshData gridData(grid);
            gridData.setColor(config.color);
            meshData += gridData;
        }
        auto& layout = prog->getVertexLayout();
        _mesh = meshData.createMesh(layout);
    }

    void GridRenderer::shutdown() noexcept
    {
        _cam.reset();
        _mesh.reset();
        _debugRender.shutdown();
    }

    void GridRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder)
    {
        if (!_mesh)
        {
            return;
        }
        _debugRender.renderMesh(*_mesh, viewId, encoder);
    }

    void GridRenderer::onCameraTransformChanged()
    {
        updateMesh();
    }
}