
#include "model.hpp"
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/vertex.hpp>

#include <assimp/vector3.h>
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <bx/math.h>
#include <bimg/decode.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include <filesystem>

namespace darmok
{
	static inline std::string_view getStringView(const aiString& str) noexcept
	{
		return std::string_view(str.data, str.length);
	}

	static inline glm::mat4 convertMatrix(const aiMatrix4x4& from) noexcept
	{
		// the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		return glm::mat4(
			from.a1, from.b1, from.c1, from.d1,
			from.a2, from.b2, from.c2, from.d2,
			from.a3, from.b3, from.c3, from.d3,
			from.a4, from.b4, from.c4, from.d4
		);
	}

	static inline glm::vec3 convertVector(const aiVector3D& vec) noexcept
	{
		return glm::vec3(vec.x, vec.y, vec.z);
	}

	static inline glm::vec2 convertVector(const aiVector2D& vec) noexcept
	{
		return glm::vec2(vec.x, vec.y);
	}

	static inline uint8_t convertColorComp(ai_real v) noexcept
	{
		return 255 * v;
	}

	Color convertColor(const aiColor4D& c) noexcept
	{
		return Color
		{
			convertColorComp(c.r),
			convertColorComp(c.g),
			convertColorComp(c.b),
			convertColorComp(c.a),
		};
	}

	Color3 convertColor(const aiColor3D& c) noexcept
	{
		return Color3
		{
			convertColorComp(c.r),
			convertColorComp(c.g),
			convertColorComp(c.b)
		};
	}


	ModelMaterialProperty::ModelMaterialProperty(aiMaterialProperty* ptr)
		: _ptr(ptr)
	{
		if (_ptr != nullptr)
		{
			_data = DataView(_ptr->mData, _ptr->mDataLength);
		}
	}

	std::string_view ModelMaterialProperty::getKey() const noexcept
	{
		return getStringView(_ptr->mKey);
	}

	ModelMaterialPropertyType ModelMaterialProperty::getType() const noexcept
	{
		return (ModelMaterialPropertyType)_ptr->mType;
	}

	ModelMaterialTextureType ModelMaterialProperty::getTextureType() const noexcept
	{
		return (ModelMaterialTextureType)_ptr->mSemantic;
	}

	size_t ModelMaterialProperty::getTextureIndex() const noexcept
	{
		return _ptr->mIndex;
	}

	const DataView& ModelMaterialProperty::getData() const noexcept
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

	ModelMaterial::ModelMaterial(aiMaterial* ptr, const aiScene* scene, const std::string& basePath, const OptionalRef<ITextureLoader>& textureLoader, bx::AllocatorI* alloc)
		: _ptr(ptr)
		, _scene(scene)
		, _properties(ptr)
		, _basePath(basePath)
		, _textureLoader(textureLoader)
		, _alloc(alloc)
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
			return convertColor(color);
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

	static std::shared_ptr<Texture> loadEmbeddedTexture(const aiScene* scene, const char* filePath, bx::AllocatorI* alloc = nullptr)
	{
		if (scene == nullptr)
		{
			return nullptr;
		}
		auto data = scene->GetEmbeddedTexture(filePath);
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
		bx::Error err;
		auto container = bimg::imageParse(alloc, data->pcData, size, format, &err);
		checkError(err);
		if (container == nullptr)
		{
			throw std::runtime_error("got empty image container");
		}
		auto tex = Texture::create(std::make_shared<Image>(container));
		if (tex == nullptr)
		{
			throw std::runtime_error("could not load texture");
		}
		tex->setName(filePath);
		return tex;
	}

	std::pair<MaterialTextureType, std::shared_ptr<Texture>> ModelMaterial::createMaterialTexture(const ModelMaterialTexture& modelTexture) const
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
			texture = loadEmbeddedTexture(_scene, path.c_str(), _alloc);
		}
		if (texture == nullptr && _textureLoader.hasValue())
		{
			std::filesystem::path fsPath(path);
			if (!_basePath.empty() && fsPath.is_relative())
			{
				fsPath = std::filesystem::path(_basePath) / fsPath;
			}
			texture = _textureLoader.value()(fsPath.string());
		}
		return std::make_pair(itr->second, texture);
	}

	std::shared_ptr<Material> ModelMaterial::load() noexcept
	{
		auto material = std::make_shared<Material>();
		for (auto& elm : _materialTextures)
		{
			for (auto& modelTex : getTextures(elm.first))
			{
				auto pair = createMaterialTexture(modelTex);
				if (pair.second != nullptr)
				{
					material->setTexture(pair.first, pair.second);
				}
			}
		}
		for (auto& elm : _materialColors)
		{
			if (auto v = getColor(elm.first))
			{
				material->setColor(elm.second, v.value());
			}
		}
		return material;
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

	ModelVector3Collection::ModelVector3Collection(aiVector3D* ptr, size_t size)
		: _ptr(ptr)
		, _size(size)
	{
	}

	size_t ModelVector3Collection::size() const
	{
		return _ptr == nullptr ? 0 : _size;
	}

	glm::vec3 ModelVector3Collection::create(size_t pos) const
	{
		return convertVector(_ptr[pos]);
	}

	ModelColorCollection::ModelColorCollection(aiColor4D* ptr, size_t size)
		: _ptr(ptr)
		, _size(size)
	{
	}

	Color ModelColorCollection::create(size_t pos) const
	{
		return convertColor(_ptr[pos]);
	}

	size_t ModelColorCollection::size() const
	{
		return _size;
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

	ModelMeshTextureCoordsCollection::ModelMeshTextureCoordsCollection(aiMesh* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelMeshTextureCoordsCollection::size() const
	{
		size_t count = 0;
		for (size_t i = 0; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
		{
			if (_ptr->mTextureCoords[i] != nullptr)
			{
				count++;
			}
		}
		return count;
	}

	ModelTextureCoords ModelMeshTextureCoordsCollection::create(size_t pos) const
	{
		size_t i = 0;
		size_t count = 0;
		for (; i < AI_MAX_NUMBER_OF_TEXTURECOORDS; i++)
		{
			if (_ptr->mTextureCoords[i] != nullptr)
			{
				if (count++ == pos)
				{
					break;
				}
			}
		}
		return ModelTextureCoords(_ptr, i);
	}

	ModelMeshColorsCollection::ModelMeshColorsCollection(aiMesh* ptr)
		: _ptr(ptr)
	{
	}

	size_t ModelMeshColorsCollection::size() const
	{
		size_t count = 0;
		for (size_t i = 0; i < AI_MAX_NUMBER_OF_COLOR_SETS; i++)
		{
			if (_ptr->mColors[i] != nullptr)
			{
				count++;
			}
		}
		return count;
	}

	ModelColorCollection ModelMeshColorsCollection::create(size_t pos) const
	{
		size_t i = 0;
		size_t count = 0;
		for (; i < AI_MAX_NUMBER_OF_COLOR_SETS; i++)
		{
			if (_ptr->mColors[i] != nullptr)
			{
				if (count++ == pos)
				{
					break;
				}
			}
		}
		return ModelColorCollection(_ptr->mColors[i], _ptr->mNumVertices);
	}

	ModelMesh::ModelMesh(aiMesh* ptr, ModelMaterial& material)
		: _ptr(ptr)
		, _positions(ptr->mVertices, ptr->mNumVertices)
		, _normals(ptr->mNormals, ptr->mNumVertices)
		, _tangents(ptr->mTangents, ptr->mNumVertices)
		, _bitangents(ptr->mBitangents, ptr->mNumVertices)
		, _material(material)
		, _faces(ptr)
		, _texCoords(ptr)
		, _colors(ptr)
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

	const ModelVector3Collection& ModelMesh::getPositions() const
	{
		return _positions;
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

	const ModelMeshColorsCollection& ModelMesh::getColors() const
	{
		return _colors;
	}

	static Data createModelMeshVertexData(const ModelMesh& modelMesh, const bgfx::VertexLayout& layout) noexcept
	{
		VertexDataWriter writer(layout, modelMesh.getVertexCount());
		writer.write(bgfx::Attrib::Position, modelMesh.getPositions());
		writer.write(bgfx::Attrib::Normal, modelMesh.getNormals());
		writer.write(bgfx::Attrib::Tangent, modelMesh.getTangents());
		writer.write(bgfx::Attrib::Bitangent, modelMesh.getBitangents());

		{
			auto i = 0;
			for (auto& elm : modelMesh.getTexCoords())
			{
				auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::TexCoord0 + i++);
				writer.write(attrib, elm.getCoords());
			}
		}
		{
			auto i = 0;
			for (auto& elm : modelMesh.getColors())
			{
				auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::Color0 + i++);
				writer.write(attrib, elm);
			}
		}
		return writer.finish();
	}

	Data createModelMeshIndexData(const ModelMesh& modelMesh)
	{
		std::vector<VertexIndex> indices;
		for (auto& face : modelMesh.getFaces())
		{
			indices.insert(indices.end(), face.getIndices().begin(), face.getIndices().end());
		}
		return Data::copy(indices);
	}

	std::shared_ptr<Mesh> ModelMesh::load(const bgfx::VertexLayout& layout)
	{
		auto material = getMaterial().load();
		auto vertices = createModelMeshVertexData(*this, layout);
		auto indices = createModelMeshIndexData(*this);

		auto ptr = (float*)vertices.ptr();
		return std::make_shared<Mesh>(layout, std::move(vertices), std::move(indices), material);
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

	std::vector<std::shared_ptr<Mesh>> ModelNodeMeshCollection::load(const bgfx::VertexLayout& layout)
	{
		std::vector<std::shared_ptr<Mesh>> meshes;
		for (auto& mesh : *this)
		{
			meshes.push_back(mesh.load(layout));
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
		aiMatrix4x4 mat;
		_ptr->GetCameraMatrix(mat);
		_view = convertMatrix(mat);

		auto aspect = getAspect();
		auto fovy = 0.f;
		if (aspect != 0.f)
		{
			auto fovx = getHorizontalFieldOfView();
			fovy = 2.f * atan(tan(0.5f * fovx) * aspect);
		}

		auto clipNear = getClipNear();
		auto clipFar = getClipFar();
		bx::mtxProj(glm::value_ptr(_proj), bx::toDeg(fovy), aspect, clipNear, clipFar, bgfx::getCaps()->homogeneousDepth);
	}

	std::string_view ModelCamera::getName() const
	{
		return getStringView(_ptr->mName);
	}

	const glm::mat4& ModelCamera::getProjectionMatrix() const
	{
		return _proj;
	}

	const glm::mat4& ModelCamera::getViewMatrix() const
	{
		return _view;
	}

	float ModelCamera::getAspect() const
	{
		return _ptr->mAspect;
	}

	float ModelCamera::getClipFar() const
	{
		return _ptr->mClipPlaneFar;
	}

	float ModelCamera::getClipNear() const
	{
		return _ptr->mClipPlaneNear;
	}

	float ModelCamera::getHorizontalFieldOfView() const
	{
		return _ptr->mHorizontalFOV;
	}

	float ModelCamera::getOrthographicWidth() const
	{
		return _ptr->mOrthographicWidth;
	}

	glm::vec3 ModelCamera::getLookAt() const
	{
		return convertVector(_ptr->mLookAt);
	}

	glm::vec3 ModelCamera::getPosition() const
	{
		return convertVector(_ptr->mPosition);
	}

	glm::vec3 ModelCamera::getUp() const
	{
		return convertVector(_ptr->mUp);
	}

	ModelLight::ModelLight(aiLight* ptr) noexcept
		: _ptr(ptr)
	{
	}

	void ModelLight::addToScene(Scene& scene, Entity entity) noexcept
	{
		auto& registry = scene.getRegistry();
		switch (getType())
		{
		case ModelLightType::Point:
		{
			registry.emplace<PointLight>(entity)
				.setAttenuation(getAttenuation())
				.setDiffuseColor(getColor(ModelLightColorType::Diffuse))
				.setSpecularColor(getColor(ModelLightColorType::Specular))
				;
			auto ambient = getColor(ModelLightColorType::Ambient);
			if (ambient != Colors::black3())
			{
				registry.emplace<AmbientLight>(entity)
					.setColor(ambient)
					;
			}
			break;
		}
		case ModelLightType::Ambient:
			registry.emplace<AmbientLight>(entity)
				.setColor(getColor(ModelLightColorType::Ambient))
				;
			break;
		}
	}

	std::string_view ModelLight::getName() const noexcept
	{
		return getStringView(_ptr->mName);
	}

	float ModelLight::getInnerConeAngle() const noexcept
	{
		return _ptr->mAngleInnerCone;
	}

	float ModelLight::getOuterConeAngle() const noexcept
	{
		return _ptr->mAngleOuterCone;
	}

	glm::vec3 ModelLight::getAttenuation() const noexcept
	{
		return {
			_ptr->mAttenuationConstant,
			_ptr->mAttenuationLinear,
			_ptr->mAttenuationQuadratic,
		};
	}

	Color3 ModelLight::getColor(ModelLightColorType type) const noexcept
	{
		switch (type)
		{
		case ModelLightColorType::Ambient:
			return convertColor(_ptr->mColorAmbient);
		case ModelLightColorType::Diffuse:
			return convertColor(_ptr->mColorDiffuse);
		case ModelLightColorType::Specular:
			return convertColor(_ptr->mColorSpecular);
		}
		return Colors::white3();
	}

	glm::vec3 ModelLight::getDirection() const noexcept
	{
		return convertVector(_ptr->mDirection);
	}

	glm::vec3 ModelLight::getPosition() const noexcept
	{
		return convertVector(_ptr->mPosition);
	}

	ModelLightType ModelLight::getType() const noexcept
	{
		return (ModelLightType)_ptr->mType;
	}

	glm::vec2 ModelLight::getSize() const noexcept
	{
		return convertVector(_ptr->mSize);
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
		, _model(model)
	{
	}

	std::string_view ModelNode::getName() const
	{
		return getStringView(_ptr->mName);
	}

	glm::mat4 ModelNode::getTransform() const
	{
		return convertMatrix(_ptr->mTransformation);
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
	static OptionalRef<T> getModelNodeChild(T& node, const std::string_view& path)
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

	OptionalRef<const ModelNode> ModelNode::getChild(const std::string_view& path) const
	{
		return getModelNodeChild(*this, path);
	}

	OptionalRef<const ModelCamera> ModelNode::getCamera() const
	{
		return ((const Model&)_model).getCameras().get(getName());
	}

	OptionalRef<const ModelLight> ModelNode::getLight() const
	{
		return ((const Model&)_model).getLights().get(getName());
	}

	OptionalRef<ModelNode> ModelNode::getChild(const std::string_view& path)
	{
		return getModelNodeChild(*this, path);
	}

	OptionalRef<ModelCamera> ModelNode::getCamera()
	{
		return _model.getCameras().get(getName());
	}

	OptionalRef<ModelLight> ModelNode::getLight()
	{
		return _model.getLights().get(getName());
	}

	Entity ModelNode::addToScene(Scene& scene, const bgfx::VertexLayout& layout, Entity parent)
	{
		auto entity = doAddToScene(scene, layout, parent);
		for (auto& child : getChildren())
		{
			addToScene(scene, layout, entity);
		}
		return entity;
	}

	Entity ModelNode::doAddToScene(Scene& scene, const bgfx::VertexLayout& layout, Entity parent)
	{
		auto entity = scene.getRegistry().create();
		auto transMat = getTransform();
		auto& registry = scene.getRegistry();

		auto cam = getCamera();
		if (cam.hasValue())
		{
			transMat *= cam->getViewMatrix();
			registry.emplace<Camera>(entity, cam->getProjectionMatrix());
		}

		auto light = getLight();
		if (light.hasValue())
		{
			transMat = glm::translate(transMat, light->getPosition());
			light->addToScene(scene, entity);
		}

		auto& meshes = getMeshes();
		if (!meshes.empty())
		{
			registry.emplace<MeshComponent>(entity, meshes.load(layout));
		}

		auto parentTrans = registry.try_get<Transform>(parent);
		registry.emplace<Transform>(entity, transMat, parentTrans);
		return entity;
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

	OptionalRef<ModelCamera> ModelCameraCollection::get(const std::string_view& name)
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

	OptionalRef<const ModelCamera> ModelCameraCollection::get(const std::string_view& name) const
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

	OptionalRef<ModelLight> ModelLightCollection::get(const std::string_view& name)
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

	OptionalRef<const ModelLight> ModelLightCollection::get(const std::string_view& name) const
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

	ModelMaterialCollection::ModelMaterialCollection(const aiScene* ptr, const std::string& basePath,
		const OptionalRef<ITextureLoader>& textureLoader, bx::AllocatorI* alloc)
		: _ptr(ptr)
		, _basePath(basePath)
		, _textureLoader(textureLoader)
		, _alloc(alloc)
	{
	}

	size_t ModelMaterialCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumMaterials;
	}

	ModelMaterial ModelMaterialCollection::create(size_t pos) const
	{
		return ModelMaterial(_ptr->mMaterials[pos], _ptr, _basePath, _textureLoader, _alloc);
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

	Model::Model(const aiScene* ptr, const std::string& path, const OptionalRef<ITextureLoader>& textureLoader, bx::AllocatorI* alloc)
		: _ptr(ptr)
		, _basePath(std::filesystem::path(path).parent_path().string())
		, _rootNode(ptr->mRootNode, *this, _basePath)
		, _path(path)
		, _materials(ptr, _basePath, textureLoader, alloc)
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

	Entity Model::addToScene(Scene& scene, const bgfx::VertexLayout& layout, Entity parent)
	{
		return getRootNode().addToScene(scene, layout, parent);
	}

	AssimpModelLoader::AssimpModelLoader(IDataLoader& dataLoader, const OptionalRef<ITextureLoader>& textureLoader, bx::AllocatorI* alloc)
		: _dataLoader(dataLoader)
		, _textureLoader(textureLoader)
		, _alloc(alloc)
	{
	}

	std::shared_ptr<Model> AssimpModelLoader::operator()(std::string_view name)
	{
		auto data = _dataLoader(name);
		if (data == nullptr || data->empty())
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
		auto scene = _importer.ReadFileFromMemory(data->ptr(), data->size(), flags, nameString.c_str());

		if (scene == nullptr)
		{
			throw std::runtime_error(_importer.GetErrorString());
		}

		return std::make_shared<Model>(scene, nameString, _textureLoader, _alloc);
	}
}
