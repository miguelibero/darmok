
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
#include <darmok/render.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/program.hpp>

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
			_children.push_back(std::make_shared<ModelNode>(
				std::make_unique<ModelNodeImpl>(child)));
		}
	}

	size_t AssimpModelNodeChildrenCollection::size() const noexcept
	{
		return _children.size();
	}

	std::shared_ptr<ModelNode> AssimpModelNodeChildrenCollection::operator[](size_t pos) const
	{
		return _children.at(pos);
	}

	ModelNodeImpl::ModelNodeImpl(const std::shared_ptr<AssimpNode>& assimp) noexcept
		: _assimp(assimp)
		, _children(*assimp)
	{
	}

	std::string_view ModelNodeImpl::getName() const noexcept
	{
		return _assimp->getName();
	}

	glm::mat4 ModelNodeImpl::getTransform() const noexcept
	{
		return _assimp->getTransform();
	}

	const ValCollection<std::shared_ptr<ModelNode>>& ModelNodeImpl::getChildren() const noexcept
	{
		return _children;
	}

	void ModelNodeImpl::configureEntity(Entity entity, const ModelSceneConfig& config) const
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
		for (auto& assimpMesh : _assimp->getMeshes())
		{
			configureMesh(*assimpMesh, entity, config);
		}
	}

	void ModelNodeImpl::configureMesh(const AssimpMesh& assimpMesh, Entity entity, const ModelSceneConfig& config) const noexcept
	{
		auto meshEntity = config.registry.create();
		auto parentTrans = config.registry.try_get<Transform>(entity);
		config.registry.emplace<Transform>(meshEntity, parentTrans);
		auto assimpMat = assimpMesh.getMaterial();
		if (assimpMat != nullptr)
		{
			auto mat = assimpMat->load(config.assets.getTextureLoader(), config.assets.getAllocator());
			if (mat->getProgram() == nullptr)
			{
				mat->setProgram(config.program);
			}
			auto mesh = assimpMesh.load(config.program->getVertexLayout(), config.assets.getTextureLoader(), config.assets.getAllocator());
			config.registry.emplace<Renderable>(entity, mesh, mat);
			auto armature = assimpMesh.loadArmature();
			if (armature != nullptr)
			{
				config.registry.emplace<Skinnable>(entity, armature);
			}
		}
	}

	void ModelNodeImpl::configureCamera(const AssimpCamera& cam, Entity entity, const ModelSceneConfig& config) const noexcept
	{
		auto camEntity = config.registry.create();
		auto parentTrans = config.registry.try_get<Transform>(entity);
		config.registry.emplace<Transform>(camEntity, cam.getViewMatrix(), parentTrans);
		config.registry.emplace<Camera>(camEntity, cam.getProjectionMatrix());
	}

	void ModelNodeImpl::configureLight(const AssimpLight& light, Entity entity, const ModelSceneConfig& config) const noexcept
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

	ModelImpl::ModelImpl(const AssimpScene& assimp) noexcept
		: _assimp(assimp)
		, _rootNode(std::make_shared<ModelNode>(std::make_unique<ModelNodeImpl>(_assimp.getRootNode())))
	{
	}

	std::shared_ptr<ModelNode> ModelImpl::getRootNode() const noexcept
	{
		return _rootNode;
	}

	Model::Model(std::unique_ptr<ModelImpl>&& impl) noexcept
		: _impl(std::move(impl))
	{
	}

	std::shared_ptr<ModelNode> Model::getRootNode() const noexcept
	{
		return _impl->getRootNode();
	}

	ModelNode::ModelNode(std::unique_ptr<ModelNodeImpl>&& impl) noexcept
		: _impl(std::move(impl))
	{
	}

	std::string_view ModelNode::getName() const noexcept
	{
		return _impl->getName();
	}

	glm::mat4 ModelNode::getTransform() const noexcept
	{
		return _impl->getTransform();
	}

	const ValCollection<std::shared_ptr<ModelNode>>& ModelNode::getChildren() const noexcept
	{
		return _impl->getChildren();
	}

	void ModelNode::configureEntity(Entity entity, const ModelSceneConfig& config) const
	{
		_impl->configureEntity(entity, config);
	}

	AssimpModelLoader::AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc)
		: _dataLoader(dataLoader)
		, _alloc(alloc)
	{
	}

	std::shared_ptr<Model> AssimpModelLoader::operator()(std::string_view name)
	{
		auto data = _dataLoader(name);
		if (data.empty())
		{
			throw std::runtime_error("got empty data");
		}
		return std::make_shared<Model>(
			std::make_unique<ModelImpl>(AssimpScene(_importer, data.view(), std::string(name))));
	}
}
