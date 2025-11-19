#include <darmok/shape_serialize.hpp>
#include <darmok/glm_serialize.hpp>

namespace darmok
{
    namespace protobuf
    {
        darmok::BoundingBox convert(const protobuf::BoundingBox& v)
        {
            return { convert(v.min()), convert(v.max()) };
        }

        darmok::Cube convert(const protobuf::Cube& v)
        {
			return { convert(v.size()), convert(v.origin()) };
        }

        darmok::Sphere convert(const protobuf::Sphere& v)
        {
			return { convert(v.origin()), v.radius() };
        }

        darmok::Capsule convert(const protobuf::Capsule& v)
        {
			return { v.cylinder_height(), v.radius(), convert(v.origin()) };
        }

        darmok::Rectangle convert(const protobuf::Rectangle& v)
        {
			return { convert(v.size()), convert(v.origin()) };
        }

        darmok::Plane convert(const protobuf::Plane& v)
        {
			return { convert(v.normal()), v.distance() };
        }

        darmok::Triangle convert(const protobuf::Triangle& v)
        {
			return { convert(v.vertex1()), convert(v.vertex2()), convert(v.vertex3()) };
        }

        darmok::Polygon convert(const protobuf::Polygon& v)
        {
            std::vector<darmok::Triangle> tris;
            tris.reserve(v.triangles_size());
            for (const auto& tri : v.triangles())
            {
                tris.push_back(convert(tri));
            }
            return { std::move(tris), convert(v.origin()) };
        }

        protobuf::BoundingBox convert(const darmok::BoundingBox& v)
        {
            protobuf::BoundingBox msg;
			*msg.mutable_min() = std::move(convert(v.min));
            *msg.mutable_max() = std::move(convert(v.max));
            return msg;
        }

        protobuf::Cube convert(const darmok::Cube& v)
        {
            protobuf::Cube msg;
            *msg.mutable_size() = std::move(convert(v.size));
            *msg.mutable_origin() = std::move(convert(v.origin));
            return msg;
        }

        protobuf::Sphere convert(const darmok::Sphere& v)
        {
            protobuf::Sphere msg;
            *msg.mutable_origin() = std::move(convert(v.origin));
			msg.set_radius(v.radius);
            return msg;
        }

        protobuf::Capsule convert(const darmok::Capsule& v)
        {
            protobuf::Capsule msg;
			msg.set_cylinder_height(v.cylinderHeight);
			msg.set_radius(v.radius);
			*msg.mutable_origin() = std::move(convert(v.origin));
            return msg;
        }

        protobuf::Rectangle convert(const darmok::Rectangle& v)
        {
            protobuf::Rectangle msg;
			*msg.mutable_size() = std::move(convert(v.size));
			*msg.mutable_origin() = std::move(convert(v.origin));
            return msg;
        }

        protobuf::Plane convert(const darmok::Plane& v)
        {
            protobuf::Plane msg;
			*msg.mutable_normal() = std::move(convert(v.normal));
			msg.set_distance(v.distance);
            return msg;
        }
        protobuf::Triangle convert(const darmok::Triangle& v)
        {
			protobuf::Triangle msg;
			*msg.mutable_vertex1() = convert(v.vertices[0]);
            *msg.mutable_vertex2() = convert(v.vertices[1]);
            *msg.mutable_vertex3() = convert(v.vertices[2]);
            return msg;
        }

        protobuf::Polygon convert(const darmok::Polygon& v)
        {
            protobuf::Polygon msg;
            for (const auto& tri : v.triangles)
            {
                *msg.add_triangles() = convert(tri);
            }
            *msg.mutable_origin() = convert(v.origin);
            return msg;
		}
    }
}