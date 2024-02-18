
#include <darmok/model.hpp>
#include <darmok/asset.hpp>
#include "asset.hpp"

#include <assimp/vector3.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>

#include <bimg/decode.h>

#include <glm/ext/matrix_clip_space.hpp>

#include <filesystem>

namespace darmok
{
	Entity addModelNodeToScene(Scene& scene, ModelNode& node, Entity entity, const OptionalRef<Transform>& parent)
	{
		if (entity == 0)
		{
			entity = scene.createEntity();
		}
		auto& t = scene.addComponent<Transform>(entity, node.getTransform(), parent);

		auto& meshes = node.getMeshes();
		if (!meshes.empty())
		{
			scene.addComponent<MeshComponent>(entity, meshes.load());
		}
		for (auto& child : node.getChildren())
		{
			addModelNodeToScene(scene, child, 0, &t);
		}
		return entity;
	}

	Entity addModelToScene(Scene& scene, Model& model, Entity entity)
	{
		if (entity == 0)
		{
			entity = scene.createEntity();
		}
		return addModelNodeToScene(scene, model.getRootNode(), entity);
	}

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

	ModelMaterial::ModelMaterial(aiMaterial* ptr, const std::string& basePath, aiScene* scene)
		: _ptr(ptr)
		, _scene(scene)
		, _properties(ptr)
		, _basePath(basePath)
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

	MaterialPropertyCollection createMaterialPropertyCollection(const ModelMaterialPropertyCollection& model, int textureIndex)
	{
		MaterialPropertyCollection collection;
		for (auto& prop : model)
		{
			if (textureIndex < 0 && prop.getTextureType() != ModelMaterialTextureType::None)
			{
				continue;
			}
			else if (prop.getTextureIndex() != textureIndex)
			{
				continue;
			}
			collection.set(std::string(prop.getKey()), Data::copy(prop.getData()));
		}
		return collection;
	}

	uint8_t createColorValue(ai_real v)
	{
		return 255 * v;
	}

	Color createColor(const aiColor4D& c)
	{
		return Color
		{
			createColorValue(c.r),
			createColorValue(c.g),
			createColorValue(c.b),
			createColorValue(c.a),
		};
	}

	Color createColor(const aiColor3D& c)
	{
		return Color
		{
			createColorValue(c.r),
			createColorValue(c.g),
			createColorValue(c.b),
			255,
		};
	}

	std::optional<Color> ModelMaterial::getColor(ModelMaterialColorType type) const
	{
		aiColor4D color;
		aiReturn r = AI_FAILURE;
		switch (type)
		{
		case ModelMaterialColorType::Diffuse:
			r = _ptr->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			break;
		case ModelMaterialColorType::Specular:
			r = _ptr->Get(AI_MATKEY_COLOR_SPECULAR, color);
			break;
		case ModelMaterialColorType::Ambient:
			r = _ptr->Get(AI_MATKEY_COLOR_AMBIENT, color);
			break;
		case ModelMaterialColorType::Emissive:
			r = _ptr->Get(AI_MATKEY_COLOR_EMISSIVE, color);
			break;
		case ModelMaterialColorType::Transparent:
			r = _ptr->Get(AI_MATKEY_COLOR_TRANSPARENT, color);
			break;
		case ModelMaterialColorType::Reflective:
			r = _ptr->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			break;
		}
		if (r == AI_SUCCESS)
		{
			return createColor(color);
		}
		return std::nullopt;
	}

	bool ModelMaterial::showWireframe() const
	{
		int v;
		if (AI_SUCCESS == _ptr->Get(AI_MATKEY_ENABLE_WIREFRAME, v))
		{
			return v != 0;
		}
		return false;
	}

	ModelMaterialShadingModel ModelMaterial::getShadingModel() const
	{
		int v;
		if (AI_SUCCESS == _ptr->Get(AI_MATKEY_SHADING_MODEL, v))
		{
			return (ModelMaterialShadingModel)v;
		}
		return ModelMaterialShadingModel::Unknown;
	}

	float ModelMaterial::getOpacity() const
	{
		float v;
		if (AI_SUCCESS == _ptr->Get(AI_MATKEY_OPACITY, v))
		{
			return v != 0;
		}
		return 1.f;
	}

	static const auto _materialColors = std::unordered_map<ModelMaterialColorType, MaterialColorType>
	{
		{ ModelMaterialColorType::Diffuse, MaterialColorType::Diffuse },
		{ ModelMaterialColorType::Specular, MaterialColorType::Specular },
		{ ModelMaterialColorType::Ambient, MaterialColorType::Ambient },
		{ ModelMaterialColorType::Emissive, MaterialColorType::Emissive },
		{ ModelMaterialColorType::Transparent, MaterialColorType::Transparent },
		{ ModelMaterialColorType::Reflective, MaterialColorType::Reflective },
	};


	static const auto _materialTextures = std::unordered_map<ModelMaterialTextureType, MaterialTextureType>
	{
		{ ModelMaterialTextureType::Diffuse, MaterialTextureType::Diffuse },
		{ ModelMaterialTextureType::DiffuseRoughness, MaterialTextureType::Diffuse },
		{ ModelMaterialTextureType::Specular, MaterialTextureType::Specular },
		{ ModelMaterialTextureType::Normal, MaterialTextureType::Normal },
	};

	std::shared_ptr<Texture> loadEmbeddedTexture(aiScene* scene, const std::string& filePath)
	{
		auto data = scene->GetEmbeddedTexture(filePath.c_str());
		if (data == nullptr)
		{
			return nullptr;
		}
		auto format = bimg::TextureFormat::Count;
		uint32_t size = 0;
		if (data->mHeight == 0)
		{
			// compressed
			size = data->mWidth;
		}
		else
		{
			size = data->mWidth * data->mHeight * 4;
			format = bimg::TextureFormat::RGBA8;
		}
		auto alloc = AssetContext::get().getImpl().getAllocator();
		bx::Error err;
		auto container = bimg::imageParse(alloc, data->pcData, size, format, &err);
		checkError(err);
		if (container == nullptr)
		{
			throw std::runtime_error("got empty image container");
		}
		auto tex = std::make_shared<Texture>(std::make_shared<Image>(container), filePath);
		if (!isValid(tex->getHandle()))
		{
			throw std::runtime_error("could not load texture");
		}
		return tex;
	}

	MaterialTexture ModelMaterial::createMaterialTexture(const ModelMaterialTexture& modelTexture)
	{
		auto itr = _materialTextures.find(modelTexture.getType());
		if (itr == _materialTextures.end())
		{
			throw std::runtime_error("unsupported texture type");
		}

		auto path = modelTexture.getPath();
		std::shared_ptr<Texture> texture;
		if (_scene != nullptr)
		{
			texture = loadEmbeddedTexture(_scene, path);
		}
		if (texture == nullptr)
		{
			std::filesystem::path fsPath(path);
			if (!_basePath.empty() && fsPath.is_relative())
			{
				fsPath = std::filesystem::path(_basePath) / fsPath;
			}
			texture = AssetContext::get().getTextureLoader()(fsPath.string());
		}

		auto props = createMaterialPropertyCollection(getProperties(), modelTexture.getIndex());
		return MaterialTexture(texture, itr->second, std::move(props));
	}

	std::shared_ptr<Material> ModelMaterial::load()
	{
		if (_material != nullptr)
		{
			return _material;
		}
		std::vector<MaterialTexture> textures;
		for (auto& elm : _materialTextures)
		{
			for (auto& modelTex : getTextures(elm.first))
			{
				textures.push_back(createMaterialTexture(modelTex));
			}
		}
		
		auto props = createMaterialPropertyCollection(getProperties(), -1);
		_material = std::make_shared<Material>(std::move(textures), std::move(props));

		for (auto& elm : _materialColors)
		{
			if (auto v = getColor(elm.first))
			{
				_material->setColor(elm.second, v.value());
			}
		}
		return _material;
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

	ModelMesh::ModelMesh(aiMesh* ptr, ModelMaterial& material)
		: _ptr(ptr)
		, _vertices(ptr->mVertices, ptr->mNumVertices)
		, _normals(ptr->mNormals, ptr->mNumVertices)
		, _tangents(ptr->mTangents, ptr->mNumVertices)
		, _bitangents(ptr->mBitangents, ptr->mNumVertices)
		, _material(material)
		, _faces(ptr)
		, _texCoords(ptr)
	{
	}

	ModelMaterial& ModelMesh::getMaterial()
	{
		return _material;
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


	static bgfx::VertexLayout createModelMeshVertexLayout(const ModelMesh& modelMesh)
	{
		bgfx::VertexLayout layout;
		layout.begin();
		if (!modelMesh.getVertices().empty())
		{
			layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
		}
		if (!modelMesh.getNormals().empty())
		{
			layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
		}
		if (!modelMesh.getTangents().empty())
		{
			layout.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float);
		}
		if (!modelMesh.getBitangents().empty())
		{
			layout.add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float);
		}
		auto i = 0;
		for (auto& texCoords : modelMesh.getTexCoords())
		{
			auto attrib = bgfx::Attrib::TexCoord0 + i++;
			layout.add((bgfx::Attrib::Enum)attrib, texCoords.getCompCount(), bgfx::AttribType::Float);
		}
		layout.end();
		return layout;
	}

	std::vector<float> createModelMeshVertexData(const ModelMesh& modelMesh, const bgfx::VertexLayout& layout)
	{
		std::vector<float> vertices;
		vertices.reserve(layout.getSize(modelMesh.getVertexCount()) / sizeof(float));
		auto& verts = modelMesh.getVertices();
		auto& norms = modelMesh.getNormals();
		auto& tangs = modelMesh.getTangents();
		auto& btngs = modelMesh.getBitangents();
		for (size_t i = 0; i < modelMesh.getVertexCount(); i++)
		{
			if (!verts.empty())
			{
				auto& v = verts[i];
				vertices.insert(vertices.end(), { v.x, v.y, v.z });
			}
			if (!norms.empty())
			{
				auto& v = norms[i];
				vertices.insert(vertices.end(), { v.x, v.y, v.z });
			}
			if (!tangs.empty())
			{
				auto& v = tangs[i];
				vertices.insert(vertices.end(), { v.x, v.y, v.z });
			}
			if (!btngs.empty())
			{
				auto& v = btngs[i];
				vertices.insert(vertices.end(), { v.x, v.y, v.z });
			}

			for (auto& texCoords : modelMesh.getTexCoords())
			{
				auto& v = texCoords.getCoords()[i];
				switch (texCoords.getCompCount())
				{
				case 1:
					vertices.push_back(v.x);
					break;
				case 2:
					vertices.push_back(v.x);
					vertices.push_back(v.y);
					break;
				case 3:
					vertices.push_back(v.x);
					vertices.push_back(v.y);
					vertices.push_back(v.z);
					break;
				default:
					break;
				}
			}
		}
		return vertices;
	}

	std::vector<VertexIndex> createModelMeshIndexData(const ModelMesh& modelMesh)
	{
		std::vector<VertexIndex> indices;
		for (auto& face : modelMesh.getFaces())
		{
			indices.insert(indices.end(), face.getIndices().begin(), face.getIndices().end());
		}
		return indices;
	}

	std::shared_ptr<Mesh> ModelMesh::load()
	{
		if (_mesh != nullptr)
		{
			return _mesh;
		}
		auto layout = createModelMeshVertexLayout(*this);
		auto material = getMaterial().load();
		auto vertices = createModelMeshVertexData(*this, layout);
		auto indices = createModelMeshIndexData(*this);
		_mesh = std::make_shared<Mesh>(material, std::move(vertices), layout, std::move(indices));
		return _mesh;
	}

	ModelNodeMeshCollection::ModelNodeMeshCollection(aiNode* ptr, Model& model)
		: _ptr(ptr)
		, _model(model)
	{
	}
	
	size_t ModelNodeMeshCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumMeshes;
	}

	std::vector<std::shared_ptr<Mesh>> ModelNodeMeshCollection::load()
	{
		std::vector<std::shared_ptr<Mesh>> meshes;
		for (auto& mesh : *this)
		{
			meshes.push_back(mesh.load());
		}
		return meshes;
	}

	const ModelMesh& ModelNodeMeshCollection::operator[](size_t pos) const
	{
		auto m = _ptr->mMeshes[pos];
		return _model.getMeshes()[m];
	}

	ModelMesh& ModelNodeMeshCollection::operator[](size_t pos)
	{
		auto m = _ptr->mMeshes[pos];
		return _model.getMeshes()[m];
	}

	ModelCamera::ModelCamera(aiCamera* ptr)
		: _ptr(ptr)
	{
	}

	std::string_view ModelCamera::getName() const
	{
		return getStringView(_ptr->mName);
	}

	glm::mat4 ModelCamera::getProjection() const
	{
		return glm::perspective(getHorizontalFieldOfView(), getAspect(), getClipNear(), getClipFar());
	}

	const glm::mat4& ModelCamera::getTransform() const
	{
		_ptr->GetCameraMatrix((aiMatrix4x4&)_transform);
		return _transform;
	}

	float ModelCamera::getAspect() const
	{
		return _ptr->mAspect;
	}

	float ModelCamera::getClipFar() const
	{
		return _ptr->mAspect;
	}

	float ModelCamera::getClipNear() const
	{
		return _ptr->mClipPlaneFar;
	}

	float ModelCamera::getHorizontalFieldOfView() const
	{
		return _ptr->mHorizontalFOV;
	}

	float ModelCamera::getOrthographicWidth() const
	{
		return _ptr->mOrthographicWidth;
	}

	const glm::vec3& ModelCamera::getLookAt() const
	{
		return (glm::vec3&)_ptr->mLookAt;
	}

	const glm::vec3& ModelCamera::getPosition() const
	{
		return (glm::vec3&)_ptr->mPosition;
	}

	const glm::vec3& ModelCamera::getUp() const
	{
		return (glm::vec3&)_ptr->mUp;
	}

	ModelLight::ModelLight(aiLight* ptr)
		: _ptr(ptr)
	{
	}

	std::string_view ModelLight::getName() const
	{
		return getStringView(_ptr->mName);
	}

	float ModelLight::getInnerConeAngle() const
	{
		return _ptr->mAngleInnerCone;
	}

	float ModelLight::getOuterConeAngle() const
	{
		return _ptr->mAngleOuterCone;
	}

	float ModelLight::getAttenuation(ModelLightAttenuationType type) const
	{
		switch(type)
		{
			case ModelLightAttenuationType::Constant:
				return _ptr->mAttenuationConstant;
			case ModelLightAttenuationType::Linear:
				return _ptr->mAttenuationLinear;
			case ModelLightAttenuationType::Quadratic:
				return _ptr->mAttenuationQuadratic;
		}
		return 0.f;
	}

	Color ModelLight::getColor(ModelLightColorType type) const
	{
		switch (type)
		{
		case ModelLightColorType::Ambient:
			return createColor(_ptr->mColorAmbient);
		case ModelLightColorType::Diffuse:
			return createColor(_ptr->mColorDiffuse);
		case ModelLightColorType::Specular:
			return createColor(_ptr->mColorSpecular);
		}
		return Colors::white;
	}

	const glm::vec3& ModelLight::getDirection() const
	{
		return (glm::vec3&)_ptr->mDirection;
	}

	const glm::vec3& ModelLight::getPosition() const
	{
		return (glm::vec3&)_ptr->mPosition;
	}

	ModelLightType ModelLight::getType() const
	{
		return (ModelLightType)_ptr->mType;
	}

	ModelNodeChildrenCollection::ModelNodeChildrenCollection(aiNode* ptr, Model& model, const std::string& basePath)
		: _ptr(ptr)
		, _model(model)
		, _basePath(basePath)
	{
	}

	size_t ModelNodeChildrenCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumChildren;
	}

	ModelNode ModelNodeChildrenCollection::create(size_t pos) const
	{
		return ModelNode(_ptr->mChildren[pos], _model, _basePath);
	}

	ModelNode::ModelNode(aiNode* ptr, Model& model, const std::string& basePath)
		: _ptr(ptr)
		, _meshes(ptr, model)
		, _children(ptr, model, basePath)
		, _basePath(basePath)
	{
	}

	std::string_view ModelNode::getName() const
	{
		return getStringView(_ptr->mName);
	}

	const glm::mat4& ModelNode::getTransform() const
	{
		return (glm::mat4&)_ptr->mTransformation;
	}

	const ModelNodeMeshCollection& ModelNode::getMeshes() const
	{
		return _meshes;
	}

	const ModelNodeChildrenCollection& ModelNode::getChildren() const
	{
		return _children;
	}

	ModelNodeMeshCollection& ModelNode::getMeshes()
	{
		return _meshes;
	}

	ModelNodeChildrenCollection& ModelNode::getChildren()
	{
		return _children;
	}

	template<typename T>
	static OptionalRef<T> getModelNodeChild(T& node, const std::string& path)
	{
		auto itr = path.find('/');
		auto sep = itr != std::string::npos;
		auto name = sep ? path.substr(0, itr) : path;
		for (auto& child : node.getChildren())
		{
			if (child.getName() != name)
			{
				continue;
			}
			if (!sep)
			{
				return child;
			}
			return child.getChild(path.substr(itr + 1));
		}
		return std::nullopt;
	}


	OptionalRef<const ModelNode> ModelNode::getChild(const std::string& path) const
	{
		return getModelNodeChild(*this, path);
	}

	OptionalRef<ModelNode> ModelNode::getChild(const std::string& path)
	{
		return getModelNodeChild(*this, path);
	}

	ModelCameraCollection::ModelCameraCollection(const aiScene* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelCameraCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumCameras;
	}

	ModelCamera ModelCameraCollection::create(size_t pos) const
	{
		return ModelCamera(_ptr->mCameras[pos]);
	}

	OptionalRef<ModelCamera> ModelCameraCollection::get(const std::string& name)
	{
		for (auto& elm : *this)
		{
			if (elm.getName() == name)
			{
				return elm;
			}
		}
		return std::nullopt;
	}

	OptionalRef<const ModelCamera> ModelCameraCollection::get(const std::string& name) const
	{
		for (auto& elm : *this)
		{
			if (elm.getName() == name)
			{
				return elm;
			}
		}
		return std::nullopt;
	}

	ModelLightCollection::ModelLightCollection(const aiScene* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelLightCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumCameras;
	}

	ModelLight ModelLightCollection::create(size_t pos) const
	{
		return ModelLight(_ptr->mLights[pos]);
	}

	OptionalRef<ModelLight> ModelLightCollection::get(const std::string& name)
	{
		for (auto& elm : *this)
		{
			if (elm.getName() == name)
			{
				return elm;
			}
		}
		return std::nullopt;
	}

	OptionalRef<const ModelLight> ModelLightCollection::get(const std::string& name) const
	{
		for (auto& elm : *this)
		{
			if (elm.getName() == name)
			{
				return elm;
			}
		}
		return std::nullopt;
	}

	ModelMaterialCollection::ModelMaterialCollection(const aiScene* ptr, const std::string& basePath)
		: _ptr(ptr)
		, _basePath(basePath)
	{
	}

	size_t ModelMaterialCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumMaterials;
	}

	ModelMaterial ModelMaterialCollection::create(size_t pos) const
	{
		return ModelMaterial(_ptr->mMaterials[pos], _basePath);
	}

	ModelMeshCollection::ModelMeshCollection(const aiScene* ptr, ModelMaterialCollection& materials)
		: _ptr(ptr)
		, _materials(materials)
	{
	}

	size_t ModelMeshCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumMeshes;
	}

	ModelMesh ModelMeshCollection::create(size_t pos) const
	{
		auto ptr = _ptr->mMeshes[pos];
		auto& material = _materials[ptr->mMaterialIndex];
		return ModelMesh(ptr, material);
	}

	Model::Model(const aiScene* ptr, const std::string& path)
		: _ptr(ptr)
		, _basePath(std::filesystem::path(path).parent_path().string())
		, _rootNode(ptr->mRootNode, *this, _basePath)
		, _path(path)
		, _materials(ptr, _basePath)
		, _meshes(ptr, _materials)
		, _cameras(ptr)
		, _lights(ptr)
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

	const ModelMeshCollection& Model::getMeshes() const
	{
		return _meshes;
	}

	const ModelMaterialCollection& Model::getMaterials() const
	{
		return _materials;
	}

	const ModelCameraCollection& Model::getCameras() const
	{
		return _cameras;
	}

	const ModelLightCollection& Model::getLights() const
	{
		return _lights;
	}

	ModelNode& Model::getRootNode()
	{
		return _rootNode;
	}

	ModelMeshCollection& Model::getMeshes()
	{
		return _meshes;
	}

	ModelMaterialCollection& Model::getMaterials()
	{
		return _materials;
	}

	ModelCameraCollection& Model::getCameras()
	{
		return _cameras;
	}

	ModelLightCollection& Model::getLights()
	{
		return _lights;
	}
}
