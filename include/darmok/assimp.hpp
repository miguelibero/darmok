#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <darmok/convert.hpp>

#include <string>
#include <filesystem>
#include <memory>

#include <glm/glm.hpp>

struct aiScene;
struct aiMesh;
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
        float getIntensity(const aiColor3D& c) noexcept;
    }

    namespace protobuf
    {
        class Polygon;
    }

    template<>
    struct Converter<std::string_view, aiString>
    {
        static std::string_view run(const aiString& v) noexcept;
    };

    template<>
    struct Converter<std::string, aiString>
    {
        static std::string run(const aiString& v) noexcept;
    };

    template<>
    struct Converter<glm::mat4, aiMatrix4x4>
    {
        static glm::mat4 run(const aiMatrix4x4& v) noexcept;
    };

    template<>
    struct Converter<glm::vec3, aiVector3D>
    {
        static glm::vec3 run(const aiVector3D& v) noexcept;
    };

    template<>
    struct Converter<glm::vec2, aiVector2D>
    {
        static glm::vec2 run(const aiVector2D& v) noexcept;
    };

    template<>
    struct Converter<Color, aiColor4D>
    {
        static Color run(const aiColor4D& v) noexcept;
    };

    template<>
    struct Converter<Color3, aiColor3D>
    {
        static Color3 run(const aiColor3D& v) noexcept;
    };

    template<>
    struct Converter<protobuf::Polygon, aiMesh>
    {
        static protobuf::Polygon run(const aiMesh& v) noexcept;
    };

    class AssimpLoaderImpl;

    class DARMOK_EXPORT AssimpLoader final
    {
    public:
        struct Config final
        {
            bool leftHanded = true;
            bool populateArmature = false;
            std::string basePath = {};
            std::string format = {};

            void setPath(const std::filesystem::path& path) noexcept;
        };

        using Result = expected<std::shared_ptr<aiScene>, std::string>;

		AssimpLoader() noexcept;
		~AssimpLoader() noexcept;
        bool supports(const std::filesystem::path& path) const noexcept;
        Result loadFromFile(const std::filesystem::path& path, const Config& config) const noexcept;
        Result loadFromMemory(DataView data, const Config& config) const noexcept;
    private:
		std::unique_ptr<AssimpLoaderImpl> _impl;
    };
}