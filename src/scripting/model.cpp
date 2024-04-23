#include "model.hpp"
#include "scene.hpp"
#include <darmok/scene.hpp>
#include <darmok/model.hpp>

namespace darmok
{
    LuaModel::LuaModel(const std::shared_ptr<Model>& model) noexcept
		: _model(model)
		{
		}

	const std::shared_ptr<Model>& LuaModel::getReal() const noexcept
	{
		return _model;
	}

	class LuaModelAddToSceneCallback final
	{
	public:
		LuaModelAddToSceneCallback(Scene& scene, const sol::protected_function& callback)
			: _scene(scene)
			, _callback(callback)
		{
		}

		void operator()(const ModelNode& node, Entity entity) const noexcept
		{
			auto result = _callback(node, LuaEntity(entity, _scene));
			if (result.valid())
			{
				return;
			}
			sol::error err = result;
			std::cerr << "error adding model to scene:" << std::endl;
			std::cerr << err.what() << std::endl;
		}


	private:
		Scene& _scene;
		const sol::protected_function& _callback;
	};

	LuaEntity LuaModel::addToScene1(LuaScene& scene, const bgfx::VertexLayout& layout)
	{
		auto& realScene = scene.getReal();
		auto entity = _model->addToScene(realScene, layout);
		return LuaEntity(entity, realScene);
	}

	LuaEntity LuaModel::addToScene2(LuaScene& scene, const bgfx::VertexLayout& layout, sol::protected_function callback)
	{
		auto& realScene = scene.getReal();
		auto entity = _model->addToScene(realScene, layout, LuaModelAddToSceneCallback(realScene, callback));
		return LuaEntity(entity, realScene);
	}

	LuaEntity LuaModel::addToScene3(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent)
	{
		auto& realScene = scene.getReal();
		auto entity = _model->addToScene(realScene, layout, parent.getReal());
		return LuaEntity(entity, realScene);
	}

	LuaEntity LuaModel::addToScene4(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent, sol::protected_function callback)
	{
		auto& realScene = scene.getReal();
		auto entity = _model->addToScene(realScene, layout, parent.getReal(), LuaModelAddToSceneCallback(realScene, callback));
		return LuaEntity(entity, realScene);
	}

	void LuaModel::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaModel>("Model",
			sol::constructors<>(),
			"add_to_scene", sol::overload(
				&LuaModel::addToScene1,
				&LuaModel::addToScene2,
				&LuaModel::addToScene3,
				&LuaModel::addToScene4
			)
		);
	}

}