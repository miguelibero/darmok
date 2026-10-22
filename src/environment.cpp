#include <darmok/environment.hpp>
#include <darmok/camera.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/shape.hpp>
#include <darmok/program.hpp>
#include <darmok/program_core.hpp>
#include <darmok/color.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/scene_serialize.hpp>
#include "generated/shaders/core/skybox.h"
#include "generated/shaders/core/grid.h"
#include "detail/render_samplers.hpp"

namespace darmok
{
    SkyboxRenderer::SkyboxRenderer(const std::shared_ptr<Texture>& texture) noexcept
        : _texture{ texture }
    {
    }

    expected<void, std::string> SkyboxRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        auto progResult = Program::loadStaticMem(darmok_program_skybox);
        if (!progResult)
        {
            return unexpected{ std::move(progResult).error() };
        }
        _program = std::make_unique<Program>(std::move(progResult).value());
        _texUniform = { "s_texColor", bgfx::UniformType::Sampler };

        static const Cube screen{ glm::uvec3{ 2 } };
        auto meshResult = MeshData{ screen }.createMesh(_program->getVertexLayout());
        if (!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
		}
        _mesh = std::make_unique<Mesh>(std::move(meshResult).value());
        return {};
    }

    expected<void, std::string> SkyboxRenderer::load(const Definition& def, IComponentLoadContext& context) noexcept
    {
        auto texResult = context.getAssets().getTextureLoader()(def.texture_path());
        if (!texResult)
        {
            return unexpected{ std::move(texResult).error() };
        }
        _texture = texResult.value();
        return {};
    }

    expected<void, std::string> SkyboxRenderer::shutdown() noexcept
    {
        _texUniform.reset();
        _mesh.reset();
        _program.reset();
        _cam.reset();

        return {};
    }

    expected<void, std::string> SkyboxRenderer::afterRender(const RenderContext& ctx) noexcept
    {
        if (!_mesh || !_program || !_texture)
        {
            return unexpected<std::string>{"not initialized"};
        }

        auto renderResult = _mesh->render(ctx.encoder);
        if(!renderResult)
        {
            return unexpected{ std::move(renderResult).error() };
		}

        ctx.encoder.setTexture(RenderSamplers::MATERIAL_ALBEDO, _texUniform, _texture->getHandle());
        if (_cam)
        {
            _cam->setUniforms(ctx);
        }

        static const uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            ;
        ctx.encoder.setState(state);
        ctx.encoder.submit(ctx.viewId, _program->getHandle(_defines));

        return {};
    }

    void SkyboxRenderer::setDefines(const ProgramDefines& defines) noexcept
    {
        _defines = defines;
    }

    SkyboxRenderer::Definition SkyboxRenderer::createDefinition() noexcept
    {
        Definition def;
        return def;
    }

    GridRenderer::GridRenderer(const Definition& def) noexcept
        : _def{ def }
    {
    }

    GridRenderer::Definition GridRenderer::createDefinition() noexcept
    {
        Definition def;
        *def.mutable_x_color() = convert<protobuf::Color>(Colors::red());
        *def.mutable_z_color() = convert<protobuf::Color>(Colors::blue());
        def.set_fade_start(20.0F);
        def.set_fade_end(100.0F);
        def.set_axis_width(0.01F);

        auto& smallGrid = *def.mutable_grid1();
        smallGrid.set_separation(1.0F);
        smallGrid.set_width(0.01F);
        *smallGrid.mutable_color() = convert<protobuf::Color>(Colors::fromNumber(0x505050FF));

        auto& bigGrid = *def.mutable_grid2();
        bigGrid.set_separation(10.0F);
        bigGrid.set_width(0.01F);
        *bigGrid.mutable_color() = convert<protobuf::Color>(Colors::fromNumber(0x707070FF));

        return def;
    }

    expected<void, std::string> GridRenderer::load(const Definition& def) noexcept
    {
        _def = def;
        return {};
    }

    expected<void, std::string> GridRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        auto progResult = Program::loadStaticMem(darmok_program_grid);
        if (!progResult)
        {
            return unexpected{ std::move(progResult).error() };
        }
        _program = std::make_unique<Program>(std::move(progResult).value());

        _majorColorUniform = {"u_majorGridColor", bgfx::UniformType::Vec4};
        _minorColorUniform = {"u_minorGridColor", bgfx::UniformType::Vec4};
        _xColorUniform = { "u_axisXColor", bgfx::UniformType::Vec4 };
        _zColorUniform = {"u_axisZColor", bgfx::UniformType::Vec4};
        _gridParamsUniform = { "u_gridParams", bgfx::UniformType::Vec4 };
        _depthParamsUniform = {"u_depthParams", bgfx::UniformType::Vec4};
        _fadeParamsUniform = {"u_fadeParams", bgfx::UniformType::Vec4};

        static const Rectangle rect{ glm::vec2{2.0f} };
        MeshData meshData{ rect };
        auto meshResult = meshData.createMesh(_program->getVertexLayout());
        if (!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
        }
        _mesh = std::make_unique<Mesh>(std::move(meshResult).value());
        _cam = cam;
        return {};
    }

    expected<void, std::string> GridRenderer::shutdown() noexcept
    {
        _program.reset();
        _mesh.reset();
        _cam.reset();
        _majorColorUniform.reset();
        _minorColorUniform.reset();
        _xColorUniform.reset();
        _zColorUniform.reset();
        _gridParamsUniform.reset();
        _depthParamsUniform.reset();
        _fadeParamsUniform.reset();
        return {};
    }

    expected<void, std::string> GridRenderer::afterRender(const RenderContext& ctx) noexcept
    {
        if (!_mesh || !_program)
        {
            return unexpected<std::string>{"uninitialized"};
        }

        auto result = _mesh->render(ctx.encoder);
        if (!result)
        {
            return result;
        }

        auto majorColor = Colors::normalize(convert<Color>(_def.grid1().color()));
        auto minorColor = Colors::normalize(convert<Color>(_def.grid2().color()));
        auto xColor = Colors::normalize(convert<Color>(_def.x_color()));
        auto zColor = Colors::normalize(convert<Color>(_def.z_color()));
        ctx.encoder.setUniform(_majorColorUniform, glm::value_ptr(majorColor));
        ctx.encoder.setUniform(_minorColorUniform, glm::value_ptr(minorColor));
        ctx.encoder.setUniform(_xColorUniform, glm::value_ptr(xColor));
        ctx.encoder.setUniform(_zColorUniform, glm::value_ptr(zColor));

        glm::vec4 data;

        data.x = _def.grid1().separation();
        data.y = _def.grid2().separation();
        data.z = _def.grid1().width();
        data.w = _def.grid2().width();
        ctx.encoder.setUniform(_gridParamsUniform, glm::value_ptr(data));

        auto& proj = _cam->getProjectionMatrix();
        auto depthRange = Math::projDepthRange(proj);
        data.x = depthRange.x;
        data.y = depthRange.y;
        data.z = 0.01;
        ctx.encoder.setUniform(_depthParamsUniform, glm::value_ptr(data));

        data.x = _def.fade_start();
        data.y = _def.fade_end();
        data.z = _def.axis_width();
        ctx.encoder.setUniform(_fadeParamsUniform, glm::value_ptr(data));

        if(_cam)
        {
            _cam->setUniforms(ctx);
        }

        static const uint64_t state = 0
            | BGFX_STATE_WRITE_RGB
            | BGFX_STATE_WRITE_A
            | BGFX_STATE_WRITE_Z
            | BGFX_STATE_BLEND_ALPHA
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            ;
        ctx.encoder.setState(state);
        ctx.encoder.submit(ctx.viewId, _program->getHandle());
        return {};
    }
}
