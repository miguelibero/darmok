
#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/window.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
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

			auto arial = getAssets().getFontLoader()("ARIALUNI.TTF");
			auto noto = getAssets().getFontLoader()("noto.ttf");
			auto comic = getAssets().getFontLoader()("COMIC.xml");

			auto camEntity = registry.create();
			auto& camTrans = registry.emplace<Transform>(camEntity);
			auto& cam = registry.emplace<Camera>(camEntity);
			/*
				camTrans.setPosition(glm::vec3(0.f, -1.f, -1.f))
				.lookAt(glm::vec3(0, 0, 0));
				cam.setPerspective(60, getWindow().getSize(), 0.3, 1000);
			*/

			cam.setOrtho(Viewport(glm::uvec2(2)));

			cam.addRenderer<ForwardRenderer>();
			cam.addRenderer<TextRenderer>();


			auto text1Entity = scene.createEntity();
			_text1 = scene.addComponent<Text>(text1Entity, arial, _textStr);
			scene.addComponent<Transform>(text1Entity, glm::vec3(0, 0.5, 0))
				.setScale(glm::vec3(2));

			auto text2Entity = scene.createEntity();
			_text2 = scene.addComponent<Text>(text2Entity, comic, _textStr);
			scene.addComponent<Transform>(text2Entity, glm::vec3(0, 0, 0))
				.setScale(glm::vec3(2));

			auto text3Entity = scene.createEntity();
			_text3 = scene.addComponent<Text>(text3Entity, noto, _textStr);
			_text3->setContentSize(glm::vec2(200, noto->getLineSize()));
			scene.addComponent<Transform>(text3Entity, glm::vec3(0, -0.5, 0))
				.setScale(glm::vec3(2));
		}

		void imguiRender()
		{
			if (ImGui::InputText("Text", &_textStr))
			{
				// casting because ImGui::InputText does not have an std::u8string method
				auto u8str = StringUtils::utf8Cast(_textStr);
				_text1->setContent(u8str);
				_text2->setContent(u8str);
				_text3->setContent(u8str);
			}
			if (ImGui::ColorPicker4("Color", &_color[0]))
			{
				auto c = Colors::denormalize(_color);
				_text1->setColor(c);
				_text2->setColor(c);
				_text3->setColor(c);
			}
		}
	private:
		OptionalRef<Text> _text1;
		OptionalRef<Text> _text2;
		OptionalRef<Text> _text3;
		std::string _textStr = "darmok engine rules!";
		glm::vec4 _color = glm::vec4(1);
	};
}

DARMOK_RUN_APP(SceneTextApp);
