#include <darmok/app.hpp>
#include <darmok/cegui.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <CEGUI/CEGUI.h>
#include <iostream>

namespace
{
	using namespace darmok;

	// https://github.com/cegui/cegui/blob/master/samples/HelloWorld/HelloWorld.cpp

	class CeguiApp : public App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);
			_cegui = addComponent<CeguiAppComponent>();
			_cegui->setResourceGroupDirectory("schemes", "cegui/schemes/");
			_cegui->setResourceGroupDirectory("imagesets", "cegui/imagesets/");
			_cegui->setResourceGroupDirectory("fonts", "cegui/fonts/");
			_cegui->setResourceGroupDirectory("layouts", "cegui/layouts/");
			_cegui->setResourceGroupDirectory("looknfeels", "cegui/looknfeel/");
			_cegui->setResourceGroupDirectory("lua_scripts", "cegui/lua_scripts/");

			CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
			CEGUI::Font::setDefaultResourceGroup("fonts");
			CEGUI::Scheme::setDefaultResourceGroup("schemes");
			CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
			CEGUI::WindowManager::setDefaultResourceGroup("layouts");
			CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");

			auto guiContext = _cegui->getGuiContext();

			CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
			getWindow().requestCursorMode(WindowCursorMode::Hidden);
			guiContext->setDefaultCursorImage("TaharezLook/MouseArrow");
			auto& winMgr = CEGUI::WindowManager::getSingleton();

			_win = static_cast<CEGUI::DefaultWindow*>(winMgr.createWindow("DefaultWindow", "Root"));
			auto loadedFonts = CEGUI::FontManager::getSingleton().createFromFile("DejaVuSans-12.font");
			auto defaultFont = loadedFonts.empty() ? 0 : loadedFonts.front();
			
			guiContext->setDefaultFont(defaultFont);
			guiContext->setRootWindow(_win.ptr());

			auto wnd = static_cast<CEGUI::FrameWindow*>(winMgr.createWindow("TaharezLook/FrameWindow", "Sample Window"));

			_win->addChild(wnd);

			wnd->setArea(CEGUI::UVector2(cegui_reldim(0.25f), cegui_reldim(0.25f)), CEGUI::USize(cegui_reldim(0.5f), cegui_reldim(0.5f)));

			wnd->setMaxSize(CEGUI::USize(cegui_reldim(1.0f), cegui_reldim(1.0f)));
			wnd->setMinSize(CEGUI::USize(cegui_reldim(0.1f), cegui_reldim(0.1f)));

			wnd->setText("Hello World!");

			wnd->subscribeEvent(CEGUI::Window::EventClick, CEGUI::Event::Subscriber(&CeguiApp::handleHelloWorldClicked, this));
		}

	private:
		OptionalRef<CeguiAppComponent> _cegui;
		OptionalRef<CEGUI::DefaultWindow> _win;

		bool handleHelloWorldClicked(const CEGUI::EventArgs&)
		{
			std::cout << "Hello World!" << std::endl;
			return false;
		}
	};
}

DARMOK_MAIN(CeguiApp);
