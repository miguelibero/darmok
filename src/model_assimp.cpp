
#include "model_assimp.hpp"
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/vertex.hpp>
#include <darmok/image.hpp>
#include <darmok/texture.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>

#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace darmok
{
	AssimpModelNodeChildrenCollection::AssimpModelNodeChildrenCollection(const AssimpNode& parent) noexcept
	{
		auto& children = parent.getChildren();
		_children.reserve(children.size());
		for (auto& child : children)
		{
			_children.push_back(std::make_shared<AssimpModelNode>(child));
		}
	}

	size_t AssimpModelNodeChildrenCollection::size() const noexcept
	{
		return _children.size();
	}

	std::shared_ptr<IModelNode> AssimpModelNodeChildrenCollection::operator[](size_t pos) const
	{
		return _children.at(pos);
	}

	AssimpModelNode::AssimpModelNode(const std::shared_ptr<AssimpNode>& assimp) noexcept
		: _assimp(assimp)
		, _children(*assimp)
	{
	}

	std::string_view AssimpModelNode::getName() const noexcept
	{
		return _assimp->getName();
	}

	glm::mat4 AssimpModelNode::getTransform() const noexcept
	{
		return _assimp->getTransform();
	}

	const ValCollection<std::shared_ptr<IModelNode>>& AssimpModelNode::getChildren() const noexcept
	{
		return _children;
	}

	void AssimpModelNode::configureEntity(Entity entity, const ModelSceneConfig& config) const
	{
		auto assimpCam = _assimp->getCamera();
		if (assimpCam != nullptr)
		{
			configureCamera(*assimpCam, entity, config);
		}

		auto assimpLight = _assimp->getLight();
		if (assimpLight != nullptr)
		{
			configureLight(*assimpLight, entity, config);
		}

		auto meshes = _assimp->loadMeshes(config.layout, config.assets.getTextureLoader(), config.assets.getAllocator());
		if (!meshes.empty())
		{
			config.registry.emplace<MeshComponent>(entity, meshes);
		}
	}

	void AssimpModelNode::configureCamera(const AssimpCamera& cam, Entity entity, const ModelSceneConfig& config) const noexcept
	{
		auto camEntity = config.registry.create();
		auto parentTrans = config.registry.try_get<Transform>(entity);
		config.registry.emplace<Transform>(camEntity, cam.getViewMatrix(), parentTrans);
		config.registry.emplace<Camera>(camEntity, cam.getProjectionMatrix());
	}

	void AssimpModelNode::configureLight(const AssimpLight& light, Entity entity, const ModelSceneConfig& config) const noexcept
	{
		auto lightEntity = config.registry.create();
		auto parentTrans = config.registry.try_get<Transform>(entity);
		config.registry.emplace<Transform>(lightEntity, parentTrans, light.getPosition());

		switch (light.getType())
		{
		case aiLightSource_POINT:
		{
			config.registry.emplace<PointLight>(lightEntity)
				.setAttenuation(light.getAttenuation())
				.setDiffuseColor(light.getDiffuseColor())
				.setSpecularColor(light.getSpecularColor())
				;
			auto ambient = light.getAmbientColor();
			if (ambient != Colors::black3())
			{
				config.registry.emplace<AmbientLight>(lightEntity)
					.setColor(ambient);
			}
			break;
		}
		case aiLightSource_AMBIENT:
			config.registry.emplace<AmbientLight>(lightEntity)
				.setColor(light.getAmbientColor());
			break;
		}
	}

	AssimpModel::AssimpModel(const AssimpScene& assimp) noexcept
		: _assimp(assimp)
		, _rootNode(std::make_shared<AssimpModelNode>(_assimp.getRootNode()))
	{
	}

	std::shared_ptr<IModelNode> AssimpModel::getRootNode() const noexcept
	{
		return _rootNode;
	}

	AssimpModelLoader::AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc)
		: _dataLoader(dataLoader)
		, _alloc(alloc)
	{
	}

	std::shared_ptr<IModel> AssimpModelLoader::operator()(std::string_view name)
	{
		auto data = _dataLoader(name);
		if (data.empty())
		{
			throw std::runtime_error("got empty data");
		}
		return std::make_shared<AssimpModel>(AssimpScene(_importer, data.view(), std::string(name)));
	}
}
