#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <cereal/archives/adapters.hpp>

namespace darmok
{
	class Scene;
	class App;

	struct SceneArchiveData final
	{
		std::reference_wrapper<App> app;
		std::reference_wrapper<Scene> scene;
	};

	template<typename Archive>
	static Scene& getSceneFromArchive(Archive& archive) noexcept
	{
		return cereal::get_user_data<SceneArchiveData>(archive).scene.get();
	}

	template<typename Archive>
	static App& getAppFromArchive(Archive& archive) noexcept
	{
		return cereal::get_user_data<SceneArchiveData>(archive).app.get();
	}

	template<typename Archive>
	using SceneArchive = cereal::UserDataAdapter<SceneArchiveData, Archive>;

	template<typename T>
	class SerializedEntityComponentRef final
	{
	public:
		SerializedEntityComponentRef(OptionalRef<T>& ref) noexcept
			: _ref(ref)
		{
		}

		template<typename Archive>
		void save(Archive& archive) const
		{
			Entity entity = entt::null;
			if (_ref)
			{
				auto& scene = getSceneFromArchive(archive);
				entity = scene.getEntity(_ref.value());
			}
			archive(entity);
		}


		template<typename Archive>
		void load(Archive& archive) const
		{
			Entity entity;
			archive(entity);
			if (entity != entt::null)
			{
				auto& scene = getSceneFromArchive(archive);
				_ref = scene.getComponent<T>(entity);
			}
		}

	private:
		OptionalRef<T>& _ref;
	};

	template<typename T>
	static SerializedEntityComponentRef<T> createSerializedEntityComponentRef(OptionalRef<T>& ref)
	{
		return { ref };
	}
}
