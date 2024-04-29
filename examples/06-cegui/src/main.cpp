#include <darmok/app.hpp>
#include <darmok/cegui.hpp>
#include <darmok/optional_ref.hpp>
#include <CEGUI/CEGUI.h>

namespace
{
	using namespace darmok;

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

			CEGUI::SchemeManager::getSingleton().createFromFile("GameMenuSample.scheme");
			CEGUI::SchemeManager::getSingleton().createFromFile("Generic.scheme");			

			auto ceguiCtxt = _cegui->getGuiContext();

			auto loadedFonts = CEGUI::FontManager::getSingleton().createFromFile("Jura-13.font");
			auto defaultFont = loadedFonts.empty() ? 0 : loadedFonts.front();
			ceguiCtxt->setDefaultFont(defaultFont);
			CEGUI::FontManager::getSingleton().createFromFile("DejaVuSans-12.font");

			auto& winMgr = CEGUI::WindowManager::getSingleton();

			auto rootWin = winMgr.loadLayoutFromFile("GameMenuSample.layout");
			ceguiCtxt->setRootWindow(rootWin);
		}

	private:
		OptionalRef<CeguiAppComponent> _cegui;
	};
}

DARMOK_MAIN(CeguiApp);
