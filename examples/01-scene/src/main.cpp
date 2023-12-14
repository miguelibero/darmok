

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

			auto tex = darmok::AssetContext::get().loadTexture("assets/darmok.jpg");
			
			auto camEntity = scene.createEntity();
			_cam = &scene.addComponent<darmok::Camera>(camEntity);
			
			auto atlas = darmok::AssetContext::get().loadAtlas("assets/warrior-0.xml");

			auto spriteEntity = scene.createEntity();
			_spriteTrans = &scene.addComponent<darmok::Transform>(spriteEntity);
			scene.addComponent<darmok::Sprite>(spriteEntity, darmok::SpriteData::fromTexture(tex, _spriteSize));
			
			_spriteDir = { 1, 1 };
			_spriteDir *= 20.f;

			auto atlasSpriteEntity = scene.createEntity();
			_atlasSpriteTrans = &scene.addComponent<darmok::Transform>(atlasSpriteEntity);
			auto& atlasElm = atlas.elements[2];
			scene.addComponent<darmok::Sprite>(atlasSpriteEntity, darmok::SpriteData::fromAtlas(atlas, atlasElm));
			scene.addComponent<darmok::BoxCollider2D>(atlasSpriteEntity, atlasElm.originalSize);
			//_atlasSpriteTrans->setPivot(glm::vec3(atlasElm.originalSize, 0) / 2.f);
			_atlasSpriteTrans->setScale(glm::vec3(4));
			_atlasSpriteTrans->setPosition(glm::vec3(10));
		}

		void updateLogic(float dt) override
		{
			auto size = darmok::WindowContext::get().getWindow().getSize();
			//_atlasSpriteTrans->setPosition(glm::vec3(size, 0) / 2.f);

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
			_cam->setMatrix(glm::ortho<float>(0.f, size.x, 0.0f, size.y, -1.f, 1000.0f));
		}
	private:
		darmok::Camera* _cam;
		darmok::Transform* _spriteTrans;
		darmok::Transform* _atlasSpriteTrans;
		static const glm::vec2 _spriteSize;
		glm::vec2 _spriteDir;
	};

	const glm::vec2 ExampleScene::_spriteSize = { 100, 100 };
}

DARMOK_IMPLEMENT_MAIN(ExampleScene);
