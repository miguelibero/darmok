#include <darmok/app.hpp>
#include <darmok/cegui.hpp>
#include <darmok/optional_ref.hpp>

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
			
		}

	private:
		OptionalRef<CeguiAppComponent> _cegui;
	};
}

DARMOK_MAIN(CeguiApp);
