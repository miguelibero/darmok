#include <darmok/assimp.hpp>
#include <darmok/shape.hpp>
#include <darmok/glm_serialize.hpp>

#include <assimp/Importer.hpp>
#include <assimp/BaseImporter.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <glm/gtx/component_wise.hpp>

namespace darmok
{
    namespace AssimpUtils
    {
        uint8_t convertColorComp(ai_real v) noexcept
        {
            return 255 * v;
        }

        float getIntensity(const aiColor3D& c) noexcept
        {
            return glm::compMax(glm::vec3{ c.r, c.g, c.b });
        }
    };

    std::string_view Converter<std::string_view, aiString>::run(const aiString& v) noexcept
    {
        return std::string_view{ v.data, v.length };
    }

    std::string Converter<std::string, aiString>::run(const aiString& v) noexcept
    {
        return std::string{ v.data, v.length };
    }

    glm::mat4 Converter<glm::mat4, aiMatrix4x4>::run(const aiMatrix4x4& v) noexcept
    {
        // the a,b,c,d in assimp is the row; the 1,2,3,4 is the column
        return glm::mat4{
            v.a1, v.b1, v.c1, v.d1,
            v.a2, v.b2, v.c2, v.d2,
            v.a3, v.b3, v.c3, v.d3,
            v.a4, v.b4, v.c4, v.d4
        };
    }

    glm::vec3 Converter<glm::vec3, aiVector3D>::run(const aiVector3D& v) noexcept
    {
        return glm::vec3{ v.x, v.y, v.z };
    }

    glm::vec2 Converter<glm::vec2, aiVector2D>::run(const aiVector2D& v) noexcept
    {
        return glm::vec2{ v.x, v.y };
    }

    Color Converter<Color, aiColor4D>::run(const aiColor4D& v) noexcept
    {
        return {
            AssimpUtils::convertColorComp(v.r),
            AssimpUtils::convertColorComp(v.g),
            AssimpUtils::convertColorComp(v.b),
            AssimpUtils::convertColorComp(v.a),
        };
    }

    Color3 Converter<Color3, aiColor3D>::run(const aiColor3D& v) noexcept
    {
        return {
            AssimpUtils::convertColorComp(v.r),
            AssimpUtils::convertColorComp(v.g),
            AssimpUtils::convertColorComp(v.b)
        };
    }

    protobuf::Polygon Converter<protobuf::Polygon, aiMesh>::run(const aiMesh& v) noexcept
    {
        auto getVertex = [&v](size_t i)
        {
            return convert<protobuf::Vec3>(convert<glm::vec3>(v.mVertices[i]));
        };

        protobuf::Polygon polygon;
        auto& tris = *polygon.mutable_triangles();
        tris.Reserve(v.mNumFaces);
        for (size_t i = 0; i < v.mNumFaces; ++i)
        {
            auto& face = v.mFaces[i];
            if (face.mNumIndices != 3)
            {
                continue;
            }
            auto& tri = *tris.Add();
            *tri.mutable_vertex1() = getVertex(face.mIndices[0]);
            *tri.mutable_vertex2() = getVertex(face.mIndices[1]);
            *tri.mutable_vertex3() = getVertex(face.mIndices[2]);
        }
        return polygon;
    };

    class AssimpLoaderImpl final
    {
    public:
        using Config = AssimpLoader::Config;
        using Result = AssimpLoader::Result;
        bool supports(const std::filesystem::path& path) const noexcept;
        Result loadFromFile(const std::filesystem::path& path, const Config& config = {}) const ;
        Result loadFromMemory(DataView data, const Config& config = {}) const;
    private:
        static unsigned int getImporterFlags(const Config& config = {}) noexcept;
        static std::shared_ptr<aiScene> fixScene(Assimp::Importer& importer) noexcept;
    };

    bool AssimpLoaderImpl::supports(const std::filesystem::path& path) const noexcept
    {
        Assimp::Importer importer;
        return importer.IsExtensionSupported(path.extension().string());
    }

    unsigned int AssimpLoaderImpl::getImporterFlags(const Config& config) noexcept
    {
        auto flags = // aiProcess_CalcTangentSpace | // produces weird tangents, we use mikktspace instead
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType |
            aiProcess_GenSmoothNormals |
            aiProcess_GenBoundingBoxes |
            aiProcess_LimitBoneWeights |
            aiProcess_FlipUVs |
            // apply UnitScaleFactor to everything
            aiProcess_GlobalScale;

        if (config.leftHanded)
        {
            // assimp (and opengl) is right handed (+Z points towards the camera)
            // while bgfx (and darmok and directx) is left handed (+Z points away from the camera)
            flags |= aiProcess_MakeLeftHanded;
        }
        if (config.populateArmature)
        {
            flags |= aiProcess_PopulateArmatureData;
        }
        return flags;
    }

    AssimpLoader::Result AssimpLoaderImpl::loadFromFile(const std::filesystem::path& path, const Config& config) const
    {
        try
        {
            Assimp::Importer importer;
            const aiScene* ptr = nullptr;
            if (!config.format.empty())
            {
                auto hintImporter = importer.GetImporter(config.format.c_str());
                if (hintImporter != nullptr)
                {
                    ptr = hintImporter->ReadFile(&importer, path.string().c_str(), importer.GetIOHandler());
                }
            }
            if (ptr == nullptr)
            {
                ptr = importer.ReadFile(path.string(), getImporterFlags(config));
            }
            if (ptr == nullptr)
            {
                return unexpected{ importer.GetErrorString() };
            }
            return fixScene(importer);
        }
        catch (const std::exception& e)
        {
            return unexpected{ e.what() };
        }
    }

    AssimpLoader::Result AssimpLoaderImpl::loadFromMemory(DataView data, const Config& config) const
    {
        Assimp::Importer importer;
        if (!config.basePath.empty())
        {
            auto path = std::filesystem::path(config.basePath) / "file";
            if (!config.format.empty())
            {
                path += "." + config.format;
            }
            importer.SetPropertyString("sourceFilePath", path.string());
        }

        try
        {
            auto ptr = importer.ReadFileFromMemory(data.ptr(), data.size(), getImporterFlags(config), config.format.c_str());
            if (ptr == nullptr)
            {
                return unexpected{ importer.GetErrorString() };
            }
            return fixScene(importer);
        }
        catch(const std::exception& e)
        {
            return unexpected{ e.what() };
		}
    }

    std::shared_ptr<aiScene> AssimpLoaderImpl::fixScene(Assimp::Importer& importer) noexcept
    {
        auto scene = importer.GetOrphanedScene();

        // https://github.com/assimp/assimp/issues/3240
        auto scale = importer.GetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);
        scale *= importer.GetPropertyFloat(AI_CONFIG_APP_SCALE_KEY, 1.0f);

        // scale camera clip planes, seems to be an assimp bug
        for (auto i = 0; i < scene->mNumCameras; ++i)
        {
            auto cam = scene->mCameras[i];
            cam->mClipPlaneNear *= scale;
            cam->mClipPlaneFar *= scale;
        }

        // scale light parameters
        for (auto i = 0; i < scene->mNumLights; ++i)
        {
            auto light = scene->mLights[i];
            light->mColorAmbient = light->mColorAmbient * scale;
            light->mColorDiffuse = light->mColorDiffuse * scale;
            light->mColorSpecular = light->mColorSpecular * scale;
            light->mAttenuationLinear /= scale;
            light->mAttenuationQuadratic /= scale * scale;
        }
        return std::shared_ptr<aiScene>(scene);
    }

    void AssimpLoader::Config::setPath(const std::filesystem::path& path) noexcept
    {
        basePath = path.parent_path().string();
        format = path.extension().string();
    }

    AssimpLoader::AssimpLoader() noexcept
        : _impl{std::make_unique<AssimpLoaderImpl>()}
    {
    }

    AssimpLoader::~AssimpLoader() = default;

    bool AssimpLoader::supports(const std::filesystem::path& path) const noexcept
    {
        return _impl->supports(path);
    }

    AssimpLoader::Result AssimpLoader::loadFromFile(const std::filesystem::path& path, const Config& config) const
    {
        return _impl->loadFromFile(path, config);
    }

    AssimpLoader::Result AssimpLoader::loadFromMemory(DataView data, const Config& config) const
    {
        return _impl->loadFromMemory(data, config);
    }
}