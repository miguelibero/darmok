

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>

namespace
{
	class ExampleScene : public darmok::App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto prog = darmok::AssetContext::get().loadProgram("assets/sprite_vertex.sc");

			auto& scene = addComponent<darmok::SceneAppComponent>(prog).getScene();
			auto texture = darmok::AssetContext::get().loadTexture("assets/darmok.jpg");
			auto& win = darmok::WindowContext::get().getWindow();
			auto& size = win.getSize();

			auto camEntity = scene.createEntity();
			scene.addComponent<darmok::Camera>(camEntity, glm::ortho<float>(0.f, size.x, 0.f, size.y));

			auto spriteEntity = scene.createEntity();
			scene.addComponent<darmok::Transform>(spriteEntity, glm::vec3(win.getSize(), 0) / 2.f);
			scene.addComponent<darmok::Sprite>(spriteEntity, texture);
		}
	};
}

DARMOK_IMPLEMENT_MAIN(ExampleScene);
