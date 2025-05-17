#pragma once

#include <darmok/shape.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/shape.pb.h>

namespace darmok
{
    namespace protobuf
    {
        darmok::BoundingBox convert(const protobuf::BoundingBox& v);
        darmok::Cube convert(const protobuf::Cube& v);
        darmok::Sphere convert(const protobuf::Sphere& v);
        darmok::Capsule convert(const protobuf::Capsule& v);
        darmok::Rectangle convert(const protobuf::Rectangle& v);
        darmok::Plane convert(const protobuf::Plane& v);

        protobuf::BoundingBox convert(const darmok::BoundingBox& v);
        protobuf::Cube convert(const darmok::Cube& v);
        protobuf::Sphere convert(const darmok::Sphere& v);
        protobuf::Capsule convert(const darmok::Capsule& v);
        protobuf::Rectangle convert(const darmok::Rectangle& v);
        protobuf::Plane  convert(const darmok::Plane& v);
    }
}