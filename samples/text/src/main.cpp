
#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/window.hpp>
#include <darmok/texture.hpp>
#include <darmok/text.hpp>
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/imgui.hpp>
#include <darmok/string.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace
{
	using namespace darmok;

	class SceneTextApp : public App, public IImguiRenderer
	{
	public:
		void init() override
		{
			App::init();

			auto& imgui = addComponent<darmok::ImguiAppComponent>(*this);

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			auto& registry = scene.getRegistry();

			auto font = getAssets().getFontLoader()("COMIC.TTF");

			auto camEntity = registry.create();
			registry.emplace<Transform>(camEntity)
				.setPosition(glm::vec3(0.f, 1.f, -1.f))
				.lookAt(glm::vec3(0, 0, 0));

			auto& cam = registry.emplace<Camera>(camEntity)
				.setPerspective(60, getWindow().getSize(), 0.3, 1000);

			cam.addComponent<TextRenderer>();
			cam.setRenderer<ForwardRenderer>();

			auto textEntity = scene.createEntity();

			_text = scene.addComponent<Text>(textEntity, font, _textStr);
		}

		void imguiRender()
		{
			if (ImGui::InputText("Text", &_textStr) && _text)
			{
				// casting because ImGui::InputText does not have an std::u8string method
				_text->setContent(StringUtils::utf8Cast(_textStr));
			}
			if (ImGui::ColorPicker4("Color", &_color[0]))
			{
				_text->setColor(Colors::denormalize(_color));
			}
		}
	private:
		OptionalRef<Text> _text;
		std::string _textStr = "darmok engine rules!";
		glm::vec4 _color = glm::vec4(1);
	};
}

DARMOK_RUN_APP(SceneTextApp);
