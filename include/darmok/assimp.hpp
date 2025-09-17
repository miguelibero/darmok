#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>

#include <string>
#include <filesystem>
#include <memory>

#include <glm/glm.hpp>

struct aiScene;
struct aiString;
using ai_real = float;
template <typename TReal> class aiVector2t;
typedef aiVector2t<ai_real> aiVector2D;
template <typename TReal> class aiVector3t;
typedef aiVector3t<ai_real> aiVector3D;
template <typename TReal> class aiColor4t;
typedef aiColor4t<ai_real> aiColor4D;
struct aiColor3D;
template <typename TReal> class aiMatrix4x4t;
typedef aiMatrix4x4t<ai_real> aiMatrix4x4;


namespace darmok
{
    namespace AssimpUtils
    {
        std::string_view getStringView(const aiString& str) noexcept;
        std::string getString(const aiString& str) noexcept;
        glm::mat4 convert(const aiMatrix4x4& from) noexcept;
        glm::vec3 convert(const aiVector3D& vec) noexcept;
        glm::vec2 convert(const aiVector2D& vec) noexcept;
        Color convert(const aiColor4D& c) noexcept;
        Color3 convert(aiColor3D c) noexcept;
    }

    class AssimpLoaderImpl;

    class DARMOK_EXPORT AssimpLoader final
    {
    public:
        struct Config final
        {
            bool leftHanded = true;
            bool populateArmature = false;
            std::string basePath;
            std::string format;

            void setPath(const std::filesystem::path& path) noexcept;
        };

        using Result = expected<std::shared_ptr<aiScene>, std::string>;

		AssimpLoader();
		~AssimpLoader();
        bool supports(const std::filesystem::path& path) const noexcept;
        Result loadFromFile(const std::filesystem::path& path, const Config& config = {}) const;
        Result loadFromMemory(DataView data, const Config& config = {}) const;
    private:
		std::unique_ptr<AssimpLoaderImpl> _impl;
    };
}