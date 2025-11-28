

#include <darmok/app.hpp>
#include <darmok/rmlui.hpp>
#include <darmok/data.hpp>
#include <darmok/window.hpp>
#include <darmok/scene.hpp>
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/math.hpp>
#include <darmok/transform.hpp>
#include <bgfx/bgfx.h>
#include <glm/gtx/component_wise.hpp>
#include <RmlUi/Core.h>

#ifdef _DEBUG
#define RMLUI_DEBUGGER
#include <darmok/rmlui_debug.hpp>
#endif

namespace
{
	using namespace darmok;

	class WobbleUpdater final : public ISceneComponent
	{
	public:
		WobbleUpdater(Transform& trans, float speed = 0.1f, float maxAngle = 10.f)
			: _trans{ trans }
			, _speed{ speed }
			, _maxAngle{ maxAngle }
			, _factor{ 1.f }
		{
		}

		expected<void, std::string> update(float dt) noexcept override
		{
			auto rot = _trans.getRotation();
			glm::quat target{ glm::radians(_factor * _maxAngle * glm::vec3{ 1.f }) };
			rot = Math::rotateTowards(rot, target, _speed * dt);
			if (rot == target)
			{
				_factor *= -1.f;
			}
			_trans.setRotation(rot);
			return {};
		}

	private:
		Transform& _trans;
		float _speed;
		float _maxAngle;
		float _factor;
	};

	class RmluiSampleAppDelegate final : public IAppDelegate
	{
	public:
		RmluiSampleAppDelegate(App& app) noexcept
			: _app{ app }
		{
		}

		expected<void, std::string> init() noexcept override
		{
			auto& scene = *_app.tryAddComponent<SceneAppComponent>()->getScene();
			scene.addSceneComponent<RmluiSceneComponent>();

			// scene.setViewport(Viewport(glm::ivec2(500), glm::ivec2(200)));

			auto camEntity = scene.createEntity();
			auto& cam = scene.addComponent<Camera>(camEntity);

			scene.addComponent<Transform>(camEntity)
				.setPosition({ 0.f, 0.f, -3.f })
				.lookAt({ 0.f, 0.f, 0.f });
			cam.setPerspective(glm::radians(60.f), 0.3f, 5.f);
			
			cam.addComponent<ForwardRenderer>();
			cam.addComponent<RmluiRenderer>();

			glm::uvec2 canvasSize{ 800, 600 };

			auto canvasEntity = scene.createEntity();

			auto& canvasTrans = scene.addComponent<Transform>(canvasEntity);
			canvasTrans.setScale(glm::vec3{ 2.f } / static_cast<float>(glm::compMax(canvasSize)));
			canvasTrans.setEulerAngles(glm::radians(glm::vec3{ 0.f, 0.f, 30.f }));
			scene.addSceneComponent<WobbleUpdater>(canvasTrans);

			auto& canvas = scene.addComponent<RmluiCanvas>(canvasEntity, "rmlui", canvasSize);
			canvas.setOffset(glm::vec3{ canvasSize, 0.f } *-0.5f);
			canvas.setInputEnabled(true);
			canvas.setMousePositionMode(RmluiCanvasMousePositionMode::Absolute);
			auto& context = canvas.getContext();

#ifdef RMLUI_DEBUGGER
			_app.addComponent<RmluiDebuggerComponent>();
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
				constructor.Bind("show_text", &_showText);
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
			// _app.getWindow().requestVideoMode({
			// 	.screenMode = WindowScreenMode::WindowedFullscreen,
			// });

			return {};
		}
	private:
		App& _app;
		bool _showText = true;
		Rml::String _animal = "dog";
	};
}

DARMOK_RUN_APP(RmluiSampleAppDelegate);
