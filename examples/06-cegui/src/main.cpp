#include <darmok/app.hpp>
#include <darmok/cegui.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <CEGUI/CEGUI.h>
#include <iostream>
#include "GameMenu.h"

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
			_cegui->setResourceGroupDirectory("anims", "cegui/animations/");

			CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");
			CEGUI::Font::setDefaultResourceGroup("fonts");
			CEGUI::Scheme::setDefaultResourceGroup("schemes");
			CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
			CEGUI::WindowManager::setDefaultResourceGroup("layouts");
			CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
			CEGUI::AnimationManager::setDefaultResourceGroup("anims");

			getWindow().requestCursorMode(WindowCursorMode::Hidden);

			_sample.initialise(_cegui->getGuiContext().ptr());
			_sample.onEnteringSample();
		}

		int shutdown() override
		{
			_sample.deinitialise();
			return App::shutdown();
		}
	protected:

		void updateLogic(float deltaTime) override
		{
			_sample.update(deltaTime);
		}

	private:
		OptionalRef<CeguiAppComponent> _cegui;
		GameMenuSample _sample;
	};
}

DARMOK_MAIN(CeguiApp);
