
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

	class SceneTextAppDelegate : public IAppDelegate, public IImguiRenderer
	{
	public:
		SceneTextAppDelegate(App& app)
			: _app(app)
		{
		}

		void init() override
		{
			auto& imgui = _app.addComponent<darmok::ImguiAppComponent>(*this);

			auto& scene = *_app.addComponent<SceneAppComponent>().getScene();

			auto arial = _app.getAssets().getFontLoader()("ARIALUNI.TTF");
			auto noto = _app.getAssets().getFontLoader()("../../assets/noto.ttf");
			auto comic = _app.getAssets().getFontLoader()("COMIC.xml");

			auto camEntity = scene.createEntity();
			auto& camTrans = scene.addComponent<Transform>(camEntity);
			auto& cam = scene.addComponent<Camera>(camEntity);
			/*
				camTrans.setPosition(glm::vec3(0.f, -1.f, -1.f))
				    .lookAt(glm::vec3(0, 0, 0));
				cam.setViewportPerspective(60, 0.3, 1000);
			*/

			cam.setViewportOrtho();

			cam.addComponent<ForwardRenderer>();
			cam.addComponent<TextRenderer>();

			auto text1Entity = scene.createEntity();
			_text1 = scene.addComponent<Text>(text1Entity, arial, _textStr);
			scene.addComponent<Transform>(text1Entity, glm::vec3(0, 200, 0))
				.setScale(glm::vec3(1000));

			auto text2Entity = scene.createEntity();
			_text2 = scene.addComponent<Text>(text2Entity, comic, _textStr);
			scene.addComponent<Transform>(text2Entity, glm::vec3(0, 0, 0))
				.setScale(glm::vec3(1000));

			auto text3Entity = scene.createEntity();
			_text3 = scene.addComponent<Text>(text3Entity, noto, _textStr);
			_text3->setContentSize(glm::vec2(200, noto->getLineSize()));
			scene.addComponent<Transform>(text3Entity, glm::vec3(0, -200, 0))
				.setScale(glm::vec3(1000));
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
		App& _app;
		OptionalRef<Text> _text1;
		OptionalRef<Text> _text2;
		OptionalRef<Text> _text3;
		std::string _textStr = "darmok engine rules!";
		glm::vec4 _color = glm::vec4(1);
	};
}

DARMOK_RUN_APP(SceneTextAppDelegate);
