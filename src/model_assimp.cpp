
#include "model_assimp.hpp"
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/vertex.hpp>
#include <darmok/image.hpp>
#include <darmok/texture.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>

#include <assimp/postprocess.h>

namespace darmok
{
	ModelImpl::ModelImpl(AssimpScene&& scene, const std::string& path = {}, const OptionalRef<ITextureLoader>& textureLoader, bx::AllocatorI* alloc)
	{
	}

	ModelNode& ModelImpl::getRootNode() noexcept;
	Entity ModelImpl::addToScene(Scene& scene, const bgfx::VertexLayout& layout, Entity parent = entt::null) const;


	AssimpModelLoader::AssimpModelLoader(IDataLoader& dataLoader)
		: _dataLoader(dataLoader)
	{
	}

	std::shared_ptr<Model> AssimpModelLoader::operator()(std::string_view name)
	{
		auto data = _dataLoader(name);
		if (data.empty())
		{
			throw std::runtime_error("got empty data");
		}
		unsigned int flags = aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType | 
			aiProcess_ConvertToLeftHanded
			;
		// assimp (and opengl) is right handed (+Z points towards the camera)
		// while bgfx (and darmok and directx) is left handed (+Z points away from the camera)

		std::string nameString(name);
		auto scene = _importer.ReadFileFromMemory(data.ptr(), data.size(), flags, nameString.c_str());

		if (scene == nullptr)
		{
			throw std::runtime_error(_importer.GetErrorString());
		}

		return std::make_shared<Model>(
			std::make_unique<ModelImpl>(
				AssimpScene(scene), nameString))
		);
	}
}
