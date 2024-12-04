#pragma once

#include <darmok/export.h>
#include <darmok/shape.hpp>
#include <darmok/mesh_fwd.hpp>

#include <memory>

#include <cereal/cereal.hpp>

namespace darmok
{
    class Program;
    class Material;

    class CubeRenderable final
    {
    public:
        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("cube", _cube),
                CEREAL_NVP_("type", _type)
            );
        }

        static void bindMeta();

    private:
        Cube _cube;
        RectangleMeshType _type;
    };

    class SphereRenderable final
    {
    public:
        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("sphere", _sphere),
                CEREAL_NVP_("lod", _lod)
            );
        }

        static void bindMeta();

    private:
        Sphere _sphere;
        unsigned int _lod;
    };

    class CapsuleRenderable final
    {
    public:
        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("capsule", _capsule),
                CEREAL_NVP_("lod", _lod)
            );
        }

        static void bindMeta();

    private:
        Capsule _capsule;
        unsigned int _lod;
    };

    class PlaneRenderable final
    {
    public:
        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("plane", _plane),
                CEREAL_NVP_("type", _type)
            );
        }

        static void bindMeta();

    private:
        Plane _plane;
        RectangleMeshType _type;
    };
}