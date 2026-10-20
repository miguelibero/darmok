#include <darmok/render_forward.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/light.hpp>
#include <darmok/material.hpp>
#include <darmok/scene.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/texture.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/shadow.hpp>
#include "detail/render_samplers.hpp"

namespace darmok
{
	ForwardRenderer::Definition ForwardRenderer::createDefinition() noexcept
	{
		Definition def;
        def.set_pbr(true);
		return def;
	}

	ForwardRenderer::ForwardRenderer(bool pbr, const std::string& name) noexcept
        : _pbr{pbr}
        , _name{name}
	{
	}

	ForwardRenderer::~ForwardRenderer() noexcept = default;

	expected<void, std::string> ForwardRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		_cam = cam;
		_scene = scene;
		_app = app;
        auto result = app.getOrAddComponent<MaterialAppComponent>(Material::RenderConfig::createDefault(_pbr));
		if (!result)
		{
			return unexpected{std::move(result).error()};
		}
		_materials = result.value().get();

		auto progResult = StandardProgramLoader::load(_pbr ? Program::Standard::Forward : Program::Standard::ForwardBasic);
        if(!progResult)
		{
            return unexpected{"failed to load forward program: " + std::move(progResult).error()};
		}
        _prog = std::move(progResult).value();
		return {};
	}

	expected<void, std::string> ForwardRenderer::load(const Definition& def) noexcept
	{
		return {};
	}

	expected<bgfx::ViewId, std::string> ForwardRenderer::renderReset(bgfx::ViewId viewId) noexcept
	{
		_viewId.reset();
		if (!_cam)
		{
			return unexpected<std::string>{"camera not loaded"};
		}
		_cam->configureView(viewId, _name);
		_viewId = viewId;
		return ++viewId;
	}

	expected<void, std::string> ForwardRenderer::shutdown() noexcept
	{
		_cam.reset();
		_scene.reset();
		_app.reset();
		_materials.reset();
		_prog.reset();
		return {};
	}

    expected<void, std::string> ForwardRenderer::render() noexcept
    {
        if(!_viewId)
        {
            return unexpected<std::string>{"view not initialized"};
        }
        auto entitiesView = _cam->getEntities<Renderable>();
        std::vector<Entity> entities;
        // TODO: check why the constructor of the vector is not working with the view
        for(auto entity : entitiesView)
        {
            entities.push_back(entity);
        }
        auto& encoder = *bgfx::begin();
        RenderContext ctx{*_viewId, encoder};
        auto result = render(entities, ctx);
        bgfx::end(&encoder);
        return result;
    }

	expected<void, std::string> ForwardRenderer::render(const std::vector<Entity>& entities, const RenderContext& ctx) noexcept
	{
		if (!_scene)
		{
			return unexpected<std::string>{"scene not loaded"};
		}
		if (!_cam)
		{
			return unexpected<std::string>{"camera not loaded"};
		}
        auto result = _cam->beforeRenderView(ctx);
		if (!result)
		{
			return result;
		}

        ProgramDefines defines;
        auto hasShadows = _cam->getComponent<ShadowRenderer>();

		std::vector<std::string> errors;
		for (auto entity : entities)
		{
			auto renderable = _scene->getComponent<const Renderable>(entity);
			if (!renderable->valid())
			{
				continue;
			}
			if (_cam->shouldEntityBeCulled(entity))
			{
				continue;
			}
            auto result = _cam->beforeRenderEntity(entity, ctx);
			if (!result)
			{
				errors.push_back(std::move(result).error());
				continue;
			}
            auto material = renderable->getMaterial();
            if (material && material->shadingModel != Material::Definition::Unlit)
            {
                auto lightResult = _cam->beforeRenderLight(ctx);
                if(!lightResult)
                {
                    errors.push_back(std::move(lightResult).error());
                    continue;
                }
            }
			if (!renderable->render(ctx.encoder))
			{
				continue;
			}
            ProgramDefines defines;
            if (material)
            {
                _materials->renderBind(ctx.encoder, *material);
                ctx.encoder.setState(material->getRenderState());
                defines = material->programDefines;
            }
            if(hasShadows)
            {
                defines.insert("SHADOW_ENABLED");
            }

            ctx.encoder.submit(ctx.viewId, _prog->getHandle(defines));
		}
		return StringUtils::joinExpectedErrors(errors);
	}
}
