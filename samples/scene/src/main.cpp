

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
#include <darmok/program_standard.hpp>
#include <darmok/material.hpp>
#include <darmok/render.hpp>
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
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			scene.addLogicUpdater<FrameAnimationUpdater>();
			auto& registry = scene.getRegistry();

			_prog = getAssets().getStandardProgramLoader()(StandardProgramType::Unlit);

			auto cam2d = registry.create();
			registry.emplace<Camera>(cam2d)
				.setOrtho(getWindow().getSize())
				.setEntityComponentFilter<Culling2D>()
				.setRenderer<ForwardRenderer>();

			auto cam3d = registry.create();
			registry.emplace<Transform>(cam3d)
				.setPosition(glm::vec3(0.f, 2.f, -2.f))
				.lookAt(glm::vec3(0, 0, 0));

			registry.emplace<Camera>(cam3d)
				.setPerspective(60, getWindow().getSize(), 0.3, 1000)
				.setEntityComponentFilter<Culling3D>()
				.setRenderer<ForwardRenderer>();

			auto debugTexture = getAssets().getColorTextureLoader()(Colors::red());
			_debugMaterial = std::make_shared<Material>(_prog, debugTexture);
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
			trans.setPivot(glm::vec3(-0.5F));
			float scale = 0.5;

			MeshCreator meshCreator(_prog->getVertexLayout());
			meshCreator.config.type = MeshType::Dynamic;
			meshCreator.config.scale = glm::vec3(0.5F);
			auto mesh = meshCreator.createRectangle(Rectangle(tex->getSize()));
			auto mat = std::make_shared<Material>(_prog, tex);
			registry.emplace<Renderable>(sprite, mesh, mat);

			auto spriteBorder = registry.create();
			auto size = scale * glm::vec2(tex->getSize());
			meshCreator.config.scale = glm::vec3(size, 0);
			auto debugMesh = meshCreator.createLineRectangle();
			registry.emplace<Renderable>(spriteBorder, debugMesh, _debugMaterial);
			registry.emplace<Transform>(spriteBorder).setParent(trans);
			registry.emplace<Culling2D>(spriteBorder);

			registry.emplace<Culling2D>(sprite);
			scene.addLogicUpdater<ScreenBounceUpdater>(trans, size, 100.f);
		}

		void createSpriteAnimation(Scene& scene)
		{
			auto& registry = scene.getRegistry();
			auto texAtlas = getAssets().getTextureAtlasLoader()("warrior-0.xml", BGFX_SAMPLER_MAG_POINT);
			static const std::string animNamePrefix = "Attack/";
			auto animBounds = texAtlas->getBounds(animNamePrefix);
			auto anim = registry.create();

			TextureAtlasMeshCreator meshCreator(_prog->getVertexLayout(), *texAtlas);
			meshCreator.config.scale = glm::vec3(2.F);
			auto frames = meshCreator.createAnimation(animNamePrefix, 0.1f);
			
			auto material = std::make_shared<Material>(_prog, texAtlas->texture);
			auto& renderable = registry.emplace<Renderable>(anim, material);
			registry.emplace<FrameAnimation>(anim, frames, renderable);
			
			registry.emplace<Culling2D>(anim);
			auto& winSize = getWindow().getSize();
			registry.emplace<Transform>(anim)
				.setPosition(glm::vec3(winSize, 0) / 2.f)
				.setPivot(glm::vec3(animBounds.size.x, animBounds.size.y, 0.f) / 2.f);
		}

		void createRotatingCube(Scene& scene)
		{
			auto texture = getAssets().getTextureLoader()("brick.png");
			auto material = std::make_shared<Material>(_prog, texture);
			material->setColor(MaterialColorType::Diffuse, Colors::red());

			auto cubeMesh = MeshCreator(_prog->getVertexLayout()).createCube();

			auto& registry = scene.getRegistry();
			auto cube = registry.create();
			registry.emplace<Culling3D>(cube);
			registry.emplace<Renderable>(cube, cubeMesh, material);
			auto& trans = registry.emplace<Transform>(cube);
			scene.addLogicUpdater<RotateUpdater>(trans, 100.f);
		}
	private:
		std::shared_ptr<Material> _debugMaterial;
		std::shared_ptr<Program> _prog;
	};
}

DARMOK_RUN_APP(SceneSampleApp);
