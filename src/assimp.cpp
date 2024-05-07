
#include "assimp.hpp"

#include <assimp/vector3.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <bx/math.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>

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


	AssimpMaterialProperty::AssimpMaterialProperty(aiMaterialProperty* ptr)
		: _ptr(ptr)
	{
		if (_ptr != nullptr)
		{
			_data = DataView(_ptr->mData, _ptr->mDataLength);
		}
	}

	std::string_view AssimpMaterialProperty::getKey() const noexcept
	{
		return getStringView(_ptr->mKey);
	}

	aiPropertyTypeInfo AssimpMaterialProperty::getType() const noexcept
	{
		return _ptr->mType;
	}

	aiTextureType AssimpMaterialProperty::getTextureType() const noexcept
	{
		return (aiTextureType)_ptr->mSemantic;
	}

	size_t AssimpMaterialProperty::getTextureIndex() const noexcept
	{
		return _ptr->mIndex;
	}

	const DataView& AssimpMaterialProperty::getData() const noexcept
	{
		return _data;
	}

	AssimpMaterialPropertyCollection::AssimpMaterialPropertyCollection(aiMaterial* ptr)
		: _ptr(ptr)
	{
	}

	size_t AssimpMaterialPropertyCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumProperties;
	}

	AssimpMaterialProperty AssimpMaterialPropertyCollection::create(size_t pos) const
	{
		return AssimpMaterialProperty(_ptr->mProperties[pos]);
	}

	AssimpMaterialTexture::AssimpMaterialTexture(aiMaterial* ptr, aiTextureType type, unsigned int index)
		: _ptr(ptr)
		, _type(type)
		, _index(index)
		, _coordIndex(0)
		, _mapping(aiTextureMapping_OTHER)
		, _blend(0)
		, _operation(aiTextureOp_Add)
		, _mapMode(aiTextureMapMode_Clamp)
	{
		if (ptr == nullptr)
		{
			return;
		}
		aiString path;
		unsigned int uvindex;
		ptr->GetTexture((aiTextureType)type, index, &path,
			&_mapping,
			&uvindex,
			&_blend,
			& _operation,
			&_mapMode
		);
		_coordIndex = uvindex;
		_path = path.C_Str();
	}

	aiTextureType AssimpMaterialTexture::getType() const
	{
		return _type;
	}

	size_t AssimpMaterialTexture::getIndex() const
	{
		return _index;
	}

	const std::string& AssimpMaterialTexture::getPath() const
	{
		return _path;
	}

	aiTextureMapping AssimpMaterialTexture::getMapping() const
	{
		return _mapping;
	}

	aiTextureOp AssimpMaterialTexture::getOperation() const
	{
		return _operation;
	}

	aiTextureMapMode AssimpMaterialTexture::getMapMode() const
	{
		return _mapMode;
	}

	float AssimpMaterialTexture::getBlend() const
	{
		return _blend;
	}

	size_t AssimpMaterialTexture::getCoordIndex() const
	{
		return _coordIndex;
	}

	AssimpMaterialTextureCollection::AssimpMaterialTextureCollection(aiMaterial* ptr, aiTextureType type)
		: _ptr(ptr)
		, _type(type)
	{
	}

	size_t AssimpMaterialTextureCollection::size() const
	{
		return _ptr->GetTextureCount((aiTextureType)_type);
	}

	AssimpMaterialTexture AssimpMaterialTextureCollection::create(size_t pos) const
	{
		return AssimpMaterialTexture(_ptr, _type, pos);
	}

	AssimpMaterial::AssimpMaterial(aiMaterial* ptr)
		: _ptr(ptr)
		, _properties(ptr)
	{
	}

	std::string_view AssimpMaterial::getName() const
	{
		return getStringView(_ptr->GetName());
	}

	AssimpMaterialTextureCollection AssimpMaterial::getTextures(aiTextureType type) const
	{
		return AssimpMaterialTextureCollection(_ptr, type);
	}

	const AssimpMaterialPropertyCollection& AssimpMaterial::getProperties() const
	{
		return _properties;
	}

	std::optional<Color> AssimpMaterial::getColor(AssimpMaterialColorType type) const
	{
		aiColor4D color;
		aiReturn r = AI_FAILURE;
		switch (type)
		{
		case AssimpMaterialColorType::Diffuse:
			r = _ptr->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			break;
		case AssimpMaterialColorType::Specular:
			r = _ptr->Get(AI_MATKEY_COLOR_SPECULAR, color);
			break;
		case AssimpMaterialColorType::Ambient:
			r = _ptr->Get(AI_MATKEY_COLOR_AMBIENT, color);
			break;
		case AssimpMaterialColorType::Emissive:
			r = _ptr->Get(AI_MATKEY_COLOR_EMISSIVE, color);
			break;
		case AssimpMaterialColorType::Transparent:
			r = _ptr->Get(AI_MATKEY_COLOR_TRANSPARENT, color);
			break;
		case AssimpMaterialColorType::Reflective:
			r = _ptr->Get(AI_MATKEY_COLOR_REFLECTIVE, color);
			break;
		}
		if (r == AI_SUCCESS)
		{
			return convertColor(color);
		}
		return std::nullopt;
	}

	bool AssimpMaterial::showWireframe() const
	{
		int v;
		if (AI_SUCCESS == _ptr->Get(AI_MATKEY_ENABLE_WIREFRAME, v))
		{
			return v != 0;
		}
		return false;
	}

	aiShadingMode AssimpMaterial::getShadingMode() const
	{
		int v;
		if (AI_SUCCESS == _ptr->Get(AI_MATKEY_SHADING_MODEL, v))
		{
			return (aiShadingMode)v;
		}
		return aiShadingMode_NoShading;
	}

	float AssimpMaterial::getOpacity() const
	{
		float v;
		if (AI_SUCCESS == _ptr->Get(AI_MATKEY_OPACITY, v))
		{
			return v != 0;
		}
		return 1.f;
	}

	static VertexIndex* getAssimpIndex(unsigned int* indices, size_t pos)
	{
		if (indices == nullptr)
		{
			return nullptr;
		}
		return (VertexIndex*) & (indices[pos]);
	}

	AssimpMeshFaceIndexCollection::AssimpMeshFaceIndexCollection(aiFace* ptr)
		: _ptr(ptr)
	{
	}

	size_t AssimpMeshFaceIndexCollection::size() const
	{
		return _ptr->mNumIndices;
	}

	const VertexIndex& AssimpMeshFaceIndexCollection::operator[](size_t pos) const
	{
		return (VertexIndex&)_ptr->mIndices[pos];
	}

	VertexIndex& AssimpMeshFaceIndexCollection::operator[](size_t pos)
	{
		return (VertexIndex&)_ptr->mIndices[pos];
	}

	AssimpMeshFace::AssimpMeshFace(aiFace* ptr)
		: _ptr(ptr)
		, _indices(ptr)
	{
	}

	size_t AssimpMeshFace::size() const
	{
		if (_ptr == nullptr)
		{
			return 0;
		}
		return _ptr->mNumIndices;
	}

	bool AssimpMeshFace::empty() const
	{
		return size() == 0;
	}

	const AssimpMeshFaceIndexCollection& AssimpMeshFace::getIndices() const
	{
		return _indices;
	}

	AssimpVector3Collection::AssimpVector3Collection(aiVector3D* ptr, size_t size)
		: _ptr(ptr)
		, _size(size)
	{
	}

	size_t AssimpVector3Collection::size() const
	{
		return _ptr == nullptr ? 0 : _size;
	}

	glm::vec3 AssimpVector3Collection::create(size_t pos) const
	{
		return convertVector(_ptr[pos]);
	}

	AssimpColorCollection::AssimpColorCollection(aiColor4D* ptr, size_t size)
		: _ptr(ptr)
		, _size(size)
	{
	}

	Color AssimpColorCollection::create(size_t pos) const
	{
		return convertColor(_ptr[pos]);
	}

	size_t AssimpColorCollection::size() const
	{
		return _size;
	}

	AssimpTextureCoords::AssimpTextureCoords(aiMesh* mesh, size_t pos)
		: _compCount(mesh->mNumUVComponents[pos])
		, _coords(mesh->mTextureCoords[pos], mesh->mNumVertices)
	{
	}

	size_t AssimpTextureCoords::getCompCount() const
	{
		return _compCount;
	}

	const AssimpVector3Collection& AssimpTextureCoords::getCoords() const
	{
		return _coords;
	}

	AssimpMeshFaceCollection::AssimpMeshFaceCollection(aiMesh* ptr)
		: _ptr(ptr)
	{
	}

	size_t AssimpMeshFaceCollection::size() const
	{
		return _ptr == nullptr || _ptr->mFaces == nullptr ? 0 : _ptr->mNumFaces;
	}

	AssimpMeshFace AssimpMeshFaceCollection::create(size_t pos) const
	{
		return AssimpMeshFace(&_ptr->mFaces[pos]);
	}

	AssimpMeshTextureCoordsCollection::AssimpMeshTextureCoordsCollection(aiMesh* ptr)
		: _ptr(ptr)
	{
	}

	size_t AssimpMeshTextureCoordsCollection::size() const
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

	AssimpTextureCoords AssimpMeshTextureCoordsCollection::create(size_t pos) const
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
		return AssimpTextureCoords(_ptr, i);
	}

	AssimpMeshColorsCollection::AssimpMeshColorsCollection(aiMesh* ptr)
		: _ptr(ptr)
	{
	}

	size_t AssimpMeshColorsCollection::size() const
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

	AssimpColorCollection AssimpMeshColorsCollection::create(size_t pos) const
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
		return AssimpColorCollection(_ptr->mColors[i], _ptr->mNumVertices);
	}

	AssimpMesh::AssimpMesh(aiMesh* ptr, AssimpMaterial& material)
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

	AssimpMaterial& AssimpMesh::getMaterial()
	{
		return _material;
	}

	const AssimpMaterial& AssimpMesh::getMaterial() const
	{
		return _material;
	}

	const AssimpVector3Collection& AssimpMesh::getPositions() const
	{
		return _positions;
	}

	const AssimpVector3Collection& AssimpMesh::getNormals() const
	{
		return _normals;
	}

	const AssimpVector3Collection& AssimpMesh::getTangents() const
	{
		return _tangents;
	}

	const AssimpVector3Collection& AssimpMesh::getBitangents() const
	{
		return _bitangents;
	}

	const AssimpMeshTextureCoordsCollection& AssimpMesh::getTexCoords() const
	{
		return _texCoords;
	}

	const size_t AssimpMesh::getVertexCount() const
	{
		return _ptr->mNumVertices;
	}

	const AssimpMeshFaceCollection& AssimpMesh::getFaces() const
	{
		return _faces;
	}

	const AssimpMeshColorsCollection& AssimpMesh::getColors() const
	{
		return _colors;
	}

	AssimpNodeMeshCollection::AssimpNodeMeshCollection(aiNode* ptr, AssimpScene& scene)
		: _ptr(ptr)
		, _scene(scene)
	{
	}
	
	size_t AssimpNodeMeshCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumMeshes;
	}

	const AssimpMesh& AssimpNodeMeshCollection::operator[](size_t pos) const
	{
		auto m = _ptr->mMeshes[pos];
		return _scene.getMeshes()[m];
	}

	AssimpMesh& AssimpNodeMeshCollection::operator[](size_t pos)
	{
		auto m = _ptr->mMeshes[pos];
		return _scene.getMeshes()[m];
	}

	AssimpCamera::AssimpCamera(aiCamera* ptr)
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

	std::string_view AssimpCamera::getName() const
	{
		return getStringView(_ptr->mName);
	}

	const glm::mat4& AssimpCamera::getProjectionMatrix() const
	{
		return _proj;
	}

	const glm::mat4& AssimpCamera::getViewMatrix() const
	{
		return _view;
	}

	float AssimpCamera::getAspect() const
	{
		return _ptr->mAspect;
	}

	float AssimpCamera::getClipFar() const
	{
		return _ptr->mClipPlaneFar;
	}

	float AssimpCamera::getClipNear() const
	{
		return _ptr->mClipPlaneNear;
	}

	float AssimpCamera::getHorizontalFieldOfView() const
	{
		return _ptr->mHorizontalFOV;
	}

	float AssimpCamera::getOrthographicWidth() const
	{
		return _ptr->mOrthographicWidth;
	}

	glm::vec3 AssimpCamera::getLookAt() const
	{
		return convertVector(_ptr->mLookAt);
	}

	glm::vec3 AssimpCamera::getPosition() const
	{
		return convertVector(_ptr->mPosition);
	}

	glm::vec3 AssimpCamera::getUp() const
	{
		return convertVector(_ptr->mUp);
	}

	AssimpLight::AssimpLight(aiLight* ptr) noexcept
		: _ptr(ptr)
	{
	}

	std::string_view AssimpLight::getName() const noexcept
	{
		return getStringView(_ptr->mName);
	}

	float AssimpLight::getInnerConeAngle() const noexcept
	{
		return _ptr->mAngleInnerCone;
	}

	float AssimpLight::getOuterConeAngle() const noexcept
	{
		return _ptr->mAngleOuterCone;
	}

	glm::vec3 AssimpLight::getAttenuation() const noexcept
	{
		return {
			_ptr->mAttenuationConstant,
			_ptr->mAttenuationLinear,
			_ptr->mAttenuationQuadratic,
		};
	}

	Color3 AssimpLight::getAmbientColor() const noexcept
	{
		return convertColor(_ptr->mColorAmbient);
	}

	Color3 AssimpLight::getDiffuseColor() const noexcept
	{
		return convertColor(_ptr->mColorDiffuse);
	}

	Color3 AssimpLight::getSpecularColor() const noexcept
	{
		return convertColor(_ptr->mColorSpecular);
	}

	glm::vec3 AssimpLight::getDirection() const noexcept
	{
		return convertVector(_ptr->mDirection);
	}

	glm::vec3 AssimpLight::getPosition() const noexcept
	{
		return convertVector(_ptr->mPosition);
	}

	aiLightSourceType AssimpLight::getType() const noexcept
	{
		return _ptr->mType;
	}

	glm::vec2 AssimpLight::getSize() const noexcept
	{
		return convertVector(_ptr->mSize);
	}

	AssimpNodeChildrenCollection::AssimpNodeChildrenCollection(aiNode* ptr, AssimpScene& scene)
		: _ptr(ptr)
		, _scene(scene)
	{
	}

	size_t AssimpNodeChildrenCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumChildren;
	}

	AssimpNode AssimpNodeChildrenCollection::create(size_t pos) const
	{
		return AssimpNode(_ptr->mChildren[pos], _scene);
	}

	AssimpNode::AssimpNode(aiNode* ptr, AssimpScene& scene)
		: _ptr(ptr)
		, _meshes(ptr, scene)
		, _children(ptr, scene)
		, _scene(scene)
	{
	}

	std::string_view AssimpNode::getName() const
	{
		return getStringView(_ptr->mName);
	}

	glm::mat4 AssimpNode::getTransform() const
	{
		return convertMatrix(_ptr->mTransformation);
	}

	const AssimpNodeMeshCollection& AssimpNode::getMeshes() const
	{
		return _meshes;
	}

	const AssimpNodeChildrenCollection& AssimpNode::getChildren() const
	{
		return _children;
	}

	AssimpNodeMeshCollection& AssimpNode::getMeshes()
	{
		return _meshes;
	}

	AssimpNodeChildrenCollection& AssimpNode::getChildren()
	{
		return _children;
	}

	template<typename T>
	static OptionalRef<T> getAssimpNodeChild(T& node, const std::string_view& path)
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

	OptionalRef<const AssimpNode> AssimpNode::getChild(const std::string_view& path) const
	{
		return getAssimpNodeChild(*this, path);
	}

	OptionalRef<const AssimpCamera> AssimpNode::getCamera() const
	{
		return _scene.getCameras().get(getName());
	}

	OptionalRef<const AssimpLight> AssimpNode::getLight() const
	{
		return _scene.getLights().get(getName());
	}

	OptionalRef<AssimpNode> AssimpNode::getChild(const std::string_view& path)
	{
		return getAssimpNodeChild(*this, path);
	}

	OptionalRef<AssimpCamera> AssimpNode::getCamera()
	{
		return _scene.getCameras().get(getName());
	}

	OptionalRef<AssimpLight> AssimpNode::getLight()
	{
		return _scene.getLights().get(getName());
	}

	AssimpCameraCollection::AssimpCameraCollection(const aiScene* ptr)
		: _ptr(ptr)
	{
	}

	size_t AssimpCameraCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumCameras;
	}

	AssimpCamera AssimpCameraCollection::create(size_t pos) const
	{
		return AssimpCamera(_ptr->mCameras[pos]);
	}

	OptionalRef<AssimpCamera> AssimpCameraCollection::get(const std::string_view& name)
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

	OptionalRef<const AssimpCamera> AssimpCameraCollection::get(const std::string_view& name) const
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

	AssimpLightCollection::AssimpLightCollection(const aiScene* ptr)
		: _ptr(ptr)
	{
	}

	size_t AssimpLightCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumCameras;
	}

	AssimpLight AssimpLightCollection::create(size_t pos) const
	{
		return AssimpLight(_ptr->mLights[pos]);
	}

	OptionalRef<AssimpLight> AssimpLightCollection::get(const std::string_view& name)
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

	OptionalRef<const AssimpLight> AssimpLightCollection::get(const std::string_view& name) const
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

	AssimpMaterialCollection::AssimpMaterialCollection(const aiScene* ptr)
		: _ptr(ptr)
	{
	}

	size_t AssimpMaterialCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumMaterials;
	}

	AssimpMaterial AssimpMaterialCollection::create(size_t pos) const
	{
		return AssimpMaterial(_ptr->mMaterials[pos]);
	}

	AssimpMeshCollection::AssimpMeshCollection(const aiScene* ptr, AssimpMaterialCollection& materials)
		: _ptr(ptr)
		, _materials(materials)
	{
	}

	size_t AssimpMeshCollection::size() const
	{
		return _ptr == nullptr ? 0 : _ptr->mNumMeshes;
	}

	AssimpMesh AssimpMeshCollection::create(size_t pos) const
	{
		auto ptr = _ptr->mMeshes[pos];
		auto& material = _materials[ptr->mMaterialIndex];
		return AssimpMesh(ptr, material);
	}

	AssimpScene::AssimpScene(const aiScene* ptr) noexcept
		: _ptr(ptr)
		, _rootNode(ptr->mRootNode, *this)
		, _materials(ptr)
		, _meshes(ptr, _materials)
		, _cameras(ptr)
		, _lights(ptr)
	{
	}

	AssimpScene::~AssimpScene() noexcept
	{
		delete _ptr;
	}

	std::string_view AssimpScene::getName() const noexcept
	{
		return getStringView(_ptr->mName);
	}

	const AssimpNode& AssimpScene::getRootNode() const noexcept
	{
		return _rootNode;
	}

	const AssimpMeshCollection& AssimpScene::getMeshes() const noexcept
	{
		return _meshes;
	}

	const AssimpMaterialCollection& AssimpScene::getMaterials() const noexcept
	{
		return _materials;
	}

	const AssimpCameraCollection& AssimpScene::getCameras() const noexcept
	{
		return _cameras;
	}

	const AssimpLightCollection& AssimpScene::getLights() const noexcept
	{
		return _lights;
	}

	AssimpNode& AssimpScene::getRootNode() noexcept
	{
		return _rootNode;
	}

	AssimpMeshCollection& AssimpScene::getMeshes() noexcept
	{
		return _meshes;
	}

	AssimpMaterialCollection& AssimpScene::getMaterials() noexcept
	{
		return _materials;
	}

	AssimpCameraCollection& AssimpScene::getCameras() noexcept
	{
		return _cameras;
	}

	AssimpLightCollection& AssimpScene::getLights() noexcept
	{
		return _lights;
	}
}
