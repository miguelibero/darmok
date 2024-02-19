

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/sprite.hpp>
#include <darmok/physics2d.hpp>
#include <darmok/input.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace
{
	class ExampleScene : public darmok::App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto& scene = addComponent<darmok::SceneAppComponent>().getScene();
			scene.addRenderer<darmok::SpriteRenderer>();
			scene.addRenderer<darmok::Physics2DDebugRenderer>();
			scene.addLogicUpdater<darmok::SpriteAnimationUpdater>();

			auto tex = darmok::AssetContext::get().getTextureLoader()("assets/darmok.jpg");
			
			auto camEntity = scene.createEntity();
			_cam = &scene.addComponent<darmok::Camera>(camEntity);
			
			// bouncing sprite
			auto spriteEntity = scene.createEntity();
			_spriteTrans = &scene.addComponent<darmok::Transform>(spriteEntity);
			scene.addComponent<darmok::SpriteComponent>(spriteEntity, darmok::Sprite::fromTexture(tex, _spriteSize));
			scene.addComponent<darmok::BoxCollider2D>(spriteEntity, _spriteSize);

			_spriteDir = { 1, 1 };
			_spriteDir *= 80.f;

			// sprite animation
			auto texAtlas = darmok::AssetContext::get().getTextureAtlasLoader()("assets/warrior-0.xml", BGFX_SAMPLER_MAG_POINT);
			static const std::string animNamePrefix = "Attack/";
			auto animBounds = texAtlas->getBounds(animNamePrefix);
			auto animEntity = scene.createEntity();
			scene.addComponent<darmok::SpriteAnimationComponent>(animEntity, darmok::Sprite::fromAtlas(*texAtlas, animNamePrefix));
			scene.addComponent<darmok::BoxCollider2D>(animEntity, glm::vec2(animBounds.size), glm::vec2(animBounds.offset));
			_animTrans = &scene.addComponent<darmok::Transform>(animEntity, glm::vec3{}, glm::vec3{}, glm::vec3(4));
		}

		void updateLogic(float dt) override
		{
			auto size = darmok::WindowContext::get().getWindow().getSize();
			_animTrans->setPosition(glm::vec3(size, 0) / 2.f);

			size -= _spriteSize;
			auto pos = _spriteTrans->getPosition() + (glm::vec3(_spriteDir, 0) * dt);
			if (pos.x > size.x)
			{
				pos.x = size.x;
				_spriteDir.x *= -1;
			}
			if (pos.y > size.y)
			{
				pos.y = size.y;
				_spriteDir.y *= -1;
			}
			if (pos.x < 0.f)
			{
				pos.x = 0.f;
				_spriteDir.x *= -1;
			}
			if (pos.y < 0.f)
			{
				pos.y = 0.f;
				_spriteDir.y *= -1;
			}
			_spriteTrans->setPosition(pos);
		}

		void beforeRender(bgfx::ViewId viewId) override
		{
			bgfx::setViewClear(viewId
				, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
				, 0x303030ff
				, 1.0f
				, 0
			);

			auto& size = darmok::WindowContext::get().getWindow(getViewWindow(viewId)).getSize();
			_cam->setMatrix(glm::ortho<float>(0.f, size.x, size.y, 0.0f, -1.f, 1000.0f));
		}
	private:
		darmok::Camera* _cam;

		darmok::Transform* _spriteTrans;
		static const glm::vec2 _spriteSize;
		glm::vec2 _spriteDir;

		darmok::TextureAtlas _texAtlas;
		darmok::BoxCollider2D* _animCollider;
		darmok::Transform* _animTrans;
	};

	const glm::vec2 ExampleScene::_spriteSize = { 100, 100 };
}

DARMOK_IMPLEMENT_MAIN(ExampleScene);
