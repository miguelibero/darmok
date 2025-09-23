

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
#include <darmok/scene_filter.hpp>
#include <darmok/shape.hpp>

namespace
{
	using namespace darmok;

	struct Culling2D final
	{
		bool v; // entt does not accept empty structs
	};

	struct Culling3D final
	{
		bool v; // entt does not accept empty structs
	};

	class ScreenBounceUpdater final : public IAppUpdater
	{
	public:
		ScreenBounceUpdater(Window& win, Transform& trans, const glm::vec2& size = {1, 1}, float speed = 100.f)
			: _win(win)
			, _trans(trans)
			, _size(size)
			, _dir(speed)
		{
		}

		void update(float dt) override
		{
			auto margin = _size * 0.5F * glm::vec2(_trans.getScale());
			auto max = glm::vec2(_win.getPixelSize()) - margin;
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
		Window& _win;
		Transform& _trans;
		glm::vec2 _size;
		glm::vec2 _dir;
	};

	class RotateUpdater final : public IAppUpdater
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

	class SceneSampleAppDelegate final : public IAppDelegate
	{
	public:
		SceneSampleAppDelegate(App& app)
			: _app(app)
		{
		}

		void init() override
		{
			auto& scene = *_app.addComponent<SceneAppComponent>().getScene();
			scene.addSceneComponent<FrameAnimationUpdater>();

			_prog = StandardProgramLoader::load(Program::Standard::Unlit);

			auto cam3d = scene.createEntity();
			scene.addComponent<Transform>(cam3d)
				.setPosition(glm::vec3{ 0.f, 2.f, -2.f })
				.lookAt(glm::vec3{ 0, 0, 0 });
			scene.addComponent<Camera>(cam3d)
				.setPerspective(glm::radians(60.f), 0.3f, 1000.f)
				.setCullingFilter<Culling3D>()
				.addComponent<ForwardRenderer>();

			auto cam2d = scene.createEntity();
			scene.addComponent<Camera>(cam2d)
				.setOrtho(glm::vec2{ 0.f })
				.setCullingFilter<Culling2D>()
				.addComponent<ForwardRenderer>();

			_debugMaterial = std::make_shared<Material>(_prog, Colors::red());
			_debugMaterial->primitiveType = Material::Definition::Line;

			createBouncingSprite(scene);
			createSpriteAnimation(scene);
			createRotatingCube(scene);
		}

		void createBouncingSprite(Scene& scene)
		{
			auto tex = _app.getAssets().getTextureLoader()("engine.png").value();
			auto sprite = scene.createEntity();
			auto& trans = scene.addComponent<Transform>(sprite);
			float scale = 0.5f;
			auto& win = _app.getWindow();
			auto fbScale = win.getFramebufferScale();

			MeshData meshData{ Rectangle{tex->getSize()}};
			meshData.scalePositions(glm::vec3{ 0.5f } * glm::vec3{ fbScale, 1.f });

			auto mesh = std::make_shared<Mesh>(meshData.createMesh(_prog->getVertexLayout()));
			auto mat = std::make_shared<Material>(_prog, tex);
			mat->opacityType = Material::Definition::Transparent;
			scene.addComponent<Renderable>(sprite, std::move(mesh), mat);

			auto spriteBorder = scene.createEntity();
			auto size = scale * glm::vec2{ tex->getSize() } *fbScale;
			meshData = MeshData{ Rectangle::standard(), Mesh::Definition::OutlineRectangle };
			meshData.scalePositions(glm::vec3{ size, 0.f });
			auto debugMesh = std::make_shared<Mesh>(meshData.createMesh(_prog->getVertexLayout()));

			scene.addComponent<Renderable>(spriteBorder, std::move(debugMesh), _debugMaterial);
			scene.addComponent<Transform>(spriteBorder).setParent(trans);
			scene.addComponent<Culling2D>(spriteBorder);

			scene.addComponent<Culling2D>(sprite);
			_app.addUpdater<ScreenBounceUpdater>(win, trans, size, 100.f);
		}

		void createSpriteAnimation(Scene& scene)
		{
			auto texAtlasDef = _app.getAssets().getTextureAtlasLoader().loadDefinition("warrior-0.xml").value();
			auto texDef = _app.getAssets().getTextureLoader().loadDefinition(texAtlasDef->texture_path()).value();
			texDef->set_flags(texDef->flags() | BGFX_SAMPLER_MAG_POINT);
			auto texAtlas = _app.getAssets().getTextureAtlasLoader().loadResource(texAtlasDef).value();
			static const std::string animNamePrefix = "Attack/";
			auto animBounds = texAtlas->getBounds(animNamePrefix);
			auto anim = scene.createEntity();
			auto& win = _app.getWindow();

			TextureAtlasMeshConfig config;
			config.offset = -glm::vec3{ animBounds.size.x, animBounds.size.y, 0.f } / 2.f;
			config.scale = glm::vec3{ 2.f } *glm::vec3{ win.getFramebufferScale(), 1.f };
			auto frames = texAtlas->createAnimation(_prog->getVertexLayout(), animNamePrefix, 0.1f, config);
			
			auto material = std::make_shared<Material>(_prog, texAtlas->texture);
			material->opacityType = Material::Definition::Transparent;
			auto& renderable = scene.addComponent<Renderable>(anim, material);
			scene.addComponent<FrameAnimation>(anim, frames, renderable);
			
			scene.addComponent<Culling2D>(anim);
			auto& winSize = win.getPixelSize();
			scene.addComponent<Transform>(anim, glm::vec3{ winSize, 0.f } / 2.f);
		}

		void createRotatingCube(Scene& scene)
		{
			auto texture = _app.getAssets().getTextureLoader()("brick.png").value();
			auto material = std::make_shared<Material>(_prog, texture);
			material->baseColor = Colors::red();

			auto meshData = MeshData{ Cube{} };
			meshData.subdivideDensity(0.25F);
			auto cubeMesh = std::make_shared<Mesh>(meshData.createMesh(_prog->getVertexLayout()));

			auto cube = scene.createEntity();
			scene.addComponent<Culling3D>(cube);
			scene.addComponent<Renderable>(cube, std::move(cubeMesh), material);
			auto& trans = scene.addComponent<Transform>(cube);
			_app.addUpdater<RotateUpdater>(trans, 100.f);
		}
	private:
		App& _app;
		std::shared_ptr<Material> _debugMaterial;
		std::shared_ptr<Program> _prog;
	};
}

DARMOK_RUN_APP(SceneSampleAppDelegate);
