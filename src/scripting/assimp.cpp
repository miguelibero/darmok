#include "assimp.hpp"
#include "scene.hpp"
#include <darmok/scene.hpp>
#include <darmok/assimp.hpp>

namespace darmok
{
	class LuaAssimpSceneAddToSceneCallback final
	{
	public:
		LuaAssimpSceneAddToSceneCallback(const std::weak_ptr<Scene>& scene, const sol::protected_function& callback)
			: _scene(scene)
			, _callback(callback)
		{
		}

		void operator()(const ModelNode& node, Entity entity) const noexcept
		{
			auto result = _callback(ConstLuaAssimpNode(node), LuaEntity(entity, _scene));
			if (result.valid())
			{
				return;
			}
			sol::error err = result;
			std::cerr << "error adding model to scene:" << std::endl;
			std::cerr << err.what() << std::endl;
		}


	private:
		std::weak_ptr<Scene> _scene;
		const sol::protected_function& _callback;
	};


	ConstLuaAssimpNode::ConstLuaAssimpNode(const ModelNode& node) noexcept
		: _node(node)
	{
	}

	const ModelNode& ConstLuaAssimpNode::getReal() const noexcept
	{
		return _node.value();
	}

	std::string ConstLuaAssimpNode::getName() const noexcept
	{
		return std::string(_node->getName());
	}

	LuaEntity ConstLuaAssimpNode::addToScene1(LuaScene& scene, const bgfx::VertexLayout& layout) const
	{
		auto& realScene = scene.getReal();
		auto entity = _node->addToScene(*realScene, layout);
		return LuaEntity(entity, realScene);
	}

	LuaEntity ConstLuaAssimpNode::addToScene2(LuaScene& scene, const bgfx::VertexLayout& layout, sol::protected_function callback) const
	{
		auto& realScene = scene.getReal();
		auto entity = _node->addToScene(*realScene, layout, LuaAssimpSceneAddToSceneCallback(realScene, callback));
		return LuaEntity(entity, realScene);
	}

	LuaEntity ConstLuaAssimpNode::addToScene3(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent) const
	{
		auto& realScene = scene.getReal();
		auto entity = _node->addToScene(*realScene, layout, parent.getReal());
		return LuaEntity(entity, realScene);
	}

	LuaEntity ConstLuaAssimpNode::addToScene4(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent, sol::protected_function callback) const
	{
		auto& realScene = scene.getReal();
		auto entity = _node->addToScene(*realScene, layout, parent.getReal(), LuaAssimpSceneAddToSceneCallback(realScene, callback));
		return LuaEntity(entity, realScene);
	}


	void ConstLuaAssimpNode::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<ConstLuaAssimpNode>("ConstModelNode",
			sol::constructors<>(),
			"name", sol::property(&ConstLuaAssimpNode::getName),
			"add_to_scene", sol::overload(
				&ConstLuaAssimpNode::addToScene1,
				&ConstLuaAssimpNode::addToScene2,
				&ConstLuaAssimpNode::addToScene3,
				&ConstLuaAssimpNode::addToScene4
			)
		);
	}


    LuaAssimpScene::LuaAssimpScene(const std::shared_ptr<AssimpScene>& scene) noexcept
		: _scene(scene)
		{
		}

	const std::shared_ptr<AssimpScene>& LuaAssimpScene::getReal() const noexcept
	{
		return _scene;
	}

	LuaEntity LuaAssimpScene::addToScene1(LuaScene& scene, const bgfx::VertexLayout& layout) const
	{
		auto& realScene = scene.getReal();
		auto entity = _scene->addToScene(*realScene, layout);
		return LuaEntity(entity, realScene);
	}

	LuaEntity LuaAssimpScene::addToScene2(LuaScene& scene, const bgfx::VertexLayout& layout, sol::protected_function callback) const
	{
		auto& realScene = scene.getReal();
		auto entity = _scene->addToScene(*realScene, layout, LuaAssimpSceneAddToSceneCallback(realScene, callback));
		return LuaEntity(entity, realScene);
	}

	LuaEntity LuaAssimpScene::addToScene3(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent) const
	{
		auto& realScene = scene.getReal();
		auto entity = _scene->addToScene(*realScene, layout, parent.getReal());
		return LuaEntity(entity, realScene);
	}

	LuaEntity LuaAssimpScene::addToScene4(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent, sol::protected_function callback) const
	{
		auto& realScene = scene.getReal();
		auto entity = _scene->addToScene(*realScene, layout, parent.getReal(), LuaAssimpSceneAddToSceneCallback(realScene, callback));
		return LuaEntity(entity, realScene);
	}

	void LuaAssimpScene::configure(sol::state_view& lua) noexcept
	{
		ConstLuaAssimpNode::configure(lua);
		lua.new_usertype<LuaAssimpScene>("Model",
			sol::constructors<>(),
			"add_to_scene", sol::overload(
				&LuaAssimpScene::addToScene1,
				&LuaAssimpScene::addToScene2,
				&LuaAssimpScene::addToScene3,
				&LuaAssimpScene::addToScene4
			)
		);
	}

}