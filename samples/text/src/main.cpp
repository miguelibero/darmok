
#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/window.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/text.hpp>
#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/imgui.hpp>
#include <darmok/string.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <utf8cpp/utf8.h>

namespace
{
	using namespace darmok;

	class SceneTextAppDelegate : public IAppDelegate, public IImguiRenderer
	{
	public:
		SceneTextAppDelegate(App& app)
			: _app{ app }
		{
		}

		void init() override
		{
			auto& imgui = _app.addComponent<darmok::ImguiAppComponent>(*this);

			auto& scene = *_app.addComponent<SceneAppComponent>().getScene();

			auto arial = _app.getAssets().getFontLoader()("arialuni.ttf").value();
			auto noto = _app.getAssets().getFontLoader()("../../assets/noto.ttf").value();
			auto comic = _app.getAssets().getFontLoader()("comic.xml").value();

			auto camEntity = scene.createEntity();
			auto& camTrans = scene.addComponent<Transform>(camEntity);
			auto& cam = scene.addComponent<Camera>(camEntity);
			/*
				camTrans.setPosition({ 0.f, -1.f, -1.f {)
				    .lookAt({ 0, 0, 0 });
				cam.setViewportPerspective(60, 0.3, 1000);
			*/

			cam.setOrtho();

			cam.addComponent<ForwardRenderer>();
			cam.addComponent<TextRenderer>();

			auto text1Entity = scene.createEntity();
			_text1 = scene.addComponent<Text>(text1Entity, arial, _textStr);
			scene.addComponent<Transform>(text1Entity, glm::vec3{ 0, 200, 0 })
				.setScale(glm::vec3{ 1000 });

			auto text2Entity = scene.createEntity();
			_text2 = scene.addComponent<Text>(text2Entity, comic, _textStr);
			scene.addComponent<Transform>(text2Entity, glm::vec3{ 0, 0, 0 })
				.setScale(glm::vec3{ 1000 });

			auto text3Entity = scene.createEntity();
			_text3 = scene.addComponent<Text>(text3Entity, noto, _textStr);
			_text3->setContentSize(glm::vec2{ 200, noto->getLineSize() });
			scene.addComponent<Transform>(text3Entity, glm::vec3(0, -200, 0))
				.setScale(glm::vec3{ 1000 });
		}

		void imguiRender()
		{
			if (ImGui::InputText("Text", &_textStr))
			{
				_text1->setContent(_textStr);
				_text2->setContent(_textStr);
				_text3->setContent(_textStr);
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
