

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/input.hpp>
#include <darmok/mesh.hpp>
#include <darmok/anim.hpp>
#include <darmok/transform.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/camera.hpp>
#include <darmok/window.hpp>
#include <darmok/program.hpp>
#include <darmok/material.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/shape.hpp>

namespace
{
	using namespace darmok;

	struct Culling2D final
	{
	};

	struct Culling3D final
	{
	};

	class ScreenBounceUpdater final : public ISceneComponent
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

	class RotateUpdater final : public ISceneComponent
	{
	public:
		RotateUpdater(Transform& trans, float speed = 100.f)
			: _trans(trans)
			, _speed(speed)
		{
		}

		void update(float dt) override
		{
			auto r = _trans.getRotation() * glm::quat(glm::radians(glm::vec3(0, dt * _speed, 0)));
			_trans.setRotation(r);
		}

	private:
		Transform& _trans;
		float _speed;
	};

	class SceneSampleApp : public App
	{
	public:
		void init() override
		{
			App::init();

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			scene.addSceneComponent<FrameAnimationUpdater>();
			auto& registry = scene.getRegistry();

			_prog = std::make_shared<Program>(StandardProgramType::Unlit);

			auto cam3d = registry.create();
			registry.emplace<Transform>(cam3d)
				.setPosition(glm::vec3(0.f, 2.f, -2.f))
				.lookAt(glm::vec3(0, 0, 0));
			registry.emplace<Camera>(cam3d)
				.setWindowPerspective(60, 0.3, 1000)
				.setEntityComponentFilter<Culling3D>()
				.addRenderer<ForwardRenderer>();

			auto cam2d = registry.create();
			registry.emplace<Camera>(cam2d)
				.setWindowOrtho(glm::vec2(0))
				.setEntityComponentFilter<Culling2D>()
				.addRenderer<ForwardRenderer>();

			_debugMaterial = std::make_shared<Material>(_prog, Colors::red());
			_debugMaterial->setPrimitiveType(MaterialPrimitiveType::Line);

			createBouncingSprite(scene);
			createSpriteAnimation(scene);
			createRotatingCube(scene);
		}

		void createBouncingSprite(Scene& scene)
		{
			auto& registry = scene.getRegistry();
			auto tex = getAssets().getTextureLoader()("engine.png");
			auto sprite = registry.create();
			auto& trans = registry.emplace<Transform>(sprite);
			float scale = 0.5;

			MeshData meshData(Rectangle(tex->getSize()));
			meshData *= glm::vec3(0.5F);

			auto mesh = meshData.createMesh(_prog->getVertexLayout());
			auto mat = std::make_shared<Material>(_prog, tex);
			registry.emplace<Renderable>(sprite, std::move(mesh), mat);

			auto spriteBorder = registry.create();
			auto size = scale * glm::vec2(tex->getSize());
			meshData = MeshData(Rectangle::standard(), RectangleMeshType::Outline);
			meshData *= glm::vec3(size, 0);
			auto debugMesh = meshData.createMesh(_prog->getVertexLayout());

			registry.emplace<Renderable>(spriteBorder, std::move(debugMesh), _debugMaterial);
			registry.emplace<Transform>(spriteBorder).setParent(trans);
			registry.emplace<Culling2D>(spriteBorder);

			registry.emplace<Culling2D>(sprite);
			scene.addSceneComponent<ScreenBounceUpdater>(trans, size, 100.f);
		}

		void createSpriteAnimation(Scene& scene)
		{
			auto& registry = scene.getRegistry();
			auto texAtlas = getAssets().getTextureAtlasLoader()("warrior-0.xml", BGFX_SAMPLER_MAG_POINT);
			static const std::string animNamePrefix = "Attack/";
			auto animBounds = texAtlas->getBounds(animNamePrefix);
			auto anim = registry.create();

			TextureAtlasMeshConfig config;
			config.offset = - glm::vec3(animBounds.size.x, animBounds.size.y, 0.f) / 2.f;
			config.scale = glm::vec3(2.F);
			auto frames = texAtlas->createAnimation(_prog->getVertexLayout(), animNamePrefix, 0.1F, config);
			
			auto material = std::make_shared<Material>(_prog, texAtlas->texture);
			auto& renderable = registry.emplace<Renderable>(anim, material);
			registry.emplace<FrameAnimation>(anim, frames, renderable);
			
			registry.emplace<Culling2D>(anim);
			auto& winSize = getWindow().getSize();
			registry.emplace<Transform>(anim, glm::vec3(winSize, 0) / 2.f);
		}

		void createRotatingCube(Scene& scene)
		{
			auto texture = getAssets().getTextureLoader()("brick.png");
			auto material = std::make_shared<Material>(_prog, texture);
			material->setBaseColor(Colors::red());

			auto cubeMesh = MeshData(Cube()).createMesh(_prog->getVertexLayout());

			auto& registry = scene.getRegistry();
			auto cube = registry.create();
			registry.emplace<Culling3D>(cube);
			registry.emplace<Renderable>(cube, std::move(cubeMesh), material);
			auto& trans = registry.emplace<Transform>(cube);
			scene.addSceneComponent<RotateUpdater>(trans, 100.f);
		}
	private:
		std::shared_ptr<Material> _debugMaterial;
		std::shared_ptr<Program> _prog;
	};
}

DARMOK_RUN_APP(SceneSampleApp);
