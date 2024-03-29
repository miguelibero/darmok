

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/sprite.hpp>
#include <darmok/input.hpp>
#include <darmok/mesh.hpp>
#include <darmok/anim.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/window.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/program_def.hpp>

namespace
{
	struct Culling2D final
	{
	};

	struct Culling3D final
	{
	};

	class ScreenBounceUpdater final : public darmok::ISceneLogicUpdater
	{
	public:
		ScreenBounceUpdater(darmok::Transform& trans, const glm::vec2& size = {1, 1}, float speed = 100.f)
			: _trans(trans)
			, _size(size)
			, _dir(speed, speed)
		{
		}

		void init(darmok::Scene& scene, darmok::App& app) override
		{
			_app = app;
		}

		void update(float dt) override
		{
			auto size = _app->getWindow().getSize();
			size -= _size * glm::vec2(_trans.getScale());
			auto pos = _trans.getPosition() + (glm::vec3(_dir, 0) * dt);
			if (pos.x > size.x)
			{
				pos.x = size.x;
				_dir.x *= -1;
			}
			if (pos.y > size.y)
			{
				pos.y = size.y;
				_dir.y *= -1;
			}
			if (pos.x < 0.f)
			{
				pos.x = 0.f;
				_dir.x *= -1;
			}
			if (pos.y < 0.f)
			{
				pos.y = 0.f;
				_dir.y *= -1;
			}
			_trans.setPosition(pos);
		}
	private:
		darmok::OptionalRef<darmok::App> _app;
		darmok::Transform& _trans;
		glm::vec2 _size;
		glm::vec2 _dir;
	};

	class RotateUpdater final : public darmok::ISceneLogicUpdater
	{
	public:
		RotateUpdater(darmok::Transform& trans, float speed = 100.f)
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
		darmok::Transform& _trans;
		float _speed;
	};

	class ExampleScene : public darmok::App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto& scene = addComponent<darmok::SceneAppComponent>().getScene();
			auto& renderer = scene.addRenderer<darmok::ForwardRenderer>();
			_progDef = renderer.getProgramDefinition();
			scene.addLogicUpdater<darmok::FrameAnimationUpdater>();

			glm::vec2 size = getWindow().getSize();

			auto cam2d = scene.createEntity();
			scene.addComponent<darmok::Camera>(cam2d)
				.setOrtho(0.f, size.x, 0.f, size.y)
				.setEntityComponentFilter<Culling2D>()
				;

			auto cam3d = scene.createEntity();
			scene.addComponent<darmok::Transform>(cam3d)
				.setPosition(glm::vec3(0.f, 2.f, -2.f))
				.setRotation(glm::vec3(45.f, 0, 0));

			scene.addComponent<darmok::Camera>(cam3d)
				.setProjection(60, size.x / size.y, 0.3, 1000)
				.setEntityComponentFilter<Culling3D>()
				;

			_debugMaterial = std::make_shared<darmok::Material>(_progDef);
			auto debugTexture = getAssets().getColorTextureLoader()(darmok::Colors::red);
			_debugMaterial->setTexture(darmok::MaterialTextureType::Diffuse, debugTexture);
			_debugMaterial->setPrimitiveType(darmok::MaterialPrimitiveType::Line);

			createBouncingSprite(scene);
			createSpriteAnimation(scene);
			createRotatingCube(scene);
		}

		void createBouncingSprite(darmok::Scene& scene)
		{
			auto tex = getAssets().getTextureLoader()("assets/engine.png");
			auto sprite = scene.createEntity();
			auto& trans = scene.addComponent<darmok::Transform>(sprite);
			float scale = 0.5;
			auto mesh = darmok::SpriteUtils::fromTexture(tex, _progDef, scale);
			auto size = scale * glm::vec2(tex->getImage()->getSize());
			auto debugMesh = darmok::Mesh::createQuad(_debugMaterial, size);
			scene.addComponent<darmok::MeshComponent>(sprite).setMeshes({ mesh, debugMesh });
			scene.addComponent<Culling2D>(sprite);
			scene.addLogicUpdater<ScreenBounceUpdater>(trans, size, 100.f);
		}

		void createSpriteAnimation(darmok::Scene& scene)
		{
			auto texAtlas = getAssets().getTextureAtlasLoader()("assets/warrior-0.xml", BGFX_SAMPLER_MAG_POINT);
			static const std::string animNamePrefix = "Attack/";
			auto animBounds = texAtlas->getBounds(animNamePrefix);
			auto anim = scene.createEntity();
			float scale = 2.f;
			auto frames = darmok::SpriteUtils::fromAtlas(*texAtlas, _progDef, animNamePrefix, 0.1f, scale);
			auto size = glm::vec2(animBounds.size) * scale;
			auto debugMesh = darmok::Mesh::createQuad(_debugMaterial, size);
			
			auto& meshComp = scene.addComponent<darmok::MeshComponent>(anim);
			scene.addComponent<darmok::FrameAnimationComponent>(anim, frames, meshComp);
			
			scene.addComponent<Culling2D>(anim);
			auto& winSize = getWindow().getSize();
			scene.addComponent<darmok::Transform>(anim)
				.setPosition(glm::vec3(winSize, 0) / 2.f)
				.setPivot(glm::vec3(animBounds.size.x, animBounds.size.y, 0.f) / 2.f);
		}

		void createRotatingCube(darmok::Scene& scene)
		{
			auto texture = getAssets().getTextureLoader()("assets/brick.png");
			auto material = std::make_shared<darmok::Material>(_progDef);
			material->setTexture(darmok::MaterialTextureType::Diffuse, texture);
			material->setColor(darmok::MaterialColorType::Diffuse, darmok::Colors::red);
			auto cube = scene.createEntity();
			scene.addComponent<Culling3D>(cube);
			scene.addComponent<darmok::MeshComponent>(cube, darmok::Mesh::createCube(material));
			auto& trans = scene.addComponent<darmok::Transform>(cube);
			scene.addLogicUpdater<RotateUpdater>(trans, 100.f);
		}
	private:
		std::shared_ptr<darmok::Material> _debugMaterial;
		darmok::ProgramDefinition _progDef;
	};
}

DARMOK_MAIN(ExampleScene);
