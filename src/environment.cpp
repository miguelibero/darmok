#include <darmok/environment.hpp>
#include <darmok/camera.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/shape.hpp>
#include <darmok/program.hpp>
#include <darmok/program_core.hpp>
#include <darmok/color.hpp>

#include "generated/skybox.program.h"
#include "generated/grid.program.h"

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

        static const uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            ;
        encoder.setState(state);
        encoder.submit(viewId, _program->getHandle());
    }

    GridRenderer::GridRenderer(const Config& config) noexcept
        : _config(config)
        , _color1Uniform{ bgfx::kInvalidHandle }
        , _color2Uniform{ bgfx::kInvalidHandle }
        , _dataUniform{ bgfx::kInvalidHandle }
    {
    }

    void GridRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        ProgramDefinition progDef;
        progDef.loadStaticMem(grid_program);
        _program = std::make_unique<Program>(progDef);

        _color1Uniform = bgfx::createUniform("u_gridColor1", bgfx::UniformType::Vec4);
        _color2Uniform = bgfx::createUniform("u_gridColor2", bgfx::UniformType::Vec4);
        _dataUniform = bgfx::createUniform("u_data", bgfx::UniformType::Vec4);

        static const Rectangle rect(glm::vec2(2.0));
        _mesh = MeshData(rect, RectangleMeshType::Full).createMesh(_program->getVertexLayout());
        _cam = cam;
    }

    void GridRenderer::shutdown() noexcept
    {
        _program.reset();
        _mesh.reset();
        _cam.reset();

        std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms
        {
            _color1Uniform, _color2Uniform, _dataUniform
        };
        for (auto uniform : uniforms)
        {
            if (isValid(uniform.get()))
            {
                bgfx::destroy(uniform.get());
                uniform.get().idx = bgfx::kInvalidHandle;
            }
        }
    }

    void GridRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder)
    {
        if (!_mesh || !_program)
        {
            return;
        }

        _mesh->render(encoder);
        auto& proj = _cam->getProjectionMatrix();
        auto depthRange = Math::projDepthRange(proj);
        glm::vec4 data(depthRange, _config.grids[0].separation, _config.grids[1].separation);

        encoder.setUniform(_color1Uniform, glm::value_ptr(Colors::normalize(_config.grids[0].color)));
        encoder.setUniform(_color2Uniform, glm::value_ptr(Colors::normalize(_config.grids[1].color)));
        encoder.setUniform(_dataUniform, glm::value_ptr(data));

        static const uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_BLEND_ALPHA
            | BGFX_STATE_DEPTH_TEST_LESS
            ;
        encoder.setState(state);
        encoder.submit(viewId, _program->getHandle());
    }
}