
#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/window.hpp>
#include <darmok/texture.hpp>
#include <darmok/text.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/program_standard.hpp>
#include <darmok/material.hpp>
#include <darmok/render_forward.hpp>

namespace
{
	using namespace darmok;

	class SceneTextApp : public App
	{
	public:
		void init() override
		{
			App::init();

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			auto& registry = scene.getRegistry();

			auto prog = getAssets().getStandardProgramLoader()(StandardProgramType::Unlit);

			auto tex = getAssets().getColorTextureLoader()(Colors::white());
			auto mat = std::make_shared<Material>(prog, tex);

			auto font = getAssets().getFontLoader()("COMIC.TTF");

			auto camEntity = registry.create();
			registry.emplace<Transform>(camEntity)
				.setPosition(glm::vec3(0.f, 2.f, -2.f))
				.lookAt(glm::vec3(0, 0, 0));

			auto& cam = registry.emplace<Camera>(camEntity)
				.setPerspective(60, getWindow().getSize(), 0.3, 1000);

			cam.addComponent<TextRenderer>();
			cam.setRenderer<ForwardRenderer>();

			auto textEntity = scene.createEntity();

			scene.addComponent<Text>(textEntity, font, "comic sans baby!");
		}
	};
}

DARMOK_RUN_APP(SceneTextApp);
