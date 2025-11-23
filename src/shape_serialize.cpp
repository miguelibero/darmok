#include <darmok/shape_serialize.hpp>
#include <darmok/glm_serialize.hpp>

namespace darmok
{
    /*
    protobuf::BoundingBox Converter<protobuf::BoundingBox, BoundingBox>::run(const BoundingBox& v)
    {
        protobuf::BoundingBox msg;
        *msg.mutable_min() = convert<protobuf::Vec3>(v.min);
        *msg.mutable_max() = convert<protobuf::Vec3>(v.max);
        return msg;
    }

    protobuf::Cube Converter<protobuf::Cube, Cube>::run(const Cube& v)
    {
        protobuf::Cube msg;
        *msg.mutable_size() = convert<protobuf::Vec3>(v.size);
        *msg.mutable_origin() = convert<protobuf::Vec3>(v.origin);
        return msg;
    }

    protobuf::Sphere Converter<protobuf::Sphere, Sphere>::run(const Sphere& v)
    {
        protobuf::Sphere msg;
        *msg.mutable_origin() = convert<protobuf::Vec3>(v.origin);
        msg.set_radius(v.radius);
        return msg;
    }

    protobuf::Capsule Converter<protobuf::Capsule, Capsule>::run(const Capsule& v)
    {
        protobuf::Capsule msg;
        msg.set_cylinder_height(v.cylinderHeight);
        msg.set_radius(v.radius);
        *msg.mutable_origin() = convert<protobuf::Vec3>(v.origin);
        return msg;
    }

    protobuf::Rectangle Converter<protobuf::Rectangle, Rectangle>::run(const Rectangle& v)
    {
        protobuf::Rectangle msg;
        *msg.mutable_size() = convert<protobuf::Vec2>(v.size);
        *msg.mutable_origin() = convert<protobuf::Vec2>(v.origin);
        return msg;
    }

    protobuf::Plane Converter<protobuf::Plane, Plane>::run(const Plane& v)
    {
        protobuf::Plane msg;
        *msg.mutable_normal() = convert<protobuf::Vec3>(v.normal);
        msg.set_distance(v.distance);
        return msg;
    }

    protobuf::Triangle Converter<protobuf::Triangle, Triangle>::run(const Triangle& v)
    {
        protobuf::Triangle msg;
        *msg.mutable_vertex1() = convert<protobuf::Vec3>(v.vertices[0]);
        *msg.mutable_vertex2() = convert<protobuf::Vec3>(v.vertices[1]);
        *msg.mutable_vertex3() = convert<protobuf::Vec3>(v.vertices[2]);
        return msg;
    }

    protobuf::Polygon Converter<protobuf::Polygon, Polygon>::run(const Polygon& v)
    {
        protobuf::Polygon msg;
        for (const auto& tri : v.triangles)
        {
            *msg.add_triangles() = convert<protobuf::Triangle>(tri);
        }
        *msg.mutable_origin() = convert<protobuf::Vec3>(v.origin);
        return msg;
    }

    BoundingBox Converter<BoundingBox, protobuf::BoundingBox>::run(const protobuf::BoundingBox& v)
    {
        return { convert<glm::vec3>(v.min()), convert<glm::vec3>(v.max()) };
    }

    Cube Converter<Cube, protobuf::Cube>::run(const protobuf::Cube& v)
    {
        return { convert<glm::vec3>(v.size()), convert<glm::vec3>(v.origin()) };
	}

    Sphere Converter<Sphere, protobuf::Sphere>::run(const protobuf::Sphere& v)
    {
        return { convert<glm::vec3>(v.origin()), v.radius() };
	}

    Capsule Converter<Capsule, protobuf::Capsule>::run(const protobuf::Capsule& v)
    {
        return { v.cylinder_height(), v.radius(), convert<glm::vec3>(v.origin()) };
    }

    Rectangle Converter<Rectangle, protobuf::Rectangle>::run(const protobuf::Rectangle& v)
    {
        return { convert<glm::vec2>(v.size()), convert<glm::vec2>(v.origin()) };
	}

    Plane Converter<Plane, protobuf::Plane>::run(const protobuf::Plane& v)
    {
        return { convert<glm::vec3>(v.normal()), v.distance() };
    }

    Triangle Converter<Triangle, protobuf::Triangle>::run(const protobuf::Triangle& v)
    {
        return { convert<glm::vec3>(v.vertex1()), convert<glm::vec3>(v.vertex2()), convert<glm::vec3>(v.vertex3()) };
	}

    Polygon Converter<Polygon, protobuf::Polygon>::run(const protobuf::Polygon& v)
    {
        std::vector<Triangle> tris;
        tris.reserve(v.triangles_size());
        for (const auto& tri : v.triangles())
        {
            tris.push_back(convert<Triangle>(tri));
        }
        return { std::move(tris), convert<glm::vec3>(v.origin()) };
	}
    */
}