#include <darmok/assimp.hpp>

#include <assimp/Importer.hpp>
#include <assimp/BaseImporter.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace darmok
{
    namespace AssimpUtils
    {
        std::string_view getStringView(const aiString& str) noexcept
        {
            return std::string_view{ str.data, str.length };
        }

        std::string getString(const aiString& str) noexcept
        {
            return std::string{ str.data, str.length };
        }

        glm::mat4 convert(const aiMatrix4x4& from) noexcept
        {
            // the a,b,c,d in assimp is the row; the 1,2,3,4 is the column
            return glm::mat4{
                from.a1, from.b1, from.c1, from.d1,
                from.a2, from.b2, from.c2, from.d2,
                from.a3, from.b3, from.c3, from.d3,
                from.a4, from.b4, from.c4, from.d4
            };
        }

        glm::vec3 convert(const aiVector3D& vec) noexcept
        {
            return glm::vec3{ vec.x, vec.y, vec.z };
        }

        glm::vec2 convert(const aiVector2D& vec) noexcept
        {
            return glm::vec2{ vec.x, vec.y };
        }

        uint8_t convertColorComp(ai_real v) noexcept
        {
            return 255 * v;
        }

        Color convert(const aiColor4D& c) noexcept
        {
            return Color
            {
                convertColorComp(c.r),
                convertColorComp(c.g),
                convertColorComp(c.b),
                convertColorComp(c.a),
            };
        }

        Color3 convert(aiColor3D c) noexcept
        {
            return Color3
            {
                convertColorComp(c.r),
                convertColorComp(c.g),
                convertColorComp(c.b)
            };
        }
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
            // apply UnitScaleFactor to everything
            aiProcess_GlobalScale;

        if (config.leftHanded)
        {
            // assimp (and opengl) is right handed (+Z points towards the camera)
            // while bgfx (and darmok and directx) is left handed (+Z points away from the camera)
            flags |= aiProcess_ConvertToLeftHanded;
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

    AssimpLoader::AssimpLoader()
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