
#include <darmok/model.hpp>
#include <darmok/asset.hpp>

#include <assimp/vector3.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>


namespace darmok
{
	std::string_view getStringView(const aiString& str)
	{
		return std::string_view(str.data, str.length);
	}

	ModelTexture::ModelTexture(aiMaterial* ptr, ModelTextureType type, unsigned int index)
		: _type(type)
	{
		aiString path;
		unsigned int uvindex;
		ptr->GetTexture((aiTextureType)type, index, &path,
			(aiTextureMapping*)&_mapping,
			&uvindex,
			&_blend,
			(aiTextureOp*) & _operation,
			(aiTextureMapMode*)&_mapMode
		);
		_coordIndex = uvindex;
		_path = path.C_Str();
	}

	ModelTextureType ModelTexture::getType() const
	{
		return _type;
	}

	const std::string& ModelTexture::getPath() const
	{
		return _path;
	}

	ModelTextureMapping ModelTexture::getMapping() const
	{
		return _mapping;
	}

	ModelTextureOperation ModelTexture::getOperation() const
	{
		return _operation;
	}

	ModelTextureMapMode ModelTexture::getMapMode() const
	{
		return _mapMode;
	}

	float ModelTexture::getBlend() const
	{
		return _blend;
	}

	size_t ModelTexture::getCoordIndex() const
	{
		return _coordIndex;
	}

	ModelTextureContainer::ModelTextureContainer(aiMaterial* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelTextureContainer::size() const
	{
		size_t size = 0;
		for (size_t i = 0; i < (size_t)ModelTextureType::Count; i++)
		{
			size += _ptr->GetTextureCount((aiTextureType)i);
		}
		return size;
	}

	ModelTexture ModelTextureContainer::create(size_t pos) const
	{
		size_t count = 0;
		for (size_t i = 0; i < (size_t)ModelTextureType::Count; i++)
		{
			size_t typeCount = _ptr->GetTextureCount((aiTextureType)i);
			size_t newCount = count + typeCount;
			if (newCount > pos)
			{
				return ModelTexture(_ptr, (ModelTextureType)i, pos - count);
			}
			count = newCount;
		}
	}

	ModelMaterialProperty::ModelMaterialProperty(aiMaterialProperty* ptr)
		: _ptr(ptr)
	{
	}

	std::string_view ModelMaterialProperty::getKey() const
	{
		return getStringView(_ptr->mKey);
	}

	ModelMaterialPropertyType ModelMaterialProperty::getType() const
	{
		return (ModelMaterialPropertyType)_ptr->mType;
	}

	ModelMaterialPropertyContainer::ModelMaterialPropertyContainer(aiMaterial* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelMaterialPropertyContainer::size() const
	{
		return _ptr->mNumProperties;
	}

	ModelMaterialProperty ModelMaterialPropertyContainer::create(size_t pos) const
	{
		return ModelMaterialProperty(_ptr->mProperties[pos]);
	}

	ModelMaterial::ModelMaterial(aiMaterial* ptr)
		: _ptr(ptr)
		, _properties(ptr)
		, _textures(ptr)
	{
	}

	std::string_view ModelMaterial::getName() const
	{
		return getStringView(_ptr->GetName());
	}

	const ModelTextureContainer& ModelMaterial::getTextures() const
	{
		return _textures;
	}

	const ModelMaterialPropertyContainer& ModelMaterial::getProperties() const
	{
		return _properties;
	}

	static VertexIndex* getModelIndex(unsigned int* indices, size_t pos)
	{
		if (indices == nullptr)
		{
			return nullptr;
		}
		return (VertexIndex*) & (indices[pos]);
	}

	ModelMeshFaceIndexContainer::ModelMeshFaceIndexContainer(aiFace* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelMeshFaceIndexContainer::size() const
	{
		return _ptr->mNumIndices;

	}

	const VertexIndex& ModelMeshFaceIndexContainer::operator[](size_t pos) const
	{
		return (VertexIndex&)_ptr->mIndices[pos];
	}

	VertexIndex& ModelMeshFaceIndexContainer::operator[](size_t pos)
	{
		return (VertexIndex&)_ptr->mIndices[pos];
	}

	ModelMeshFace::ModelMeshFace(aiFace* ptr)
		: _ptr(ptr)
		, _indices(ptr)
	{
	}

	size_t ModelMeshFace::size() const
	{
		if (_ptr == nullptr)
		{
			return 0;
		}
		return _ptr->mNumIndices;
	}

	bool ModelMeshFace::empty() const
	{
		return size() == 0;
	}

	const ModelMeshFaceIndexContainer& ModelMeshFace::getIndices() const
	{
		return _indices;
	}


	static glm::vec3* getModelVector3(aiVector3D* vertex, size_t pos)
	{
		if (vertex == nullptr)
		{
			return nullptr;
		}
		return (glm::vec3*)&(vertex[pos]);
	}

	ModelVector3Container::ModelVector3Container(aiVector3D* ptr, size_t size)
		: _ptr(ptr)
		, _size(size)
	{
	}

	size_t ModelVector3Container::size() const
	{
		return _ptr == nullptr ? 0 : _size;
	}

	const glm::vec3& ModelVector3Container::operator[](size_t pos) const
	{
		return (glm::vec3&)_ptr[pos];
	}

	glm::vec3& ModelVector3Container::operator[](size_t pos)
	{
		return (glm::vec3&)_ptr[pos];
	}

	ModelTextureCoords::ModelTextureCoords(aiMesh* mesh, size_t pos)
		: _compCount(mesh->mNumUVComponents[pos])
		, _coords(mesh->mTextureCoords[pos], mesh->mNumVertices)
	{
	}

	size_t ModelTextureCoords::getCompCount() const
	{
		return _compCount;
	}

	const ModelVector3Container& ModelTextureCoords::getCoords() const
	{
		return _coords;
	}

	ModelMeshFaceContainer::ModelMeshFaceContainer(aiMesh* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelMeshFaceContainer::size() const
	{
		return _ptr == nullptr || _ptr->mFaces == nullptr ? 0 : _ptr->mNumFaces;
	}

	ModelMeshFace ModelMeshFaceContainer::create(size_t pos) const
	{
		return ModelMeshFace(&_ptr->mFaces[pos]);
	}


	static ModelTextureCoords* getModelMeshTextureCoords(aiMesh* mesh, size_t pos)
	{
		return new ModelTextureCoords(mesh, pos);
	}

	ModelMeshTextureCoordsContainer::ModelMeshTextureCoordsContainer(aiMesh* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelMeshTextureCoordsContainer::size() const
	{
		for (size_t i = AI_MAX_NUMBER_OF_TEXTURECOORDS - 1; i >= 0; i--)
		{
			if (_ptr->mTextureCoords[i] != nullptr)
			{
				return i + 1;
			}
		}
		return 0;
	}

	ModelTextureCoords ModelMeshTextureCoordsContainer::create(size_t pos) const
	{
		return ModelTextureCoords(_ptr, pos);
	}

	ModelMesh::ModelMesh(aiMesh* ptr, const aiScene* scene)
		: _ptr(ptr)
		, _vertices(ptr->mVertices, ptr->mNumVertices)
		, _normals(ptr->mNormals, ptr->mNumVertices)
		, _tangents(ptr->mTangents, ptr->mNumVertices)
		, _bitangents(ptr->mBitangents, ptr->mNumVertices)
		, _material(scene->mMaterials[ptr->mMaterialIndex])
		, _faces(ptr)
		, _texCoords(ptr)
	{
	}

	const ModelMaterial& ModelMesh::getMaterial() const
	{
		return _material;
	}

	const ModelVector3Container& ModelMesh::getVertices() const
	{
		return _vertices;
	}

	const ModelVector3Container& ModelMesh::getNormals() const
	{
		return _normals;
	}

	const ModelVector3Container& ModelMesh::getTangents() const
	{
		return _tangents;
	}

	const ModelVector3Container& ModelMesh::getBitangents() const
	{
		return _bitangents;
	}

	const ModelMeshTextureCoordsContainer& ModelMesh::getTexCoords() const
	{
		return _texCoords;
	}

	const size_t ModelMesh::getVertexCount() const
	{
		return _ptr->mNumVertices;
	}

	const ModelMeshFaceContainer& ModelMesh::getFaces() const
	{
		return _faces;
	}

	ModelNode::ModelNode(aiNode* ptr, const aiScene* scene)
		: _ptr(ptr)
	{
		_meshes.reserve(ptr->mNumMeshes);
		for (size_t i = 0; i < ptr->mNumMeshes; i++)
		{
			auto m = ptr->mMeshes[i];
			_meshes.push_back(ModelMesh(scene->mMeshes[m], scene));
		}
		_children.reserve(ptr->mNumChildren);
		for (size_t i = 0; i < ptr->mNumChildren; i++)
		{
			_children.push_back(ModelNode(ptr->mChildren[i], scene));
		}
	}

	std::string_view ModelNode::getName() const
	{
		return getStringView(_ptr->mName);
	}

	const glm::mat4x4& ModelNode::getTransform() const
	{
		return (glm::mat4x4&)_ptr->mTransformation;
	}

	const std::vector<ModelMesh>& ModelNode::getMeshes() const
	{
		return _meshes;
	}

	const std::vector<ModelNode>& ModelNode::getChildren() const
	{
		return _children;
	}

	Model::Model(const aiScene* ptr)
		: _ptr(ptr)
		, _rootNode(ptr->mRootNode, ptr)
	{
	}

	std::string_view Model::getName() const
	{
		return getStringView(_ptr->mName);
	}

	const ModelNode& Model::getRootNode() const
	{
		return _rootNode;
	}
}
