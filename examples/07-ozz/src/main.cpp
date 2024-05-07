#include <darmok/app.hpp>
#include <darmok/cegui.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/skeleton.hpp>

namespace
{
	using namespace darmok;

	class OzzApp : public App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);
			auto skel = getAssets().getSkeletonLoader()("skeleton.ozz");
			auto anim = getAssets().getSkeletalAnimationLoader()("idle.ozz");
		}

		int shutdown() override
		{
			return App::shutdown();
		}
	protected:

		void updateLogic(float deltaTime) override
		{
		}

	private:
	};
}

DARMOK_MAIN(OzzApp);
