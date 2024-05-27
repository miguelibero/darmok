#include <darmok/app.hpp>
#include <darmok/cegui.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/scene.hpp>

namespace
{
	using namespace darmok;

	class OzzSampleApp : public App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);
			auto skel = getAssets().getSkeletonLoader()("skeleton.ozz");
			auto anim = getAssets().getSkeletalAnimationLoader()("idle.ozz");

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			auto& registry = scene.getRegistry();

			auto cam = registry.create();
			registry.emplace<Transform>(cam)
				.setPosition(glm::vec3(0.f, 2.f, -2.f))
				.lookAt(glm::vec3(0, 0, 0));

			auto skelEntity = registry.create();
			auto& ctrl = registry.emplace<SkeletalAnimationController>(skelEntity, skel);
			ctrl.addAnimation(anim);
			ctrl.playAnimation(anim->getName());
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

DARMOK_RUN_APP(OzzSampleApp);
