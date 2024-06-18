#include "model_assimp.hpp"
#include <filesystem>
#include <fstream>
#include <bx/allocator.h>
#include <bx/file.h>

#include <darmok/model_assimp.hpp>
#include <darmok/model.hpp>
#include <darmok/image.hpp>
#include <darmok/data.hpp>
#include <darmok/vertex.hpp>
#include <darmok/program_standard.hpp>
#include <darmok/string.hpp>

#include <assimp/vector3.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>

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

    AssimpModelLoaderImpl::AssimpModelLoaderImpl(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader) noexcept
        : _config{
            .dataLoader = dataLoader,
            .allocator = allocator,
            .imgLoader = imgLoader
        }
    {
    }

    void AssimpModelLoaderImpl::setVertexLayout(const bgfx::VertexLayout& vertexLayout) noexcept
    {
        _config.vertexLayout = vertexLayout;
    }

    std::shared_ptr<Model> AssimpModelLoaderImpl::operator()(std::string_view path)
    {
        auto data = _config.dataLoader(path);

        static const unsigned int flags = aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType |
            // apply UnitScaleFactor to everything
            aiProcess_GlobalScale |
            // aiProcess_PopulateArmatureData |
            // assimp (and opengl) is right handed (+Z points towards the camera)
            // while bgfx (and darmok and directx) is left handed (+Z points away from the camera)
            aiProcess_ConvertToLeftHanded
            ;

        Assimp::Importer importer;
        std::string pathStr(path);
        auto ptr = importer.ReadFileFromMemory(data.ptr(), data.size(), flags, pathStr.c_str());
        if (ptr == nullptr)
        {
            throw std::runtime_error(importer.GetErrorString());
        }

        auto scene = importer.GetScene();

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

        auto model = std::make_shared<Model>();

        auto basePath = std::filesystem::path(path).parent_path().string();
        AssimpModelLoaderContext ctxt(*scene, basePath, _config);
        ctxt.update(model->rootNode, *scene->mRootNode);

        return model;
    }

    AssimpModelLoaderContext::AssimpModelLoaderContext(const aiScene& scene, const std::string& basePath, const AssimpModelLoaderConfig& config) noexcept
        : _scene(scene)
        , _basePath(basePath)
        , _config(config)
    {
        // TODO: check that we need this for the inverse bind poses
        _inverseRoot = glm::inverse(AssimpUtils::convert(scene.mRootNode->mTransformation));
    }

    void AssimpModelLoaderContext::update(ModelNode& modelNode, const aiNode& assimpNode) noexcept
    {
        modelNode.name = AssimpUtils::getStringView(assimpNode.mName);
        modelNode.transform = AssimpUtils::convert(assimpNode.mTransformation);
        for(size_t i = 0; i < assimpNode.mNumMeshes; i++)
        {
            auto& modelRenderable = modelNode.renderables.emplace_back();
            auto assimpMesh = _scene.mMeshes[assimpNode.mMeshes[i]];
            modelRenderable.mesh = getMesh(assimpMesh);
            auto assimpMaterial = _scene.mMaterials[assimpMesh->mMaterialIndex];
            modelRenderable.material = getMaterial(assimpMaterial);
        }

        for (size_t i = 0; i < _scene.mNumCameras; i++)
        {
            auto assimpCam = _scene.mCameras[i];
            if (assimpCam->mName == assimpNode.mName)
            {
                update(modelNode, *assimpCam);
                break;
            }
        }
        for (size_t i = 0; i < _scene.mNumLights; i++)
        {
            auto assimpLight = _scene.mLights[i];
            if (assimpLight->mName == assimpNode.mName)
            {
                update(modelNode, *assimpLight);
                break;
            }
        }

        for (size_t i = 0; i < assimpNode.mNumChildren; i++)
        {
            auto& modelChild = modelNode.children.emplace_back();
            update(modelChild, *assimpNode.mChildren[i]);
        }
    }

    void AssimpModelLoaderContext::update(ModelNode& modelNode, const aiCamera& assimpCam) noexcept
    {
        auto& camNode = modelNode.children.emplace_back();
        camNode.name = AssimpUtils::getStringView(assimpCam.mName);

        aiMatrix4x4 mat;
        assimpCam.GetCameraMatrix(mat);
        camNode.transform = AssimpUtils::convert(mat);

        auto aspect = assimpCam.mAspect;
        auto fovy = 0.f;
        if (aspect != 0.f)
        {
            auto fovx = assimpCam.mHorizontalFOV;
            fovy = 2.f * atan(tan(0.5f * fovx) * aspect);
        }

        auto& cam = camNode.camera.emplace();
        cam.projection = Math::perspective(fovy, aspect, assimpCam.mClipPlaneNear, assimpCam.mClipPlaneFar);
    }

    void AssimpModelLoaderContext::update(ModelNode& modelNode, const aiLight& assimpLight) noexcept
    {
        auto& lightNode = modelNode.children.emplace_back();
        lightNode.name = AssimpUtils::getStringView(assimpLight.mName);
        auto pos = AssimpUtils::convert(assimpLight.mPosition);
        lightNode.transform = glm::translate(glm::mat4(1), pos);

        if (assimpLight.mType == aiLightSource_POINT)
        {
            auto& pointLight = lightNode.pointLight.emplace();
            pointLight.attenuation = glm::vec3(
                assimpLight.mAttenuationConstant,
                assimpLight.mAttenuationLinear,
                assimpLight.mAttenuationQuadratic
            );
            pointLight.diffuseColor = AssimpUtils::convert(assimpLight.mColorDiffuse);
            pointLight.specularColor = AssimpUtils::convert(assimpLight.mColorSpecular);
        }
        else if (assimpLight.mType == aiLightSource_AMBIENT)
        {
            auto& ambLight = lightNode.ambientLight.emplace();
            ambLight.color = AssimpUtils::convert(assimpLight.mColorAmbient);
        }
    }

    void AssimpModelLoaderContext::update(ModelTexture& modelTex, const aiMaterial& assimpMat, aiTextureType type, unsigned int index) noexcept
    {
        aiString path("");
        unsigned int uvindex = 0;
        ai_real blend = 1.F;
        aiTextureMapping mapping = aiTextureMapping::aiTextureMapping_UV;
        aiTextureOp operation = aiTextureOp::aiTextureOp_Multiply;

        assimpMat.GetTexture(type, index, &path,
            &mapping,
            &uvindex,
            &blend,
            &operation
        );

        modelTex.image = getImage(path.C_Str());
        // TODO: add support for other texture params
    }

    const std::unordered_map<aiTextureType, MaterialTextureType> AssimpModelLoaderContext::_materialTextures =
    {
        { aiTextureType_DIFFUSE, MaterialTextureType::Diffuse },
        { aiTextureType_SPECULAR, MaterialTextureType::Specular },
        { aiTextureType_NORMALS, MaterialTextureType::Normal },
    };

    void AssimpModelLoaderContext::update(ModelMaterial& modelMat, const aiMaterial& assimpMat) noexcept
    {
        modelMat.standardProgram = _config.standardProgram;
        for (auto& elm : _materialTextures)
        {
            auto size = assimpMat.GetTextureCount(elm.first);
            auto& modelTextures = modelMat.textures[elm.second];
            for(size_t i = 0; i < size; i++)
            {
                auto& modelTexture = modelTextures.emplace_back();
                update(modelTexture, assimpMat, elm.first, 0);
            }
        }
    }

    Data AssimpModelLoaderContext::createVertexData(const aiMesh& assimpMesh) const noexcept
    {
        auto vertexCount = assimpMesh.mNumVertices;
        VertexDataWriter writer(_config.vertexLayout, vertexCount, _config.allocator);

        for(size_t i = 0; i < assimpMesh.mNumVertices; i++)
        {
            if(assimpMesh.mVertices)
            {
                writer.write(bgfx::Attrib::Position, i, AssimpUtils::convert(assimpMesh.mVertices[i]));
            }
            if(assimpMesh.mNormals)
            {
                writer.write(bgfx::Attrib::Normal, i, AssimpUtils::convert(assimpMesh.mNormals[i]));
            }
            if(assimpMesh.mTangents)
            {
                writer.write(bgfx::Attrib::Tangent, i, AssimpUtils::convert(assimpMesh.mTangents[i]));
            }
            if(assimpMesh.mBitangents)
            {
                writer.write(bgfx::Attrib::Bitangent, i, AssimpUtils::convert(assimpMesh.mBitangents[i]));
            }
            for(size_t j = 0; j < AI_MAX_NUMBER_OF_COLOR_SETS; j++)
            {
                if(assimpMesh.mColors[j])
                {
                    auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::Color0 + j);
                    writer.write(attrib, i, AssimpUtils::convert(assimpMesh.mColors[j][i]));
                }
            }
            for(size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; j++)
            {
                if(assimpMesh.mTextureCoords[j])
                {
                    auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::TexCoord0 + j);
                    writer.write(attrib, i, AssimpUtils::convert(assimpMesh.mTextureCoords[j][i]));
                }
            }
        }

        if(assimpMesh.HasBones())
        {
            int boneIndex = 0;
            std::vector<int> boneCount(vertexCount);
            std::vector<glm::vec4> boneIndices(vertexCount);
            std::fill(boneIndices.begin(), boneIndices.end(), glm::vec4(-1));
            std::vector<glm::vec4> boneWeights(vertexCount);
            std::fill(boneWeights.begin(), boneWeights.end(), glm::vec4(1, 0, 0, 0));
            for(size_t i = 0; i < assimpMesh.mNumBones; i++)
            {
                auto bone = assimpMesh.mBones[i];
                for(size_t j = 0; j < bone->mNumWeights; j++)
                {
                    auto& weight = bone->mWeights[j];
                    auto i = weight.mVertexId;
                    auto c = boneCount[i]++;
                    if (c <= 3)
                    {
                        boneIndices[i][c] = boneIndex;
                        boneWeights[i][c] = weight.mWeight;
                    }
                }
                ++boneIndex;
            }
            writer.write(bgfx::Attrib::Indices, boneIndices);
            writer.write(bgfx::Attrib::Weight, boneWeights);
        }

        return writer.finish();
    }

    std::vector<VertexIndex> AssimpModelLoaderContext::createIndexData(const aiMesh& assimpMesh) const noexcept
    {
        size_t size = 0;
        for(size_t i = 0; i < assimpMesh.mNumFaces; i++)
        {
            size += assimpMesh.mFaces[i].mNumIndices;
        }
        std::vector<VertexIndex> indices;
        indices.reserve(size);
        for(size_t i = 0; i < assimpMesh.mNumFaces; i++)
        {
            auto& face = assimpMesh.mFaces[i];
            for(size_t j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }
        return indices;
    }

    void AssimpModelLoaderContext::update(ModelMesh& modelMesh, const aiMesh& assimpMesh) noexcept
    {
        modelMesh.vertexData = createVertexData(assimpMesh);
        modelMesh.indexData = createIndexData(assimpMesh);
        modelMesh.vertexLayout = _config.vertexLayout;

        for(size_t i = 0; i < assimpMesh.mNumBones; i++)
        {
            auto bone = assimpMesh.mBones[i];
            modelMesh.joints.push_back(ModelArmatureJoint{
                std::string(bone->mName.C_Str()),
                AssimpUtils::convert(bone->mOffsetMatrix) * _inverseRoot
            });
        }
    }
    
    std::shared_ptr<ModelMesh> AssimpModelLoaderContext::getMesh(const aiMesh* assimpMesh) noexcept
    {
        auto itr = _meshes.find(assimpMesh);
        if (itr != _meshes.end())
        {
            return itr->second;
        }
        auto modelMesh = std::make_shared<ModelMesh>();
        update(*modelMesh, *assimpMesh);
        _meshes.emplace(assimpMesh, modelMesh);
        return modelMesh;
    }

    std::shared_ptr<ModelMaterial> AssimpModelLoaderContext::getMaterial(const aiMaterial* assimpMat) noexcept
    {
        auto itr = _materials.find(assimpMat);
        if (itr != _materials.end())
        {
            return itr->second;
        }
        auto modelMat = std::make_shared<ModelMaterial>();
        update(*modelMat, *assimpMat);
        _materials.emplace(assimpMat, modelMat);
        return modelMat;
    }

    std::shared_ptr<ModelImage> AssimpModelLoaderContext::getImage(const std::string& path) noexcept
    {
        auto itr = _images.find(path);
        if (itr != _images.end())
        {
            return itr->second;
        }
        std::shared_ptr<Image> img;
        auto assimpTex = _scene.GetEmbeddedTexture(path.c_str());
        if (assimpTex)
        {
            size_t size;
            auto format = bimg::TextureFormat::Count;
            if (assimpTex->mHeight == 0)
            {
                // compressed;
                size = assimpTex->mWidth;
            }
            else
            {
                size = assimpTex->mWidth * assimpTex->mHeight * 4;
                format = bimg::TextureFormat::RGBA8;
            }
            auto data = DataView(assimpTex->pcData, size);
            img = std::make_shared<Image>(data, _config.allocator, format);
        }
        if (!img && _config.imgLoader)
        {
            std::filesystem::path fsPath(path);
            if (fsPath.is_relative())
            {
                fsPath = _basePath / fsPath;
            }
            img = (*_config.imgLoader)(fsPath.string());
        }

        auto modelImg = std::make_shared<ModelImage>(
            img->getData(), path, img->getTextureConfig());
        _images.emplace(path, modelImg);
        return modelImg;
    }

    AssimpModelLoader::AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader) noexcept
        : _impl(std::make_unique<AssimpModelLoaderImpl>(dataLoader, allocator, imgLoader))
    {
    }

    AssimpModelLoader::~AssimpModelLoader() noexcept
    {
        // empty to forward declare the impl pointer
    }

    void AssimpModelLoader::setVertexLayout(const bgfx::VertexLayout& vertexLayout) noexcept
    {
        _impl->setVertexLayout(vertexLayout);
    }

    AssimpModelLoader::result_type AssimpModelLoader::operator()(std::string_view name)
    {
        return (*_impl)(name);
    }

    AssimpModelProcessor::AssimpModelProcessor(const std::string& inputPath)
        : _inputPath(inputPath)
        , _embedTextures(false)
        , _vertexLayout(StandardProgramLoader::getVertexLayout(StandardProgramType::ForwardPhong))
    {
    }

    AssimpModelProcessor& AssimpModelProcessor::setConfigFromFile(const std::string& path)
    {
        std::ifstream ifs(path);
        auto json = nlohmann::ordered_json::parse(ifs);
        return setConfig(json);
    }

    const char* AssimpModelProcessor::_vertexLayoutJsonKey = "vertex_layout";
    const char* AssimpModelProcessor::_embedTexturesJsonKey = "embed_textures";

    bgfx::VertexLayout AssimpModelProcessor::loadVertexLayout(const nlohmann::ordered_json& json)
    {
        bgfx::VertexLayout layout;
        if (json.is_string())
        {
            std::string str = json;
            auto standard = StandardProgramLoader::getType(str);
            if (standard)
            {
                return StandardProgramLoader::getVertexLayout(standard.value());
            }
            VertexLayoutUtils::readFile(str, layout);
            return layout;
        }
        VertexLayoutUtils::readJson(json, layout);
        return layout;
    }

    AssimpModelProcessor& AssimpModelProcessor::setConfig(const nlohmann::ordered_json& json)
    {
        if (json.contains(_vertexLayoutJsonKey))
        {
            _vertexLayout = loadVertexLayout(json[_vertexLayoutJsonKey]);
        }
        if (json.contains(_embedTexturesJsonKey))
        {
            _embedTextures = json[_embedTexturesJsonKey];
        }
        _model = nullptr;
        return *this;
    }

    AssimpModelProcessor& AssimpModelProcessor::setHeaderVarName(const std::string& name) noexcept
    {
        _headerVarName = name;
        return *this;
    }

    std::shared_ptr<Model> AssimpModelProcessor::loadModel() const
    {
        if (_model != nullptr)
        {
            return _model;
        }
        bx::DefaultAllocator allocator;
        bx::FileReader fileReader;
        FileDataLoader dataLoader(fileReader, allocator);
        DataImageLoader imgLoader(dataLoader, allocator);
        auto optImgLoader = _embedTextures ? OptionalRef<IImageLoader>(imgLoader) : nullptr;
        AssimpModelLoader assimpLoader(dataLoader, allocator, optImgLoader);
        assimpLoader.setVertexLayout(_vertexLayout);
        _model = assimpLoader(_inputPath);
        return _model;
    }

    void AssimpModelProcessor::writeHeader(std::ostream& os, const std::string varName) const
    {
        Data data;
        DataOutputStream::write(data, *loadModel());
        os << data.view().toHeader(varName);
    }

    std::string AssimpModelProcessor::to_string() const noexcept
    {
        auto model = loadModel();
        std::stringstream ss;
        if (!_headerVarName.empty())
        {
            writeHeader(ss, _headerVarName);
        }
        else
        {
            ss << model;
        }
        return ss.str();
    }

    void AssimpModelProcessor::writeFile(const std::string& outputPath)
    {
        auto outExt = StringUtils::getPathExtension(outputPath);

        std::string headerVarName = _headerVarName;
        if (headerVarName.empty())
        {
            if (outExt == ".h" || outExt == ".hpp")
            {
                std::string outputStr(outputPath);
                headerVarName = outputStr.substr(0, outputStr.size() - outExt.size());
                headerVarName += "_model";
            }
        }

        auto& model = *loadModel();
        if (!headerVarName.empty())
        {
            std::ofstream os(outputPath);
            writeHeader(os, headerVarName);
        }
        else if (outExt == ".json")
        {
            std::ofstream os(outputPath);
            cereal::JSONOutputArchive archive(os);
            archive(model);
        }
        else if (outExt == ".xml")
        {
            std::ofstream os(outputPath);
            cereal::XMLOutputArchive archive(os);
            archive(model);
        }
        else
        {
            std::ofstream os(outputPath, std::ios::binary);
            cereal::BinaryOutputArchive archive(os);
            archive(model);
        }
    }
}

std::ostream& operator<<(std::ostream& out, const darmok::AssimpModelProcessor& process) noexcept
{
    return out << process.to_string();
}