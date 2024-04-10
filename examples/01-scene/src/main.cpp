

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/input.hpp>
#include <darmok/mesh.hpp>
#include <darmok/anim.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/window.hpp>
#include <darmok/program.hpp>
#include <darmok/render_forward.hpp>

namespace
{
	using namespace darmok;

	struct Culling2D final
	{
	};

	struct Culling3D final
	{
	};

	class ScreenBounceUpdater final : public ISceneLogicUpdater
	{
	public:
		ScreenBounceUpdater(Transform& trans, const glm::vec2& size = {1, 1}, float speed = 100.f)
			: _trans(trans)
			, _size(size)
			, _dir(speed, speed)
		{
		}

		void init(Scene& scene, App& app) override
		{
			_app = app;
		}

		void update(float dt) override
		{
			auto margin = _size * 0.5F * glm::vec2(_trans.getScale());
			auto max = glm::vec2(_app->getWindow().getSize()) - margin;
			auto min = margin;
			auto pos = _trans.getPosition() + (glm::vec3(_dir, 0) * dt);
			if (pos.x > max.x)
			{
				pos.x = max.x;
				_dir.x *= -1;
			}
			if (pos.y > max.y)
			{
				pos.y = max.y;
				_dir.y *= -1;
			}
			if (pos.x < min.x)
			{
				pos.x = min.x;
				_dir.x *= -1;
			}
			if (pos.y < min.y)
			{
				pos.y = min.y;
				_dir.y *= -1;
			}
			_trans.setPosition(pos);
		}
	private:
		OptionalRef<App> _app;
		Transform& _trans;
		glm::vec2 _size;
		glm::vec2 _dir;
	};

	class RotateUpdater final : public ISceneLogicUpdater
	{
	public:
		RotateUpdater(Transform& trans, float speed = 100.f)
			: _trans(trans)
			, _speed(speed)
		{
		}

		void update(float dt) override
		{
			auto r = _trans.getRotation() + glm::vec3(0, dt * _speed, 0);
			_trans.setRotation(r);
		}

	private:
		Transform& _trans;
		float _speed;
	};

	class ExampleScene : public App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto& scene = addComponent<SceneAppComponent>().getScene();
			scene.addLogicUpdater<FrameAnimationUpdater>();

			glm::vec2 size = getWindow().getSize();

			auto prog = Program::createStandard(StandardProgramType::Unlit);
			_layout = prog->getVertexLayout();

			auto cam2d = scene.createEntity();
			scene.addComponent<Camera>(cam2d)
				.setOrtho(0.f, size.x, 0.f, size.y)
				.setEntityComponentFilter<Culling2D>()
				.setRenderer<ForwardRenderer>(prog);

			auto cam3d = scene.createEntity();
			scene.addComponent<Transform>(cam3d)
				.setPosition(glm::vec3(0.f, 2.f, -2.f))
				.setRotation(glm::vec3(45.f, 0, 0));

			scene.addComponent<Camera>(cam3d)
				.setProjection(60, size.x / size.y, 0.3, 1000)
				.setEntityComponentFilter<Culling3D>()
				.setRenderer<ForwardRenderer>(prog);

			_debugMaterial = std::make_shared<Material>();
			auto debugTexture = getAssets().getColorTextureLoader()(Colors::red);
			_debugMaterial->setTexture(MaterialTextureType::Diffuse, debugTexture);
			_debugMaterial->setPrimitiveType(MaterialPrimitiveType::Line);

			createBouncingSprite(scene);
			createSpriteAnimation(scene);
			createRotatingCube(scene);
		}

		void createBouncingSprite(Scene& scene)
		{
			auto tex = getAssets().getTextureLoader()("assets/engine.png");
			auto sprite = scene.createEntity();
			auto& trans = scene.addComponent<Transform>(sprite);
			trans.setPivot(glm::vec3(-0.5F));
			float scale = 0.5;
			auto mesh = Mesh::createSprite(tex, _layout, scale);
			auto size = scale * glm::vec2(tex->getImage()->getSize());
			auto debugMesh = Mesh::createLineQuad(_layout, size);
			debugMesh->setMaterial(_debugMaterial);
			scene.addComponent<MeshComponent>(sprite).setMeshes({ mesh, debugMesh });
			scene.addComponent<Culling2D>(sprite);
			scene.addLogicUpdater<ScreenBounceUpdater>(trans, size, 100.f);
		}

		void createSpriteAnimation(Scene& scene)
		{
			auto texAtlas = getAssets().getTextureAtlasLoader()("assets/warrior-0.xml", BGFX_SAMPLER_MAG_POINT);
			static const std::string animNamePrefix = "Attack/";
			auto animBounds = texAtlas->getBounds(animNamePrefix);
			auto anim = scene.createEntity();
			float scale = 2.f;
			auto frames = texAtlas->createSpriteAnimation(_layout, animNamePrefix, 0.1f, scale);
			
			auto& meshComp = scene.addComponent<MeshComponent>(anim);
			scene.addComponent<FrameAnimationComponent>(anim, frames, meshComp);
			
			scene.addComponent<Culling2D>(anim);
			auto& winSize = getWindow().getSize();
			scene.addComponent<Transform>(anim)
				.setPosition(glm::vec3(winSize, 0) / 2.f)
				.setPivot(glm::vec3(animBounds.size.x, animBounds.size.y, 0.f) / 2.f);
		}

		void createRotatingCube(Scene& scene)
		{
			auto texture = getAssets().getTextureLoader()("assets/brick.png");
			auto material = std::make_shared<Material>();
			material->setTexture(MaterialTextureType::Diffuse, texture);
			material->setColor(MaterialColorType::Diffuse, Colors::red);
			auto cubeMesh = Mesh::createCube(_layout);
			cubeMesh->setMaterial(material);

			auto cube = scene.createEntity();
			scene.addComponent<Culling3D>(cube);
			scene.addComponent<MeshComponent>(cube, cubeMesh);
			auto& trans = scene.addComponent<Transform>(cube);
			scene.addLogicUpdater<RotateUpdater>(trans, 100.f);
		}
	private:
		std::shared_ptr<Material> _debugMaterial;
		bgfx::VertexLayout _layout;
	};
}

DARMOK_MAIN(ExampleScene);
