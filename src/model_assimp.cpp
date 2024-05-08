
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
	AssimpModelNode::AssimpModelNode(const AssimpNode& assimp) noexcept
		: _assimp(assimp)
	{
	}

	std::string_view AssimpModelNode::getName() const noexcept
	{
		return _assimp.getName();
	}

	glm::mat4 AssimpModelNode::getTransform() const noexcept
	{
		return _assimp.getTransform();
	}

	const ReadOnlyCollection<IModelNode>& AssimpModelNode::getChildren() const noexcept
	{
		// TODO: do this
		return _children;
	}

	void AssimpModelNode::configureEntity(Entity entity, const ModelSceneConfig& config) const
	{
		auto assimpCam = _assimp.getCamera();
		if (assimpCam.hasValue())
		{
			configureCamera(assimpCam.value(), entity, config);
		}

		auto assimpLight = _assimp.getLight();
		if (assimpLight.hasValue())
		{
			configureLight(assimpLight.value(), entity, config);
		}

		auto& assimpMeshes = _assimp.getMeshes();
		if (!assimpMeshes.empty())
		{
			auto meshes = assimpMeshes.load(config.layout, config.assets.getTextureLoader(), config.assets.getAllocator());
			config.registry.emplace<MeshComponent>(entity, meshes);
		}
	}

	void AssimpModelNode::configureCamera(const AssimpCamera& cam, Entity entity, const ModelSceneConfig& config) const noexcept
	{
		config.registry.emplace<Camera>(entity, cam.getProjectionMatrix());
	}

	void AssimpModelNode::configureLight(const AssimpLight& light, Entity entity, const ModelSceneConfig& config) const noexcept
	{
		// TODO: decide what to do with light->getPosition()
		// transMat = glm::translate(transMat, light->getPosition());
		switch (light.getType())
		{
		case aiLightSource_POINT:
		{
			config.registry.emplace<PointLight>(entity)
				.setAttenuation(light.getAttenuation())
				.setDiffuseColor(light.getDiffuseColor())
				.setSpecularColor(light.getSpecularColor())
				;
			auto ambient = light.getAmbientColor();
			if (ambient != Colors::black3())
			{
				config.registry.emplace<AmbientLight>(entity)
					.setColor(ambient);
			}
			break;
		}
		case aiLightSource_AMBIENT:
			config.registry.emplace<AmbientLight>(entity)
				.setColor(light.getAmbientColor());
			break;
		}
	}

	AssimpModel::AssimpModel(AssimpScene&& assimp) noexcept
		: _assimp(std::move(assimp))
		, _rootNode(_assimp.getRootNode())
	{
	}

	IModelNode& AssimpModel::getRootNode() noexcept
	{
		return _rootNode;
	}

	const IModelNode& AssimpModel::getRootNode() const noexcept
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
		AssimpScene assimpScene(_importer, data.view(), std::string(name));
		return std::make_shared<AssimpModel>(std::move(assimpScene));
	}
}
