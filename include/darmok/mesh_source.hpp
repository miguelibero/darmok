#pragma once

#include <darmok/mesh.hpp>
#include <darmok/shape.hpp>

#include <string>
#include <variant>

#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/string.hpp>

namespace darmok
{
    struct DARMOK_EXPORT CubeMeshSource final
    {
        Cube cube;
        RectangleMeshType type = RectangleMeshType::Full;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(cube), CEREAL_NVP(type));
        }
    };

    struct DARMOK_EXPORT SphereMeshSource final
    {
        Sphere sphere;
        unsigned int lod = 32;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(sphere), CEREAL_NVP(lod));
        }
    };

    struct DARMOK_EXPORT CapsuleMeshSource final
    {
        Capsule capsule;
        unsigned int lod = 32;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(capsule), CEREAL_NVP(lod));
        }
    };

    struct DARMOK_EXPORT RectangleMeshSource final
    {
        Rectangle rectangle;
        RectangleMeshType type = RectangleMeshType::Full;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(rectangle), CEREAL_NVP(type));
        }
    };

    struct DARMOK_EXPORT PlaneMeshSource final
    {
        Plane plane;
        RectangleMeshType type = RectangleMeshType::Full;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(plane), CEREAL_NVP(type));
        }
    };

    struct DARMOK_EXPORT ModelMeshSource final
    {
        template<class Archive>
        void serialize(Archive& archive)
        {
        }
    };

    class MeshDefinition;

    struct DARMOK_EXPORT MeshSource final
    {
        std::string name;
        MeshConfig config;

        using Content = std::variant<
            CubeMeshSource,
            SphereMeshSource,
            CapsuleMeshSource,
            RectangleMeshSource,
            PlaneMeshSource,
            ModelMeshSource
        >;
        Content content;

        std::shared_ptr<MeshDefinition> createDefinition(const bgfx::VertexLayout& layout);

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(name),
                CEREAL_NVP(config),
                CEREAL_NVP(content)
            );
        }
    };
}