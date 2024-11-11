#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/glm.hpp>
#include <darmok/scene.hpp>
#include <darmok/serialize.hpp>
#include <cereal/archives/adapters.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/unordered_set.hpp>
#include <vector>
#include <unordered_set>

namespace darmok
{
	class Scene;
	class App;

	struct DARMOK_EXPORT SceneArchiveData final
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
	class SceneOutputArchive final : public cereal::UserDataAdapter<SceneArchiveData, Archive>
	{
	public:
		template<typename... Args>
		SceneOutputArchive(App& app, Scene& scene, Args&&... args)
			: _userData{ app, scene }
			, cereal::UserDataAdapter<SceneArchiveData, Archive>(_userData, std::forward<Args>(args)...)
			, _snapshot(scene.createSnapshot())
		{
		}

		template<typename T>
		SceneOutputArchive& saveComponents()
		{
			_snapshot.get<T>(*this);
			return *this;
		}

		template<typename T>
		SceneOutputArchive& lateSaveComponents()
		{
			auto& scene = _userData.scene.get();
			for (auto entity : scene.getEntities<T>())
			{
				SerializeUtils::lateSave(*this, scene.getComponent<T>(entity).value());
			}
			return *this;
		}

	private:
		SceneArchiveData _userData;
		SceneSnapshot _snapshot;
	};

	template<typename Archive>
	class SceneInputArchive final : public cereal::UserDataAdapter<SceneArchiveData, Archive>
	{
	public:
		template<typename... Args>
		SceneInputArchive(App& app, Scene& scene, Args&&... args)
			: _userData{ app, scene }
			, cereal::UserDataAdapter<SceneArchiveData, Archive>(_userData, std::forward<Args>(args)...)
			, _snapshotLoader(scene.createSnapshotLoader())
		{
		}

		template<typename T>
		SceneInputArchive& loadComponents()
		{
			_snapshotLoader.get<T>(*this);
			return *this;
		}

		template<typename T>
		SceneInputArchive& lateLoadComponents()
		{
			auto& scene = _userData.scene.get();
			for (auto entity : scene.getEntities<T>())
			{
				SerializeUtils::lateSave(*this, scene.getComponent<T>(entity).value());
			}
			return *this;
		}

	private:
		SceneArchiveData _userData;
		SceneSnapshotLoader _snapshotLoader;
	};	

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
	class SerializedEntityComponentRefUnorderedSet final
	{
	public:
		using Collection = std::unordered_set<std::reference_wrapper<T>>;

		SerializedEntityComponentRefUnorderedSet(Collection& collection) noexcept
			: _collection(collection)
		{
		}

		template<typename Archive>
		void save(Archive& archive) const
		{
			std::unordered_set<Entity> entities;
			entities.reserve(_collection.size());
			auto& scene = getSceneFromArchive(archive);
			for (auto& ref : _collection)
			{
				entities.emplace(scene.getEntity(ref.get()));
			}
			archive(entities);
		}

		template<typename Archive>
		void load(Archive& archive)
		{
			auto& scene = getSceneFromArchive(archive);
			std::unordered_set<Entity> entities;
			archive(entities);
			_collection.reserve(_collection.size() + entities.size());
			for (auto entity : entities)
			{
				if (auto comp = scene.getComponent<T>(entity))
				{
					_collection.emplace(comp.value());
				}
			}
		}
	private:
		Collection& _collection;
	};

	template<typename T>
	class SerializedEntityComponentRefVector final
	{
	public:
		using Collection = std::vector<std::reference_wrapper<T>>;

		SerializedEntityComponentRefVector(Collection& collection) noexcept
			: _collection(collection)
		{
		}

		template<typename Archive>
		void save(Archive& archive) const
		{
			std::vector<Entity> entities;
			entities.reserve(_collection.size());
			auto& scene = getSceneFromArchive(archive);
			for (auto& ref : _collection)
			{
				entities.emplace_back(scene.getEntity(ref.get()));
			}
			archive(entities);
		}

		template<typename Archive>
		void load(Archive& archive)
		{
			auto& scene = getSceneFromArchive(archive);
			std::vector<Entity> entities;
			archive(entities);
			_collection.reserve(_collection.size() + entities.size());
			for (auto entity : entities)
			{
				if (auto comp = scene.getComponent<T>(entity))
				{
					_collection.emplace_back(comp.value());
				}
			}
		}
	private:
		Collection& _collection;
	};

	struct SceneSerializeUtils final
	{
		template<typename T>
		static SerializedEntityComponentRef<T> createComponentRef(OptionalRef<T>& ref)
		{
			return { ref };
		}

		template<typename T>
		static SerializedEntityComponentRefUnorderedSet<T> createComponentRefCollection(std::unordered_set<std::reference_wrapper<T>>& refs)
		{
			return { refs };
		}

		template<typename T>
		static SerializedEntityComponentRefVector<T> createComponentRefCollection(std::vector<std::reference_wrapper<T>>& refs)
		{
			return { refs };
		}
	};
}