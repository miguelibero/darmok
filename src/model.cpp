
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

	ModelMaterialProperty::ModelMaterialProperty(aiMaterialProperty* ptr)
		: _ptr(ptr)
	{
		if (_ptr != nullptr)
		{
			_data = Data(_ptr->mData, _ptr->mDataLength, false);
		}
	}

	std::string_view ModelMaterialProperty::getKey() const
	{
		return getStringView(_ptr->mKey);
	}

	ModelMaterialPropertyType ModelMaterialProperty::getType() const
	{
		return (ModelMaterialPropertyType)_ptr->mType;
	}

	ModelMaterialTextureType ModelMaterialProperty::getTextureType() const
	{
		return (ModelMaterialTextureType)_ptr->mSemantic;
	}

	size_t ModelMaterialProperty::getTextureIndex() const
	{
		return _ptr->mIndex;
	}

	const Data& ModelMaterialProperty::getData() const
	{
		return _data;
	}

	ModelMaterialPropertyCollection::ModelMaterialPropertyCollection(aiMaterial* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelMaterialPropertyCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumProperties;
	}

	ModelMaterialProperty ModelMaterialPropertyCollection::create(size_t pos) const
	{
		return ModelMaterialProperty(_ptr->mProperties[pos]);
	}

	ModelMaterialTexture::ModelMaterialTexture(aiMaterial* ptr, ModelMaterialTextureType type, unsigned int index)
		: _ptr(ptr)
		, _type(type)
		, _index(index)
		, _coordIndex(0)
		, _mapping(ModelMaterialTextureMapping::Other)
		, _blend(0)
		, _operation(ModelMaterialTextureOperation::Add)
		, _mapMode(ModelMaterialTextureMapMode::Clamp)
	{
		if (ptr == nullptr)
		{
			return;
		}
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

	ModelMaterialTextureType ModelMaterialTexture::getType() const
	{
		return _type;
	}

	size_t ModelMaterialTexture::getIndex() const
	{
		return _index;
	}

	const std::string& ModelMaterialTexture::getPath() const
	{
		return _path;
	}

	ModelMaterialTextureMapping ModelMaterialTexture::getMapping() const
	{
		return _mapping;
	}

	ModelMaterialTextureOperation ModelMaterialTexture::getOperation() const
	{
		return _operation;
	}

	ModelMaterialTextureMapMode ModelMaterialTexture::getMapMode() const
	{
		return _mapMode;
	}

	float ModelMaterialTexture::getBlend() const
	{
		return _blend;
	}

	size_t ModelMaterialTexture::getCoordIndex() const
	{
		return _coordIndex;
	}

	ModelMaterialTextureCollection::ModelMaterialTextureCollection(aiMaterial* ptr, ModelMaterialTextureType type)
		: _ptr(ptr)
		, _type(type)
	{
	}

	size_t ModelMaterialTextureCollection::size() const
	{
		return _ptr->GetTextureCount((aiTextureType)_type);
	}

	ModelMaterialTexture ModelMaterialTextureCollection::create(size_t pos) const
	{
		return ModelMaterialTexture(_ptr, _type, pos);
	}

	ModelMaterial::ModelMaterial(aiMaterial* ptr)
		: _ptr(ptr)
		, _properties(ptr)
	{
	}

	std::string_view ModelMaterial::getName() const
	{
		return getStringView(_ptr->GetName());
	}

	ModelMaterialTextureCollection ModelMaterial::getTextures(ModelMaterialTextureType type) const
	{
		return ModelMaterialTextureCollection(_ptr, type);
	}

	const ModelMaterialPropertyCollection& ModelMaterial::getProperties() const
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

	ModelMeshFaceIndexCollection::ModelMeshFaceIndexCollection(aiFace* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelMeshFaceIndexCollection::size() const
	{
		return _ptr->mNumIndices;
	}

	const VertexIndex& ModelMeshFaceIndexCollection::operator[](size_t pos) const
	{
		return (VertexIndex&)_ptr->mIndices[pos];
	}

	VertexIndex& ModelMeshFaceIndexCollection::operator[](size_t pos)
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

	const ModelMeshFaceIndexCollection& ModelMeshFace::getIndices() const
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

	ModelVector3Collection::ModelVector3Collection(aiVector3D* ptr, size_t size)
		: _ptr(ptr)
		, _size(size)
	{
	}

	size_t ModelVector3Collection::size() const
	{
		return _ptr == nullptr ? 0 : _size;
	}

	const glm::vec3& ModelVector3Collection::operator[](size_t pos) const
	{
		return (glm::vec3&)_ptr[pos];
	}

	glm::vec3& ModelVector3Collection::operator[](size_t pos)
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

	const ModelVector3Collection& ModelTextureCoords::getCoords() const
	{
		return _coords;
	}

	ModelMeshFaceCollection::ModelMeshFaceCollection(aiMesh* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelMeshFaceCollection::size() const
	{
		return _ptr == nullptr || _ptr->mFaces == nullptr ? 0 : _ptr->mNumFaces;
	}

	ModelMeshFace ModelMeshFaceCollection::create(size_t pos) const
	{
		return ModelMeshFace(&_ptr->mFaces[pos]);
	}


	static ModelTextureCoords* getModelMeshTextureCoords(aiMesh* mesh, size_t pos)
	{
		return new ModelTextureCoords(mesh, pos);
	}

	ModelMeshTextureCoordsCollection::ModelMeshTextureCoordsCollection(aiMesh* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelMeshTextureCoordsCollection::size() const
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

	ModelTextureCoords ModelMeshTextureCoordsCollection::create(size_t pos) const
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

	const ModelVector3Collection& ModelMesh::getVertices() const
	{
		return _vertices;
	}

	const ModelVector3Collection& ModelMesh::getNormals() const
	{
		return _normals;
	}

	const ModelVector3Collection& ModelMesh::getTangents() const
	{
		return _tangents;
	}

	const ModelVector3Collection& ModelMesh::getBitangents() const
	{
		return _bitangents;
	}

	const ModelMeshTextureCoordsCollection& ModelMesh::getTexCoords() const
	{
		return _texCoords;
	}

	const size_t ModelMesh::getVertexCount() const
	{
		return _ptr->mNumVertices;
	}

	const ModelMeshFaceCollection& ModelMesh::getFaces() const
	{
		return _faces;
	}

	ModelNodeMeshCollection::ModelNodeMeshCollection(aiNode* ptr, const aiScene* scene)
		: _ptr(ptr)
		, _scene(scene)
	{
	}
	
	size_t ModelNodeMeshCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumMeshes;
	}

	ModelMesh ModelNodeMeshCollection::create(size_t pos) const
	{
		auto m = _ptr->mMeshes[pos];
		return ModelMesh(_scene->mMeshes[m], _scene);
	}

	ModelNodeChildrenCollection::ModelNodeChildrenCollection(aiNode* ptr, const aiScene* scene)
		: _ptr(ptr)
		, _scene(scene)
	{
	}

	size_t ModelNodeChildrenCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumChildren;
	}

	ModelNode ModelNodeChildrenCollection::create(size_t pos) const
	{
		return ModelNode(_ptr->mChildren[pos], _scene);
	}

	ModelNode::ModelNode(aiNode* ptr, const aiScene* scene)
		: _ptr(ptr)
		, _meshes(ptr, scene)
		, _children(ptr, scene)
	{
	}

	std::string_view ModelNode::getName() const
	{
		return getStringView(_ptr->mName);
	}

	const glm::mat4x4& ModelNode::getTransform() const
	{
		return (glm::mat4x4&)_ptr->mTransformation;
	}

	const ModelNodeMeshCollection& ModelNode::getMeshes() const
	{
		return _meshes;
	}

	const ModelNodeChildrenCollection& ModelNode::getChildren() const
	{
		return _children;
	}

	Model::Model(const aiScene* ptr, const std::string& path)
		: _ptr(ptr)
		, _rootNode(ptr->mRootNode, ptr)
		, _path(path)
	{
	}

	std::string_view Model::getName() const
	{
		return getStringView(_ptr->mName);
	}

	const std::string& Model::getPath() const
	{
		return _path;
	}

	const ModelNode& Model::getRootNode() const
	{
		return _rootNode;
	}
}
