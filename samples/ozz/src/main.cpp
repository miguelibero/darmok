#include <darmok/app.hpp>
#include <darmok/cegui.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/scene.hpp>
#include <darmok/model.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/render.hpp>

namespace
{
	using namespace darmok;

	class OzzSampleApp : public App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);
			
			auto& scene = *addComponent<SceneAppComponent>().getScene();
			auto& skelUpdater = scene.addLogicUpdater<SkeletalAnimationUpdater>();

			auto prog = getAssets().getStandardProgramLoader()(StandardProgramType::ForwardPhong);
			
			auto& registry = scene.getRegistry();

			auto camEntity = registry.create();
			registry.emplace<Transform>(camEntity)
				.setPosition(glm::vec3(0.f, 2, -2))
				.lookAt(glm::vec3(0, 1, 0));

			auto& cam = registry.emplace<Camera>(camEntity)
				.setPerspective(60, getWindow().getSize(), 0.3, 1000);
			cam.setRenderer<ForwardRenderer>();
			cam.addComponent<PhongLightingComponent>();
			cam.addComponent<SkeletalAnimationCameraComponent>();

			auto lightEntity = registry.create();
			registry.emplace<Transform>(lightEntity, glm::vec3{ 50, 50, -100 });
			registry.emplace<PointLight>(lightEntity);
			registry.emplace<AmbientLight>(registry.create(), 0.8);

			auto skel = getAssets().getSkeletonLoader()("skeleton.ozz");
			auto anim = getAssets().getSkeletalAnimationLoader()("BasicMotions@Run01 - Forwards.ozz");

			auto skelEntity = registry.create();
			registry.emplace<Transform>(skelEntity, glm::vec3(-1, 0, 0));
			auto& ctrl = registry.emplace<SkeletalAnimationController>(skelEntity, skel);
			ctrl.addAnimation(anim);
			ctrl.playAnimation(anim->getName());

			auto boneTex = getAssets().getColorTextureLoader()(Colors::grey());
			auto boneMat = std::make_shared<Material>(prog, boneTex);
			auto boneMesh = MeshCreator(prog->getVertexLayout()).createBone();
			registry.emplace<RenderableSkeleton>(skelEntity, boneMesh, boneMat);

			auto modelTex = getAssets().getTextureLoader()("BasicMotionsTexture.png");
			auto model = getAssets().getModelLoader()("BasicMotionsDummyModelBin.fbx");

			auto skinEntity = registry.create();
			registry.emplace<Transform>(skinEntity, glm::vec3(1, 0, 0));

			ModelSceneConfigurer configurer(scene.getRegistry(), prog, getAssets());
			configurer.setParent(skinEntity);
			configurer.run(model, [&registry, modelTex, skel, anim](const auto& node, Entity entity) {
				auto renderable = registry.try_get<Renderable>(entity);
				if (renderable != nullptr)
				{
					auto mat = renderable->getMaterial();
					if (mat != nullptr)
					{
						mat->setTexture(MaterialTextureType::Diffuse, modelTex);
					}
					auto& ctrl = registry.emplace<SkeletalAnimationController>(entity, skel);
					ctrl.addAnimation(anim);
					ctrl.playAnimation(anim->getName());
				}
			});
		}
	};
}

DARMOK_RUN_APP(OzzSampleApp);
