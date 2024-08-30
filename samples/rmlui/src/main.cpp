

#include <darmok/app.hpp>
#include <darmok/rmlui.hpp>
#include <darmok/data.hpp>
#include <darmok/window.hpp>
#include <darmok/scene.hpp>
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/math.hpp>
#include <bgfx/bgfx.h>

#include <RmlUi/Core.h>

#ifdef _DEBUG
#define RMLUI_DEBUGGER
#include <darmok/rmlui_debug.hpp>
#endif

namespace
{
	using namespace darmok;

	class WobbleUpdater : public ISceneComponent
	{
	public:
		WobbleUpdater(Transform& trans, float speed = 0.1F, float maxAngle = 10.F)
			: _trans(trans)
			, _speed(speed)
			, _maxAngle(maxAngle)
			, _factor(1.F)
		{
		}

		void update(float dt) override
		{
			auto rot = _trans.getRotation();
			glm::quat target(glm::radians(_factor * _maxAngle * glm::vec3(1, 1, 1)));
			rot = Math::rotateTowards(rot, target, _speed * dt);
			if (rot == target)
			{
				_factor *= -1.F;
			}
			_trans.setRotation(rot);
		}

	private:
		Transform& _trans;
		float _speed;
		float _maxAngle;
		float _factor;
	};

	class RmluiSampleApp : public App
	{
	public:
		void init() override
		{
			App::init();

			auto& scene = *addComponent<SceneAppComponent>().getScene();

			auto camEntity = scene.createEntity();
			scene.addComponent<Transform>(camEntity)
				.setPosition({ 0, 0, -3 })
				.lookAt({ 0, 0, 0 });
			auto& cam = scene.addComponent<Camera>(camEntity)
				.setWindowPerspective(60, 0.3, 10);
			cam.addComponent<ForwardRenderer>();
			cam.addComponent<RmluiComponent>();

			auto centerEntity = scene.createEntity();
			auto& centerTrans = scene.addComponent<Transform>(centerEntity);
			centerTrans.setPosition(glm::vec3(0.5, 0, 0));
			scene.addSceneComponent<WobbleUpdater>(centerTrans);
			centerTrans.setEulerAngles(glm::vec3(0, 0, -20));

			auto canvasEntity = scene.createEntity();
			auto& canvasTrans = scene.addComponent<Transform>(canvasEntity);
			canvasTrans.setParent(centerTrans);

			glm::uvec2 canvasSize(800, 600);
			glm::vec3 scale = glm::vec3(2) / glm::vec3(canvasSize, 1);
			canvasTrans.setPosition(glm::vec3(-1.0F, -1.0F, 0));
			canvasTrans.setScale(scale);
			auto& canvas = scene.addComponent<RmluiCanvas>(canvasEntity, "rmlui", canvasSize);
			canvas.setInputActive(true);
			auto& context = canvas.getContext();

#ifdef RMLUI_DEBUGGER
			scene.addSceneComponent<RmluiDebuggerComponent>();
#endif

			// sample taken from the RmlUI README
			// https://github.com/mikke89/RmlUi

			// Tell RmlUi to load the given fonts.
			Rml::LoadFontFace("LatoLatin-Regular.ttf");
			// Fonts can be registered as fallback fonts, as in this case to display emojis.
			Rml::LoadFontFace("NotoEmoji-Regular.ttf", true);

			// Set up data bindings to synchronize application data.
			if (Rml::DataModelConstructor constructor = context.CreateDataModel("animals"))
			{
				constructor.Bind("show_text", &_show_text);
				constructor.Bind("animal", &_animal);
			}

			// Now we are ready to load our document.
			Rml::ElementDocument* document = context.LoadDocument("hello_world.rml");
			document->Show();

			// Replace and style some text in the loaded document.
			Rml::Element* element = document->GetElementById("world");
			element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
			element->SetProperty("font-size", "1.5em");

			// testing that rmlui works on borderless fullscreen
			/*
			getWindow().requestVideoMode({
				.screenMode = WindowScreenMode::WindowedFullscreen,
				.size = {1024, 768},
			});
			*/
		}
	private:
		bool _show_text = true;
		Rml::String _animal = "dog";
	};
}

DARMOK_RUN_APP(RmluiSampleApp);
