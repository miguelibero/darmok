

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

			auto prog = darmok::AssetContext::get().loadProgram("assets/sprite_vertex.sc", "assets/sprite_fragment.sc");

			auto& scene = addComponent<darmok::SceneAppComponent>(prog).getScene();
			auto texData = darmok::AssetContext::get().loadTextureWithInfo("assets/darmok.jpg");
			auto& win = darmok::WindowContext::get().getWindow();
			auto& size = win.getSize();

			auto camEntity = scene.createEntity();
			scene.addComponent<darmok::Camera>(camEntity, glm::ortho<float>(0.f, size.x, 0.f, size.y, -1.f, 1000.f));

			auto spriteEntity = scene.createEntity();
			scene.addComponent<darmok::Transform>(spriteEntity);
			scene.addComponent<darmok::Sprite>(spriteEntity, texData);
		}

		void beforeUpdate(const darmok::InputState& input, bgfx::ViewId viewId, const darmok::WindowHandle& window)
		{
			bgfx::setViewClear(viewId
				, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
				, 0x303030ff
				, 1.0f
				, 0
			);
		}

	};
}

DARMOK_IMPLEMENT_MAIN(ExampleScene);
