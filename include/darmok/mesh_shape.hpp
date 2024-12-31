#pragma once

#include <darmok/mesh_fwd.hpp>
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

    struct DARMOK_EXPORT MeshSource final
    {
        std::string name;
        using Shape = std::variant<CubeMeshSource, SphereMeshSource, CapsuleMeshSource, RectangleMeshSource, PlaneMeshSource>;
        Shape shape;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(name), CEREAL_NVP(shape));
        }
    };
}