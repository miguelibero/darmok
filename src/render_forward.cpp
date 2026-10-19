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
#include "detail/render_samplers.hpp"

namespace darmok
{
	ForwardRenderer::Definition ForwardRenderer::createDefinition() noexcept
	{
		Definition def;
		return def;
	}

	ForwardRenderer::ForwardRenderer(bool pbr) noexcept
        : _pbr{pbr}
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

		auto litResult = StandardProgramLoader::load(_pbr ? Program::Standard::Forward : Program::Standard::ForwardBasic);
		if (!litResult)
		{
			return unexpected{ "failed to load forward program: " + std::move(litResult).error() };
		}
		_litProg = std::move(litResult).value();
		auto unlitResult = StandardProgramLoader::load(Program::Standard::Unlit);
		if (!unlitResult)
		{
			return unexpected{ "failed to load unlit program: " + std::move(unlitResult).error() };
		}
		_unlitProg = std::move(unlitResult).value();
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
		_cam->configureView(viewId, "Forward");
		_viewId = viewId;
		return ++viewId;
	}

	expected<void, std::string> ForwardRenderer::shutdown() noexcept
	{
		_cam.reset();
		_scene.reset();
		_app.reset();
		_materials.reset();
		_litProg.reset();
		_unlitProg.reset();
		return {};
	}

	expected<void, std::string> ForwardRenderer::render() noexcept
	{
		if (!_viewId)
		{
			return {};
		}
		if (!_scene)
		{
			return unexpected<std::string>{"scene not loaded"};
		}
		if (!_cam)
		{
			return unexpected<std::string>{"camera not loaded"};
		}
		if (!_cam->isEnabled())
		{
			return {};
		}
		auto viewId = _viewId.value();
		auto& encoder = *bgfx::begin();
		auto result = _cam->beforeRenderView(viewId, encoder);
		if (!result)
		{
			return result;
		}
		std::vector<std::string> errors;
		auto entities = _cam->getEntities<Renderable>();
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
			auto result = _cam->beforeRenderEntity(entity, viewId, encoder);
			if (!result)
			{
				errors.push_back(std::move(result).error());
				continue;
			}
            auto lightResult = _cam->beforeRenderLight(viewId, encoder);
            if(!lightResult)
            {
                errors.push_back(std::move(lightResult).error());
                continue;
            }
			if (!renderable->render(encoder))
			{
				continue;
			}
			auto& material = *renderable->getMaterial();
			auto& prog = material.shadingModel == Material::Definition::Unlit ? _unlitProg : _litProg;
			_materials->renderSubmit(viewId, encoder, *prog, material);
		}
		bgfx::end(&encoder);
		return StringUtils::joinExpectedErrors(errors);
	}
}
