
#include "assimp.hpp"

#include <darmok/texture.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <darmok/image.hpp>
#include <darmok/vertex.hpp>

#include <assimp/vector3.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <bx/math.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_clip_space.hpp>

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

        template<typename T>
        static OptionalRef<T> getNodeChild(T& node, const std::string_view& path) noexcept
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
    };

    AssimpMaterialProperty::AssimpMaterialProperty(const aiMaterialProperty& prop, const aiSceneRef& scene) noexcept
        : _prop(prop)
        , _scene(scene)
        , _data(prop.mData, prop.mDataLength)
    {
    }

    std::string_view AssimpMaterialProperty::getKey() const noexcept
    {
        return AssimpUtils::getStringView(_prop.mKey);
    }

    aiPropertyTypeInfo AssimpMaterialProperty::getType() const noexcept
    {
        return _prop.mType;
    }

    aiTextureType AssimpMaterialProperty::getTextureType() const noexcept
    {
        return (aiTextureType)_prop.mSemantic;
    }

    size_t AssimpMaterialProperty::getTextureIndex() const noexcept
    {
        return _prop.mIndex;
    }

    const DataView& AssimpMaterialProperty::getData() const noexcept
    {
        return _data;
    }

    AssimpMaterialPropertyCollection::AssimpMaterialPropertyCollection(const aiMaterial& material, const aiSceneRef& scene) noexcept
    {
        auto size = material.mNumProperties;
        _elements.reserve(size);
        for(size_t i = 0; i < size; i++)
        {
            _elements.emplace_back(*material.mProperties[i++], scene);
        }
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
        material.GetTexture((aiTextureType)type, index, &path,
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

    AssimpMaterialTextureCollection::AssimpMaterialTextureCollection(const aiMaterial& material, aiTextureType type) noexcept
    {
        auto size = material.GetTextureCount((aiTextureType)type);
        _elements.reserve(size);
        for (unsigned int i = 0; i < size; i++)
        {
            _elements.emplace_back(material, type, i++);
        }
    }

    AssimpMaterial::AssimpMaterial(const aiMaterial& material, const aiSceneRef& scene, const std::string& basePath) noexcept
        : _material(material)
        , _scene(scene)
        , _properties(material, scene)
        , _basePath(basePath)
    {
    }

    std::string_view AssimpMaterial::getName() const noexcept
    {
        return AssimpUtils::getStringView(_material.GetName());
    }

    const AssimpMaterialTextureCollection& AssimpMaterial::getTextures(aiTextureType type) const noexcept
    {
        auto itr = _textures.find(type);
        if (itr == _textures.end())
        {
            auto r = _textures.emplace(std::make_pair(type, AssimpMaterialTextureCollection(_material, type)));
            itr = r.first;
        }
        return itr->second;
    }

    const AssimpMaterialPropertyCollection& AssimpMaterial::getProperties() const noexcept
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
            r = _material.Get(AI_MATKEY_COLOR_DIFFUSE, color);
            break;
        case AssimpMaterialColorType::Specular:
            r = _material.Get(AI_MATKEY_COLOR_SPECULAR, color);
            break;
        case AssimpMaterialColorType::Ambient:
            r = _material.Get(AI_MATKEY_COLOR_AMBIENT, color);
            break;
        case AssimpMaterialColorType::Emissive:
            r = _material.Get(AI_MATKEY_COLOR_EMISSIVE, color);
            break;
        case AssimpMaterialColorType::Transparent:
            r = _material.Get(AI_MATKEY_COLOR_TRANSPARENT, color);
            break;
        case AssimpMaterialColorType::Reflective:
            r = _material.Get(AI_MATKEY_COLOR_REFLECTIVE, color);
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
        if (AI_SUCCESS == _material.Get(AI_MATKEY_ENABLE_WIREFRAME, v))
        {
            return v != 0;
        }
        return false;
    }

    aiShadingMode AssimpMaterial::getShadingMode() const noexcept
    {
        int v;
        if (AI_SUCCESS == _material.Get(AI_MATKEY_SHADING_MODEL, v))
        {
            return (aiShadingMode)v;
        }
        return aiShadingMode_NoShading;
    }

    float AssimpMaterial::getOpacity() const noexcept
    {
        float v;
        if (AI_SUCCESS == _material.Get(AI_MATKEY_OPACITY, v))
        {
            return v != 0;
        }
        return 1.f;
    }

    const std::unordered_map<AssimpMaterialColorType, MaterialColorType> AssimpMaterial::_materialColors =
    {
        { AssimpMaterialColorType::Diffuse, MaterialColorType::Diffuse },
        { AssimpMaterialColorType::Specular, MaterialColorType::Specular },
        { AssimpMaterialColorType::Ambient, MaterialColorType::Ambient },
        { AssimpMaterialColorType::Emissive, MaterialColorType::Emissive },
        { AssimpMaterialColorType::Transparent, MaterialColorType::Transparent },
        { AssimpMaterialColorType::Reflective, MaterialColorType::Reflective },
    };

    const std::unordered_map<aiTextureType, MaterialTextureType> AssimpMaterial::_materialTextures =
    {
        { aiTextureType_DIFFUSE, MaterialTextureType::Diffuse },
        { aiTextureType_SPECULAR, MaterialTextureType::Specular },
        { aiTextureType_NORMALS, MaterialTextureType::Normal },
    };

    std::shared_ptr<Material> AssimpMaterial::load(ITextureLoader& textureLoader, bx::AllocatorI& alloc) const noexcept
    {
        auto material = std::make_shared<Material>();
        std::filesystem::path basePath(_basePath);
        for (auto& elm : _materialTextures)
        {
            for (auto& modelTex : getTextures(elm.first))
            {
                auto& path = modelTex.getPath();
                auto texture = loadEmbeddedTexture(path, alloc);
                if (texture == nullptr)
                {
                    std::filesystem::path fsPath(path);
                    if (fsPath.is_relative())
                    {
                        fsPath = basePath / fsPath;
                    }
                    texture = textureLoader(fsPath.string());
                }
                material->setTexture(elm.second, texture);
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

    std::shared_ptr<Texture> AssimpMaterial::loadEmbeddedTexture(const std::string& path, bx::AllocatorI& alloc) const noexcept
    {
        auto data = _scene->GetEmbeddedTexture(path.c_str());
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
        Image img(DataView(data->pcData, size), alloc, format);
        auto tex = std::make_shared<Texture>(img);
        tex->setName(path);
        return tex;
    }

    AssimpMeshFaceIndexCollection::AssimpMeshFaceIndexCollection(const aiFace& face, const aiSceneRef& scene) noexcept
        : _face(face)
        , _scene(scene)
    {
    }

    size_t AssimpMeshFaceIndexCollection::size() const noexcept
    {
        return _face.mNumIndices;
    }

    const VertexIndex& AssimpMeshFaceIndexCollection::operator[](size_t pos) const noexcept
    {
        return (VertexIndex&)_face.mIndices[pos];
    }

    VertexIndex& AssimpMeshFaceIndexCollection::operator[](size_t pos) noexcept
    {
        return (VertexIndex&)_face.mIndices[pos];
    }

    AssimpMeshFace::AssimpMeshFace(const aiFace& face, const aiSceneRef& scene) noexcept
        : _face(face)
        , _scene(scene)
        , _indices(face, scene)
    {
    }

    size_t AssimpMeshFace::size() const noexcept
    {
        return _face.mNumIndices;
    }

    bool AssimpMeshFace::empty() const noexcept
    {
        return size() == 0;
    }

    const AssimpMeshFaceIndexCollection& AssimpMeshFace::getIndices() const noexcept
    {
        return _indices;
    }

    AssimpVector3Collection::AssimpVector3Collection(aiVector3D* vectors, size_t size) noexcept
    {
        _elements.reserve(size);
        for(size_t i = 0; i < size; i++)
        {
            _elements.push_back(AssimpUtils::convert(vectors[i]));
        }
    }

    AssimpColorCollection::AssimpColorCollection(aiColor4D* colors, size_t size) noexcept
    {
        _elements.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            _elements.push_back(AssimpUtils::convert(colors[i]));
        }
    }

    AssimpTextureCoords::AssimpTextureCoords(const aiMesh& mesh, size_t pos) noexcept
        : _compCount(mesh.mNumUVComponents[pos])
        , _coords(mesh.mTextureCoords[pos], mesh.mNumVertices)
    {
    }

    size_t AssimpTextureCoords::getCompCount() const noexcept
    {
        return _compCount;
    }

    const AssimpVector3Collection& AssimpTextureCoords::getCoords() const noexcept
    {
        return _coords;
    }

    AssimpMeshFaceCollection::AssimpMeshFaceCollection(const aiMesh& mesh, const aiSceneRef& scene) noexcept
    {
        if (mesh.mFaces == nullptr)
        {
            return;
        }
        auto size = mesh.mNumFaces;
        _elements.reserve(size);
        for(size_t i = 0; i < size; i++)
        {
            _elements.emplace_back(mesh.mFaces[i], scene);
        }
    }

    AssimpMeshTextureCoordsCollection::AssimpMeshTextureCoordsCollection(const aiMesh& mesh) noexcept
    {
        size_t size = AI_MAX_NUMBER_OF_TEXTURECOORDS;
        _elements.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            if (mesh.mTextureCoords[i] != nullptr)
            {
                _elements.emplace_back(mesh, i);
            }
        }
    }

    AssimpMeshColorsCollection::AssimpMeshColorsCollection(const aiMesh& mesh) noexcept
    {
        size_t size = AI_MAX_NUMBER_OF_COLOR_SETS;
        _elements.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            if (mesh.mColors[i] != nullptr)
            {
                _elements.emplace_back(mesh.mColors[i], mesh.mNumVertices);
            }
        }
    }

    AssimpMesh::AssimpMesh(const aiMesh& mesh, const AssimpMaterial& material, const aiSceneRef& scene) noexcept
        : _mesh(mesh)
        , _material(material)
        , _scene(scene)
        , _faces(mesh, scene)
        , _texCoords(mesh)
        , _colors(mesh)
        , _positions(mesh.mVertices, mesh.mNumVertices)
        , _normals(mesh.mNormals, mesh.mNumVertices)
        , _tangents(mesh.mTangents, mesh.mNumVertices)
        , _bitangents(mesh.mBitangents, mesh.mNumVertices)
    {
    }

    const AssimpMaterial& AssimpMesh::getMaterial() const noexcept
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

    const AssimpMeshTextureCoordsCollection& AssimpMesh::getTexCoords() const noexcept
    {
        return _texCoords;
    }

    const size_t AssimpMesh::getVertexCount() const noexcept
    {
        return _mesh.mNumVertices;
    }

    Data AssimpMesh::createVertexData(const bgfx::VertexLayout& layout) const noexcept
    {
        VertexDataWriter writer(layout, getVertexCount());
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
        return writer.finish();
    }

    std::vector<VertexIndex> AssimpMesh::createIndexData() const noexcept
    {
        std::vector<VertexIndex> indices;
        for (auto& face : getFaces())
        {
            indices.insert(indices.end(), face.getIndices().begin(), face.getIndices().end());
        }
        return indices;
    }

    std::shared_ptr<Mesh> AssimpMesh::load(const bgfx::VertexLayout& layout, ITextureLoader& textureLoader, bx::AllocatorI& alloc) const noexcept
    {
        auto vertices = createVertexData(layout);
        auto indices = createIndexData();
        auto mesh = std::make_shared<Mesh>(layout, DataView(vertices), DataView(indices));
        mesh->setMaterial(getMaterial().load(textureLoader, alloc));
        return mesh;
    }

    const AssimpMeshFaceCollection& AssimpMesh::getFaces() const noexcept
    {
        return _faces;
    }

    const AssimpMeshColorsCollection& AssimpMesh::getColors() const noexcept
    {
        return _colors;
    }

    AssimpNodeMeshCollection::AssimpNodeMeshCollection(const aiNode& node, const AssimpMeshCollection& meshes) noexcept
    {
        auto size = node.mNumMeshes;
        _elements.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            auto j = node.mMeshes[i];
            if (j < meshes.size())
            {
                _elements.push_back(meshes[j]);
            }
        }
    }
    
    std::vector<std::shared_ptr<Mesh>> AssimpNodeMeshCollection::load(const bgfx::VertexLayout& layout, ITextureLoader& textureLoader, bx::AllocatorI& alloc) const noexcept
    {
        std::vector<std::shared_ptr<Mesh>> meshes;
        meshes.reserve(size());
        for (auto& mesh : *this)
        {
            meshes.push_back(mesh.load(layout, textureLoader, alloc));
        }
        return meshes;
    }

    AssimpCamera::AssimpCamera(const aiCamera& cam, const aiSceneRef& scene) noexcept
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

        auto clipNear = getClipNear();
        auto clipFar = getClipFar();
        bx::mtxProj(glm::value_ptr(_proj), bx::toDeg(fovy), aspect, clipNear, clipFar, bgfx::getCaps()->homogeneousDepth);
    }

    std::string_view AssimpCamera::getName() const noexcept
    {
        return AssimpUtils::getStringView(_cam.mName);
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
        return _cam.mAspect;
    }

    float AssimpCamera::getClipFar() const noexcept
    {
        return _cam.mClipPlaneFar;
    }

    float AssimpCamera::getClipNear() const noexcept
    {
        return _cam.mClipPlaneNear;
    }

    float AssimpCamera::getHorizontalFieldOfView() const noexcept
    {
        return _cam.mHorizontalFOV;
    }

    float AssimpCamera::getOrthographicWidth() const noexcept
    {
        return _cam.mOrthographicWidth;
    }

    glm::vec3 AssimpCamera::getLookAt() const noexcept
    {
        return AssimpUtils::convert(_cam.mLookAt);
    }

    glm::vec3 AssimpCamera::getPosition() const noexcept
    {
        return AssimpUtils::convert(_cam.mPosition);
    }

    glm::vec3 AssimpCamera::getUp() const noexcept
    {
        return AssimpUtils::convert(_cam.mUp);
    }

    AssimpLight::AssimpLight(const aiLight& light, const aiSceneRef& scene) noexcept
        : _light(light)
        , _scene(scene)
    {
    }

    std::string_view AssimpLight::getName() const noexcept
    {
        return AssimpUtils::getStringView(_light.mName);
    }

    float AssimpLight::getInnerConeAngle() const noexcept
    {
        return _light.mAngleInnerCone;
    }

    float AssimpLight::getOuterConeAngle() const noexcept
    {
        return _light.mAngleOuterCone;
    }

    glm::vec3 AssimpLight::getAttenuation() const noexcept
    {
        return {
            _light.mAttenuationConstant,
            _light.mAttenuationLinear,
            _light.mAttenuationQuadratic,
        };
    }

    Color3 AssimpLight::getAmbientColor() const noexcept
    {
        return AssimpUtils::convert(_light.mColorAmbient);
    }

    Color3 AssimpLight::getDiffuseColor() const noexcept
    {
        return AssimpUtils::convert(_light.mColorDiffuse);
    }

    Color3 AssimpLight::getSpecularColor() const noexcept
    {
        return AssimpUtils::convert(_light.mColorSpecular);
    }

    glm::vec3 AssimpLight::getDirection() const noexcept
    {
        return AssimpUtils::convert(_light.mDirection);
    }

    glm::vec3 AssimpLight::getPosition() const noexcept
    {
        return AssimpUtils::convert(_light.mPosition);
    }

    aiLightSourceType AssimpLight::getType() const noexcept
    {
        return _light.mType;
    }

    glm::vec2 AssimpLight::getSize() const noexcept
    {
        return AssimpUtils::convert(_light.mSize);
    }

    AssimpNodeChildrenCollection::AssimpNodeChildrenCollection(const aiNode& node, const AssimpScene& scene) noexcept
    {
        auto size = node.mNumChildren;
        _elements.reserve(size);
        for (size_t i = 0; i < size; i++)
        {
            _elements.emplace_back(*node.mChildren[i++], scene);
        }
    }

    AssimpNode::AssimpNode(const aiNode& node, const AssimpScene& scene) noexcept
        : _node(node)
        , _meshes(node, scene.getMeshes())
        , _children(node, scene)
        , _light(scene.getLights().get(getName()).optional())
        , _camera(scene.getCameras().get(getName()).optional())
    {
    }

    std::string_view AssimpNode::getName() const noexcept
    {
        return AssimpUtils::getStringView(_node.mName);
    }

    glm::mat4 AssimpNode::getTransform() const noexcept
    {
        return AssimpUtils::convert(_node.mTransformation);
    }

    const AssimpNodeMeshCollection& AssimpNode::getMeshes() const noexcept
    {
        return _meshes;
    }

    const AssimpNodeChildrenCollection& AssimpNode::getChildren() const noexcept
    {
        return _children;
    }

    OptionalRef<const AssimpNode> AssimpNode::getChild(const std::string_view& path) const noexcept
    {
        return AssimpUtils::getNodeChild(*this, path);
    }

    OptionalRef<const AssimpCamera> AssimpNode::getCamera() const noexcept
    {
        return _camera ? &_camera.value() : nullptr;
    }

    OptionalRef<const AssimpLight> AssimpNode::getLight() const noexcept
    {
        return _light ? &_light.value() : nullptr;
    }

    AssimpCameraCollection::AssimpCameraCollection(const aiSceneRef& scene) noexcept
    {
        auto size = scene->mNumCameras;
        _elements.reserve(size);
        size_t i = 0;
        for(size_t i = 0; i < size; i++)
        {
            _elements.emplace_back(*scene->mCameras[i], scene);
        }
    }

    OptionalRef<const AssimpCamera> AssimpCameraCollection::get(const std::string_view& name) const noexcept
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

    AssimpLightCollection::AssimpLightCollection(const aiSceneRef& scene) noexcept
    {
        auto size = scene->mNumLights;
        _elements.reserve(size);
        size_t i = 0;
        for (size_t i = 0; i < size; i++)
        {
            _elements.emplace_back(*scene->mLights[i], scene);
        }
    }

    OptionalRef<const AssimpLight> AssimpLightCollection::get(const std::string_view& name) const noexcept
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

    AssimpMaterialCollection::AssimpMaterialCollection(const aiSceneRef& scene, const std::string& basePath) noexcept
    {
        auto size = scene->mNumMaterials;
        _elements.reserve(size);
        size_t i = 0;
        for (size_t i = 0; i < size; i++)
        {
            _elements.emplace_back(*scene->mMaterials[i], scene, basePath);
        }
    }

    AssimpMeshCollection::AssimpMeshCollection(const aiSceneRef& scene, const AssimpMaterialCollection& materials) noexcept
    {
        auto size = scene->mNumMeshes;
        _elements.reserve(size);
        size_t i = 0;
        for (size_t i = 0; i < size; i++)
        {
            auto meshPtr = scene->mMeshes[i];
            if (meshPtr != nullptr && meshPtr->mMaterialIndex < materials.size())
            {
                auto& material = materials[meshPtr->mMaterialIndex];
                _elements.emplace_back(*meshPtr, material, scene);
            }
        }
    }

    AssimpScene::AssimpScene(const aiSceneRef& scene, const std::string& path) noexcept
        : _scene(scene)
        , _path(path)
        , _materials(scene, path)
        , _meshes(scene, _materials)
        , _cameras(scene)
        , _lights(scene)
        , _rootNode(*scene->mRootNode, *this)
    {
    }

    AssimpScene::AssimpScene(Assimp::Importer& importer, const DataView& data, const std::string& path)
        : AssimpScene(importScene(importer, data, path), path)
    {
    }

    aiSceneRef AssimpScene::importScene(Assimp::Importer& importer, const DataView& data, const std::string& path)
    {
        unsigned int flags = aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType |
            aiProcess_ConvertToLeftHanded
            ;
        // assimp (and opengl) is right handed (+Z points towards the camera)
        // while bgfx (and darmok and directx) is left handed (+Z points away from the camera)

        auto ptr = importer.ReadFileFromMemory(data.ptr(), data.size(), flags, path.c_str());
        if (ptr == nullptr)
        {
            throw std::runtime_error(importer.GetErrorString());
        }
        // take ownership of the scene
        return aiSceneRef(importer.GetOrphanedScene());
    }

    std::string_view AssimpScene::getName() const noexcept
    {
        return AssimpUtils::getStringView(_scene->mName);
    }

    const std::string& AssimpScene::getPath() const noexcept
    {
        return _path;
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
}
