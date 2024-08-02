

#include <darmok/app.hpp>
#include <darmok/rmlui.hpp>
#include <darmok/data.hpp>
#include <darmok/window.hpp>
#include <bgfx/bgfx.h>

#include <RmlUi/Core.h>

#ifdef _DEBUG
#define RMLUI_DEBUGGER
#include <darmok/rmlui_debug.hpp>
#endif

namespace
{
	using namespace darmok;

	class RmluiSampleApp : public App
	{
	public:
		void init() override
		{
			App::init();

			auto& comp = addComponent<darmok::RmluiAppComponent>();
			auto& context = comp.getView().getContext();
			comp.getView().setInputActive(true);

#ifdef RMLUI_DEBUGGER
			addComponent<darmok::RmluiDebuggerAppComponent>(comp);
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
			getWindow().requestVideoMode({
				.screenMode = WindowScreenMode::WindowedFullscreen,
				.size = {1024, 768},
			});
		}
	private:
		bool _show_text = true;
		Rml::String _animal = "dog";
	};
}

DARMOK_RUN_APP(RmluiSampleApp);
