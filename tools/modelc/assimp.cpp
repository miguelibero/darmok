
#include "assimp.hpp"

#include <darmok/vertex.hpp>
#include <darmok/math.hpp>
#include <darmok/model.hpp>
#include <darmok/image.hpp>

#include <assimp/vector3.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <filesystem>

namespace darmok
{
    struct AssimpUtils final
    {
        static inline std::string_view getStringView(const aiString& str) noexcept
        {
            return std::string_view(str.data, str.length);
        }

        static inline glm::mat4 convert(const aiMatrix4x4& from) noexcept
        {
            // the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
            return glm::mat4(
                from.a1, from.b1, from.c1, from.d1,
                from.a2, from.b2, from.c2, from.d2,
                from.a3, from.b3, from.c3, from.d3,
                from.a4, from.b4, from.c4, from.d4
            );
        }

        static inline glm::vec3 convert(const aiVector3D& vec) noexcept
        {
            return glm::vec3(vec.x, vec.y, vec.z);
        }

        static inline glm::vec2 convert(const aiVector2D& vec) noexcept
        {
            return glm::vec2(vec.x, vec.y);
        }

        static inline uint8_t convertColorComp(ai_real v) noexcept
        {
            return 255 * v;
        }

        static inline Color convert(const aiColor4D& c) noexcept
        {
            return Color
            {
                convertColorComp(c.r),
                convertColorComp(c.g),
                convertColorComp(c.b),
                convertColorComp(c.a),
            };
        }

        static inline Color3 convert(const aiColor3D& c) noexcept
        {
            return Color3
            {
                convertColorComp(c.r),
                convertColorComp(c.g),
                convertColorComp(c.b)
            };
        }
    };

    AssimpEmbeddedTexture::AssimpEmbeddedTexture(const aiTexture& tex, aiSceneRef scene)
        : _texture(tex)
        , _scene(scene)
    {
    }

    glm::uvec2 AssimpEmbeddedTexture::getSize() const noexcept
    {
        return glm::uvec2(_texture->mWidth, _texture->mHeight);
    }

    bool AssimpEmbeddedTexture::isCompressed() const noexcept
    {
        return _texture->mHeight == 0;
    }

    std::shared_ptr<Image> AssimpEmbeddedTexture::loadImage(bx::AllocatorI& alloc) const noexcept
    {
        auto data = getData();
        auto format = isCompressed() ? bimg::TextureFormat::Count : bimg::TextureFormat::RGBA8;
        return std::make_shared<Image>(data, alloc, format);
    }

    DataView AssimpEmbeddedTexture::getData() const noexcept
    {
        size_t size;
        if (isCompressed())
        {
            size = _texture->mWidth;
        }
        else
        {
            size = _texture->mWidth * _texture->mHeight * 4;
        }

        return DataView(_texture->pcData, size);
    }

    AssimpMaterialProperty::AssimpMaterialProperty(const aiMaterialProperty& prop, aiSceneRef scene) noexcept
        : _prop(prop)
        , _scene(scene)
        , _data(prop.mData, prop.mDataLength)
    {
    }

    std::string_view AssimpMaterialProperty::getKey() const noexcept
    {
        return AssimpUtils::getStringView(_prop->mKey);
    }

    aiPropertyTypeInfo AssimpMaterialProperty::getType() const noexcept
    {
        return _prop->mType;
    }

    aiTextureType AssimpMaterialProperty::getTextureType() const noexcept
    {
        return (aiTextureType)_prop->mSemantic;
    }

    size_t AssimpMaterialProperty::getTextureIndex() const noexcept
    {
        return _prop->mIndex;
    }

    const DataView& AssimpMaterialProperty::getData() const noexcept
    {
        return _data;
    }

    AssimpMaterialTexture::AssimpMaterialTexture(const aiMaterial& material, aiTextureType type, unsigned int index) noexcept
        : _type(type)
        , _index(index)
        , _coordIndex(0)
        , _mapping(aiTextureMapping_OTHER)
        , _blend(0)
        , _operation(aiTextureOp_Add)
        , _mapMode(aiTextureMapMode_Clamp)
    {

        aiString path;
        unsigned int uvindex;
        material.GetTexture(type, index, &path,
            &_mapping,
            &uvindex,
            &_blend,
            &_operation,
            &_mapMode
        );
        _coordIndex = uvindex;
        _path = path.C_Str();
    }

    aiTextureType AssimpMaterialTexture::getType() const noexcept
    {
        return _type;
    }

    size_t AssimpMaterialTexture::getIndex() const noexcept
    {
        return _index;
    }

    const std::string& AssimpMaterialTexture::getPath() const noexcept
    {
        return _path;
    }

    aiTextureMapping AssimpMaterialTexture::getMapping() const noexcept
    {
        return _mapping;
    }

    aiTextureOp AssimpMaterialTexture::getOperation() const noexcept
    {
        return _operation;
    }

    aiTextureMapMode AssimpMaterialTexture::getMapMode() const noexcept
    {
        return _mapMode;
    }

    float AssimpMaterialTexture::getBlend() const noexcept
    {
        return _blend;
    }

    size_t AssimpMaterialTexture::getCoordIndex() const noexcept
    {
        return _coordIndex;
    }

    AssimpMaterial::AssimpMaterial(const aiMaterial& material, aiSceneRef scene) noexcept
        : _material(material)
        , _scene(scene)
    {
        auto size = material.mNumProperties;
        _properties.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            _properties.emplace_back(*material.mProperties[i], scene);
        }
    }

    std::string_view AssimpMaterial::getName() const noexcept
    {
        return AssimpUtils::getStringView(_material->GetName());
    }

    std::vector<AssimpMaterialTexture> AssimpMaterial::getTextures(aiTextureType type) const noexcept
    {
        std::vector<AssimpMaterialTexture> textures;
        fillTextures(textures, type);
        return textures;
    }

    void AssimpMaterial::fillTextures(std::vector<AssimpMaterialTexture>& textures, aiTextureType type) const noexcept
    {
        auto size = _material->GetTextureCount((aiTextureType)type);
        textures.reserve(size);
        for (unsigned int i = 0; i < size; i++)
        {
            textures.push_back(AssimpMaterialTexture(_material.value(), type, i));
        }
    }

    const std::vector<AssimpMaterialProperty>& AssimpMaterial::getProperties() const noexcept
    {
        return _properties;
    }

    std::optional<Color> AssimpMaterial::getColor(AssimpMaterialColorType type) const noexcept
    {
        aiColor4D color;
        aiReturn r = AI_FAILURE;
        switch (type)
        {
        case AssimpMaterialColorType::Diffuse:
            r = _material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            break;
        case AssimpMaterialColorType::Specular:
            r = _material->Get(AI_MATKEY_COLOR_SPECULAR, color);
            break;
        case AssimpMaterialColorType::Ambient:
            r = _material->Get(AI_MATKEY_COLOR_AMBIENT, color);
            break;
        case AssimpMaterialColorType::Emissive:
            r = _material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
            break;
        case AssimpMaterialColorType::Transparent:
            r = _material->Get(AI_MATKEY_COLOR_TRANSPARENT, color);
            break;
        case AssimpMaterialColorType::Reflective:
            r = _material->Get(AI_MATKEY_COLOR_REFLECTIVE, color);
            break;
        }
        if (r == AI_SUCCESS)
        {
            return AssimpUtils::convert(color);
        }
        return std::nullopt;
    }

    bool AssimpMaterial::showWireframe() const noexcept
    {
        int v;
        if (AI_SUCCESS == _material->Get(AI_MATKEY_ENABLE_WIREFRAME, v))
        {
            return v != 0;
        }
        return false;
    }

    aiShadingMode AssimpMaterial::getShadingMode() const noexcept
    {
        int v;
        if (AI_SUCCESS == _material->Get(AI_MATKEY_SHADING_MODEL, v))
        {
            return (aiShadingMode)v;
        }
        return aiShadingMode_NoShading;
    }

    float AssimpMaterial::getOpacity() const noexcept
    {
        float v;
        if (AI_SUCCESS == _material->Get(AI_MATKEY_OPACITY, v))
        {
            return v != 0;
        }
        return 1.f;
    }

    AssimpMeshFace::AssimpMeshFace(const aiFace& face, aiSceneRef scene) noexcept
        : _face(face)
        , _scene(scene)
    {
    }

    size_t AssimpMeshFace::size() const noexcept
    {
        return _face->mNumIndices;
    }

    VertexIndex AssimpMeshFace::operator[](size_t pos) const
    {
        if (pos < 0 || pos >= size())
        {
            throw std::runtime_error("invalid face index");
        }
        return _face->mIndices[pos];

    }

    AssimpTextureCoords::AssimpTextureCoords(const aiMesh& mesh, size_t pos, aiSceneRef scene) noexcept
        : _scene(scene)
        , _compCount(mesh.mNumUVComponents[pos])
        , _coords(mesh.mTextureCoords[pos], mesh.mNumVertices)
    {
    }

    size_t AssimpTextureCoords::getCompCount() const noexcept
    {
        return _compCount;
    }
    
    AssimpVector3Collection AssimpTextureCoords::getCoords() const noexcept
    {
        return _coords;
    }

    AssimpVector3Collection::AssimpVector3Collection(const aiVector3D* ptr, size_t size) noexcept
        : _ptr(ptr)
        , _size(size)
    {
    }

    size_t AssimpVector3Collection::size() const noexcept
    {
        return _size;
    }

    glm::vec3 AssimpVector3Collection::operator[](size_t pos) const
    {
        if (pos < 0 || pos >= _size)
        {
            throw std::runtime_error("invalid access");
        }
        return AssimpUtils::convert(_ptr[pos]);
    }

    AssimpColorCollection::AssimpColorCollection(const aiColor4D* ptr, size_t size) noexcept
        : _ptr(ptr)
        , _size(size)
    {
    }

    size_t AssimpColorCollection::size() const noexcept
    {
        return _size;
    }

    Color AssimpColorCollection::operator[](size_t pos) const
    {
        if (pos < 0 || pos >= _size)
        {
            throw std::runtime_error("invalid access");
        }
        return AssimpUtils::convert(_ptr[pos]);
    }

    AssimpVertexWeightCollection::AssimpVertexWeightCollection(const aiVertexWeight* ptr, size_t size) noexcept
        : _ptr(ptr)
        , _size(size)
    {
    }

    size_t AssimpVertexWeightCollection::size() const noexcept
    {
        return _size;
    }

    AssimpVertexWeight AssimpVertexWeightCollection::operator[](size_t pos) const
    {
        auto w = _ptr[pos];
        return AssimpVertexWeight{ w.mVertexId, w.mWeight };
    }

    AssimpBone::AssimpBone(const aiBone& bone, aiSceneRef scene) noexcept
        : _bone(bone)
        , _scene(scene)
        , _weights(bone.mWeights, bone.mNumWeights)
    {
    }

    std::string_view AssimpBone::getName() const noexcept
    {
        return AssimpUtils::getStringView(_bone->mName);
    }

    glm::mat4 AssimpBone::getInverseBindPoseMatrix() const noexcept
    {
        return AssimpUtils::convert(_bone->mOffsetMatrix);
    }

    const AssimpVertexWeightCollection& AssimpBone::getWeights() const noexcept
    {
        return _weights;
    }

    AssimpMesh::AssimpMesh(const aiMesh& mesh, const std::shared_ptr<AssimpMaterial>& material, aiSceneRef scene) noexcept
        : _mesh(mesh)
        , _material(material)
        , _scene(scene)
        , _positions(mesh.mVertices, mesh.mNumVertices)
        , _normals(mesh.mNormals, mesh.mNumVertices)
        , _tangents(mesh.mTangents, mesh.mNumVertices)
        , _bitangents(mesh.mBitangents, mesh.mNumVertices)
    {
        {
            size_t size = AI_MAX_NUMBER_OF_COLOR_SETS;
            _colors.reserve(size);
            for (size_t i = 0; i < size; i++)
            {
                if (mesh.mColors[i] != nullptr)
                {
                    _colors.emplace_back(mesh.mColors[i], mesh.mNumVertices);
                }
            }
        }
        {
            size_t size = AI_MAX_NUMBER_OF_TEXTURECOORDS;
            _texCoords.reserve(size);
            for (size_t i = 0; i < size; i++)
            {
                if (mesh.mTextureCoords[i] != nullptr)
                {
                    _texCoords.emplace_back(mesh, i, scene);
                }
            }
        }
        {
            if (mesh.mFaces != nullptr)
            {
                auto size = mesh.mNumFaces;
                _faces.reserve(size);
                for (size_t i = 0; i < size; i++)
                {
                    _faces.emplace_back(mesh.mFaces[i], scene);
                }
            }
        }
        {
            if (mesh.mBones != nullptr)
            {
                auto size = mesh.mNumBones;
                _bones.reserve(size);
                for (size_t i = 0; i < size; i++)
                {
                    _bones.emplace_back(*mesh.mBones[i], scene);
                }
            }
        }
    }

    bool AssimpMesh::hasBones() const noexcept
    {
        if (!_mesh)
        {
            return false;
        }
        return _mesh->HasBones();
    }

    std::shared_ptr<AssimpMaterial> AssimpMesh::getMaterial() const noexcept
    {
        return _material;
    }

    const AssimpVector3Collection& AssimpMesh::getPositions() const noexcept
    {
        return _positions;
    }

    const AssimpVector3Collection& AssimpMesh::getNormals() const noexcept
    {
        return _normals;
    }

    const AssimpVector3Collection& AssimpMesh::getTangents() const noexcept
    {
        return _tangents;
    }

    const AssimpVector3Collection& AssimpMesh::getBitangents() const noexcept
    {
        return _bitangents;
    }

    const std::vector<AssimpTextureCoords>& AssimpMesh::getTexCoords() const noexcept
    {
        return _texCoords;
    }

    const size_t AssimpMesh::getVertexCount() const noexcept
    {
        if (!_mesh)
        {
            return 0;
        }
        return _mesh->mNumVertices;
    }

    Data AssimpMesh::createVertexData(const bgfx::VertexLayout& layout, bx::AllocatorI& alloc) const noexcept
    {
        auto vertexCount = getVertexCount();
        VertexDataWriter writer(layout, vertexCount, alloc);
        writer.write(bgfx::Attrib::Position, getPositions());
        writer.write(bgfx::Attrib::Normal, getNormals());
        writer.write(bgfx::Attrib::Tangent, getTangents());
        writer.write(bgfx::Attrib::Bitangent, getBitangents());

        {
            auto i = 0;
            for (auto& elm : getTexCoords())
            {
                auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::TexCoord0 + i++);
                writer.write(attrib, elm.getCoords());
            }
        }
        {
            auto i = 0;
            for (auto& elm : getColors())
            {
                auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::Color0 + i++);
                writer.write(attrib, elm);
            }
        }

        if(hasBones())
        {
            int boneIndex = 0;
            std::vector<int> boneCount(vertexCount);
            std::vector<glm::vec4> boneIndices(vertexCount);
            std::fill(boneIndices.begin(), boneIndices.end(), glm::vec4(-1));
            std::vector<glm::vec4> boneWeights(vertexCount);
            std::fill(boneWeights.begin(), boneWeights.end(), glm::vec4(1, 0, 0, 0));
            for (auto& bone : getBones())
            {
                for (auto weight : bone.getWeights())
                {
                    auto i = weight.vertexIndex;
                    auto c = boneCount[i]++;
                    if (c <= 3)
                    {
                        boneIndices[i][c] = boneIndex;
                        boneWeights[i][c] = weight.value;
                    }
                }
                ++boneIndex;
            }
            writer.write(bgfx::Attrib::Indices, boneIndices);
            writer.write(bgfx::Attrib::Weight, boneWeights);
        }

        return writer.finish();
    }

    std::vector<VertexIndex> AssimpMesh::createIndexData() const noexcept
    {
        size_t size = 0;
        for (auto& face : getFaces())
        {
            size += face.size();
        }
        std::vector<VertexIndex> indices;
        indices.reserve(size);
        for (auto& face : getFaces())
        {
            for (auto index : face)
            {
                indices.push_back(index);
            }
        }
        return indices;
    }

    const std::vector<AssimpMeshFace>& AssimpMesh::getFaces() const noexcept
    {
        return _faces;
    }

    const std::vector<AssimpColorCollection>& AssimpMesh::getColors() const noexcept
    {
        return _colors;
    }

    const std::vector<AssimpBone>& AssimpMesh::getBones() const noexcept
    {
        return _bones;
    }

    AssimpCamera::AssimpCamera(const aiCamera& cam, aiSceneRef scene) noexcept
        : _cam(cam)
        , _scene(scene)
        , _view(1)
        , _proj(1)
    {
        aiMatrix4x4 mat;
        cam.GetCameraMatrix(mat);
        _view = AssimpUtils::convert(mat);

        auto aspect = getAspect();
        auto fovy = 0.f;
        if (aspect != 0.f)
        {
            auto fovx = getHorizontalFieldOfView();
            fovy = 2.f * atan(tan(0.5f * fovx) * aspect);
        }

        _proj = Math::perspective(fovy, aspect, getClipNear(), getClipFar());
    }

    std::string_view AssimpCamera::getName() const noexcept
    {
        return AssimpUtils::getStringView(_cam->mName);
    }

    const glm::mat4& AssimpCamera::getProjectionMatrix() const noexcept
    {
        return _proj;
    }

    const glm::mat4& AssimpCamera::getViewMatrix() const noexcept
    {
        return _view;
    }

    float AssimpCamera::getAspect() const noexcept
    {
        return _cam->mAspect;
    }

    float AssimpCamera::getClipFar() const noexcept
    {
        return _cam->mClipPlaneFar;
    }

    float AssimpCamera::getClipNear() const noexcept
    {
        return _cam->mClipPlaneNear;
    }

    float AssimpCamera::getHorizontalFieldOfView() const noexcept
    {
        return _cam->mHorizontalFOV;
    }

    float AssimpCamera::getOrthographicWidth() const noexcept
    {
        return _cam->mOrthographicWidth;
    }

    glm::vec3 AssimpCamera::getLookAt() const noexcept
    {
        return AssimpUtils::convert(_cam->mLookAt);
    }

    glm::vec3 AssimpCamera::getPosition() const noexcept
    {
        return AssimpUtils::convert(_cam->mPosition);
    }

    glm::vec3 AssimpCamera::getUp() const noexcept
    {
        return AssimpUtils::convert(_cam->mUp);
    }

    AssimpLight::AssimpLight(const aiLight& light, aiSceneRef scene) noexcept
        : _light(light)
        , _scene(scene)
    {
    }

    std::string_view AssimpLight::getName() const noexcept
    {
        return AssimpUtils::getStringView(_light->mName);
    }

    float AssimpLight::getInnerConeAngle() const noexcept
    {
        return _light->mAngleInnerCone;
    }

    float AssimpLight::getOuterConeAngle() const noexcept
    {
        return _light->mAngleOuterCone;
    }

    glm::vec3 AssimpLight::getAttenuation() const noexcept
    {
        return {
            _light->mAttenuationConstant,
            _light->mAttenuationLinear,
            _light->mAttenuationQuadratic,
        };
    }

    Color3 AssimpLight::getAmbientColor() const noexcept
    {
        return AssimpUtils::convert(_light->mColorAmbient);
    }

    Color3 AssimpLight::getDiffuseColor() const noexcept
    {
        return AssimpUtils::convert(_light->mColorDiffuse);
    }

    Color3 AssimpLight::getSpecularColor() const noexcept
    {
        return AssimpUtils::convert(_light->mColorSpecular);
    }

    glm::vec3 AssimpLight::getDirection() const noexcept
    {
        return AssimpUtils::convert(_light->mDirection);
    }

    glm::vec3 AssimpLight::getPosition() const noexcept
    {
        return AssimpUtils::convert(_light->mPosition);
    }

    aiLightSourceType AssimpLight::getType() const noexcept
    {
        return _light->mType;
    }

    glm::vec2 AssimpLight::getSize() const noexcept
    {
        return AssimpUtils::convert(_light->mSize);
    }

    AssimpNode::AssimpNode(const aiNode& node, const AssimpScene& scene, const ParentRef& parent) noexcept
        : _node(node)
        , _parent(parent)
        , _scene(scene.getInternal())
        , _light(scene.getLight(getName()))
        , _camera(scene.getCamera(getName()))
    {
        auto size = _node->mNumMeshes;
        _meshes.reserve(size);
        auto& meshes = scene.getMeshes();
        for (size_t i = 0; i < size; i++)
        {
            auto j = node.mMeshes[i];
            if (j < meshes.size())
            {
                _meshes.push_back(meshes[j]);
            }
        }
    }

    void AssimpNode::loadChildren(const std::weak_ptr<AssimpNode> selfPtr, const AssimpScene& scene) noexcept
    {
        auto size = _node->mNumChildren;
        _children.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            auto node = std::make_shared<AssimpNode>(*_node->mChildren[i], scene, selfPtr);
            node->loadChildren(node, scene);
            _children.push_back(node);
        }
    }

    const aiNode& AssimpNode::getAssimp() const noexcept
    {
        return _node.value();
    }

    std::string_view AssimpNode::getName() const noexcept
    {
        return AssimpUtils::getStringView(_node->mName);
    }

    glm::mat4 AssimpNode::getTransform() const noexcept
    {
        return AssimpUtils::convert(_node->mTransformation);
    }

    glm::mat4 AssimpNode::getWorldTransform() const noexcept
    {
        if (_worldTransform)
        {
            return _worldTransform.value();
        }
        auto trans = getWorldTransform(_node.ptr());
        _worldTransform = trans;
        return trans;
    }

    glm::mat4 AssimpNode::getWorldTransform(const aiNode* node) noexcept
    {
        aiMatrix4x4 trans;
        while (node != nullptr)
        {
            trans = node->mTransformation * trans;
            node = node->mParent;
        }
        return AssimpUtils::convert(trans);
    }

    AssimpNode::ParentRef AssimpNode::getParent() const noexcept
    {
        return _parent;
    }

    const std::vector<std::shared_ptr<AssimpMesh>>& AssimpNode::getMeshes() const noexcept
    {
        return _meshes;
    }

    const std::vector<std::shared_ptr<AssimpNode>>& AssimpNode::getChildren() const noexcept
    {
        return _children;
    }

    std::shared_ptr<AssimpNode> AssimpNode::findChildByPath(std::string_view path) const noexcept
    {
        auto itr = path.find('/');
        auto sep = itr != std::string::npos;
        auto name = sep ? path.substr(0, itr) : path;
        for (auto& child : getChildren())
        {
            if (child->getName() != name)
            {
                continue;
            }
            if (!sep)
            {
                return child;
            }
            return child->findChildByPath(path.substr(itr + 1));
        }
        return nullptr;
    }

    std::shared_ptr<AssimpNode> AssimpNode::findChildByName(std::string_view name) const noexcept
    {
        for (auto& child : getChildren())
        {
            if (child->getName() == name)
            {
                return child;
            }
            auto result = child->findChildByName(name);
            if (result != nullptr)
            {
                return result;
            }
        }
        return nullptr;
    }

    std::shared_ptr<AssimpCamera> AssimpNode::getCamera() const noexcept
    {
        return _camera;
    }

    std::shared_ptr<AssimpLight> AssimpNode::getLight() const noexcept
    {
        return _light;
    }

    std::shared_ptr<AssimpCamera> AssimpScene::getCamera(std::string_view name) const noexcept
    {
        for (auto& cam : _cameras)
        {
            if (cam->getName() == name)
            {
                return cam;
            }
        }
        return nullptr;
    }

    std::shared_ptr<AssimpLight> AssimpScene::getLight(std::string_view name) const noexcept
    {
        for (auto& light : _lights)
        {
            if (light->getName() == name)
            {
                return light;
            }
        }
        return nullptr;
    }

    std::optional<AssimpEmbeddedTexture> AssimpScene::getEmbeddedTexture(const std::string& name) const noexcept
    {
        auto tex = _scene->GetEmbeddedTexture(name.c_str());
        if (tex == nullptr)
        {
            return std::nullopt;
        }
        return AssimpEmbeddedTexture(*tex, _scene);
    }

    std::vector<std::shared_ptr<AssimpMaterial>> AssimpScene::loadMaterials(aiSceneRef scene) noexcept
    {
        std::vector<std::shared_ptr<AssimpMaterial>> materials;
        auto size = scene->mNumMaterials;
        materials.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            materials.push_back(std::make_shared<AssimpMaterial>(*scene->mMaterials[i], scene));
        }
        return materials;
    }

    std::vector<std::shared_ptr<AssimpCamera>> AssimpScene::loadCameras(aiSceneRef scene) noexcept
    {
        std::vector<std::shared_ptr<AssimpCamera>> cameras;
        auto size = scene->mNumCameras;
        cameras.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            cameras.push_back(std::make_shared<AssimpCamera>(*scene->mCameras[i], scene));
        }
        return cameras;
    }

    std::vector<std::shared_ptr<AssimpLight>> AssimpScene::loadLights(aiSceneRef scene) noexcept
    {
        std::vector<std::shared_ptr<AssimpLight>> lights;
        auto size = scene->mNumLights;
        lights.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            lights.push_back(std::make_shared<AssimpLight>(*scene->mLights[i], scene));
        }
        return lights;
    }

    std::vector<std::shared_ptr<AssimpMesh>> AssimpScene::loadMeshes(aiSceneRef scene, const std::vector<std::shared_ptr<AssimpMaterial>>& materials) noexcept
    {
        std::vector<std::shared_ptr<AssimpMesh>> meshes;
        auto size = scene->mNumMeshes;
        meshes.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            auto meshPtr = scene->mMeshes[i];
            if (meshPtr != nullptr && meshPtr->mMaterialIndex < materials.size())
            {
                auto& material = materials[meshPtr->mMaterialIndex];
                meshes.push_back(std::make_shared<AssimpMesh>(*meshPtr, material, scene));
            }
        }
        return meshes;
    }

    AssimpScene::AssimpScene(aiSceneRef scene) noexcept
        : _scene(scene)
        , _materials(loadMaterials(scene))
        , _meshes(loadMeshes(scene, _materials))
        , _cameras(loadCameras(scene))
        , _lights(loadLights(scene))
        , _rootNode(std::make_shared<AssimpNode>(*scene->mRootNode, *this))
    {
        _rootNode->loadChildren(_rootNode, *this);
    }

    AssimpScene::AssimpScene(Assimp::Importer& importer, const DataView& data, const std::string& path)
        : AssimpScene(importScene(importer, data, path))
    {
    }

    aiSceneRef AssimpScene::importScene(Assimp::Importer& importer, const DataView& data, const std::string& path)
    {
        unsigned int flags = aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType |
            aiProcess_GlobalScale |
            aiProcess_PopulateArmatureData |
            aiProcess_ConvertToLeftHanded
            ;
        // assimp (and opengl) is right handed (+Z points towards the camera)
        // while bgfx (and darmok and directx) is left handed (+Z points away from the camera)

        auto ptr = importer.ReadFileFromMemory(data.ptr(), data.size(), flags, path.c_str());
        if (ptr == nullptr)
        {
            throw std::runtime_error(importer.GetErrorString());
        }

        auto scene = importer.GetOrphanedScene();

        // scale camera clip planes, seems to be an assimp bug
        // https://github.com/assimp/assimp/issues/3240
        auto scale = importer.GetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);
        scale *= importer.GetPropertyFloat(AI_CONFIG_APP_SCALE_KEY, 1.0f);
        for (auto i = 0; i < scene->mNumCameras; i++)
        {
            auto cam = scene->mCameras[i];
            cam->mClipPlaneNear *= scale;
            cam->mClipPlaneFar *= scale;
        }

        // take ownership of the scene
        return aiSceneRef(scene);
    }

    std::string_view AssimpScene::getName() const noexcept
    {
        return AssimpUtils::getStringView(_scene->mName);
    }

    aiSceneRef AssimpScene::getInternal() const noexcept
    {
        return _scene;
    }

    std::shared_ptr<AssimpNode> AssimpScene::getRootNode() const noexcept
    {
        return _rootNode;
    }

    const std::vector<std::shared_ptr<AssimpMesh>>& AssimpScene::getMeshes() const noexcept
    {
        return _meshes;
    }

    const std::vector<std::shared_ptr<AssimpMaterial>>& AssimpScene::getMaterials() const noexcept
    {
        return _materials;
    }

    const std::vector<std::shared_ptr<AssimpCamera>>& AssimpScene::getCameras() const noexcept
    {
        return _cameras;
    }

    const std::vector<std::shared_ptr<AssimpLight>>& AssimpScene::getLights() const noexcept
    {
        return _lights;
    }
}