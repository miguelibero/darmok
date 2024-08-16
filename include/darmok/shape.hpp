#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace darmok
{
    struct Line;

    struct DARMOK_EXPORT Rectangle final
    {
        glm::vec2 size;
        glm::vec2 origin;

        Rectangle(const glm::vec2& size = glm::vec2(1), const glm::vec2& origin = glm::vec2(0)) noexcept;
        std::string toString() const noexcept;
        std::vector<Line> toLines() const noexcept;

        static const Rectangle& standard() noexcept;

        Rectangle& operator*=(float scale) noexcept;
        Rectangle operator*(float scale) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(size, origin);
        }
    };

    struct DARMOK_EXPORT Cube final
    {
        glm::vec3 size;
        glm::vec3 origin;

        Cube(const glm::vec3& size = glm::vec3(1), const glm::vec3& origin = glm::vec3(0)) noexcept;
        std::string toString() const noexcept;

        Cube& operator*=(float scale) noexcept;
        Cube operator*(float scale) const noexcept;

        static const Cube& standard() noexcept;

        bool empty() const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(size, origin);
        }
    };

    struct TextureTriangle;

    struct DARMOK_EXPORT Triangle final
    {
        using Vertices = std::array<glm::vec3, 3>;
        Vertices vertices;

        Triangle(const glm::vec3& vert1, const glm::vec3& vert2, const glm::vec3& vert3) noexcept;
        Triangle(const Vertices& vertices = {}) noexcept;
        std::string toString() const noexcept;

        glm::vec3 getNormal() const;
        glm::vec3 getTangent(const TextureTriangle& texTri) const noexcept;
        glm::vec3 getTangent(const TextureTriangle& texTri, const glm::vec3& normal) const noexcept;

        Triangle& operator*=(float scale) noexcept;
        Triangle operator*(float scale) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(vertices);
        }
    };

    struct DARMOK_EXPORT TextureTriangle final
    {
        using Coordinates = std::array<glm::vec2, 3>;
        Coordinates coordinates;

        TextureTriangle(const glm::vec2& coord1, const glm::vec2& coord2, const glm::vec2& coord3) noexcept;
        TextureTriangle(const Coordinates& coordinates = {}) noexcept;
        std::string toString() const noexcept;

        TextureTriangle& operator*=(float scale) noexcept;
        TextureTriangle operator*(float scale) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(coordinates);
        }
    };

    struct DARMOK_EXPORT Polygon final
    {
        using Triangles = std::vector<Triangle>;

        Triangles triangles;
        glm::vec3 origin;

        Polygon(const Triangles& tris = {}, const glm::vec3& origin = glm::vec3(0)) noexcept;
        std::string toString() const noexcept;

        Polygon& operator*=(float scale) noexcept;
        Polygon operator*(float scale) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(triangles, origin);
        }
    };

    struct DARMOK_EXPORT Sphere final
    {
        float radius;
        glm::vec3 origin;

        Sphere(const glm::vec3& origin, float radius = 0.5f) noexcept;
        Sphere(float radius = 0.5f, const glm::vec3& origin = glm::vec3(0)) noexcept;
        std::string toString() const noexcept;

        static const Sphere& standard() noexcept;

        Sphere& operator*=(float scale) noexcept;
        Sphere operator*(float scale) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(radius, origin);
        }
    };

    struct DARMOK_EXPORT Plane final
    {
        glm::vec3 normal;
        float constant;

        Plane(const glm::vec3& normal = glm::vec3(0, 1, 0), float constant = 0.F) noexcept;
        std::string toString() const noexcept;

        glm::vec3 getOrigin() const noexcept;

        Plane operator*(const glm::mat4& transform) const noexcept;
        Plane& operator*=(const glm::mat4& transform) noexcept;

        static const Plane& standard() noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(normal, constant);
        }
    };

    struct DARMOK_EXPORT Capsule final
    {
        float cylinderHeight;
        float radius;
        glm::vec3 origin;

        Capsule(float cylinderHeight = 1.F, float radius = 0.5F, const glm::vec3& origin = glm::vec3(0)) noexcept;
        std::string toString() const noexcept;
        static const Capsule& standard() noexcept;

        Capsule& operator*=(float scale) noexcept;
        Capsule operator*(float scale) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(cylinderHeight, radius, origin);
        }
    };

    struct DARMOK_EXPORT NormalIntersection final
    {
        glm::vec3 position = glm::vec3(0);
        glm::vec3 normal = glm::vec3(0, 1, 0);

        std::string toString() const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(position, normal);
        }
    };

    struct DARMOK_EXPORT DistanceIntersection final
    {
        glm::vec2 position = glm::vec3(0);
        float distance = 0.F;

        std::string toString() const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(position, distance);
        }
    };

    struct Line;

    struct DARMOK_EXPORT Ray final
    {
        glm::vec3 direction;
        glm::vec3 origin;

        Ray(const glm::vec3& origin = glm::vec3(0), const glm::vec3& dir = glm::vec3(0, 1, 0)) noexcept;

        Ray operator*(const glm::mat4& transform) const noexcept;
        Ray& operator*=(const glm::mat4& transform) noexcept;

        glm::vec3 operator*(float dist) const noexcept;

        std::string toString() const noexcept;
        Line toLine() const noexcept;

        // returns distance to ray origin
        std::optional<float> intersect(const Plane& plane) const noexcept;

        // returns distance to ray origin
        std::optional<float> intersect(const Sphere& sphere) const noexcept;

        std::optional<NormalIntersection> intersectNormal(const Sphere& sphere) const noexcept;
        std::optional<DistanceIntersection> intersect(const Triangle& tri) const noexcept;

        static Ray unproject(const glm::vec2& screenPosition, const glm::mat4& model, const glm::mat4& proj, const glm::ivec4& viewport) noexcept;
    
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(direction, origin);
        }
    };

    struct DARMOK_EXPORT Line final
    {
        using Points = std::array<glm::vec3, 2>;
        Points points;

        Line(const glm::vec3& point1 = glm::vec3(0), const glm::vec3& point2 = glm::vec3(0, 1, 0)) noexcept;
        Line(const Points& points) noexcept;

        std::string toString() const noexcept;

        glm::vec3 operator*(float dist) const noexcept;

        std::optional<std::array<NormalIntersection, 2>> intersect(const Sphere& sphere) const noexcept;
        std::optional<glm::vec3> intersect(const Triangle& tri) const noexcept;
        glm::vec3 closestPoint(const glm::vec3& p);

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(points);
        }
    };

    struct DARMOK_EXPORT BoundingBox final
    {
        glm::vec3 min;
        glm::vec3 max;

        BoundingBox() noexcept;
        BoundingBox(const glm::vec3& min, const glm::vec3& max) noexcept;

        BoundingBox& operator+=(const BoundingBox& bb) noexcept;
        BoundingBox operator+(const BoundingBox& bb) noexcept;

        BoundingBox expand(const glm::vec3& size) const noexcept;
        BoundingBox contract(const glm::vec3& size) const noexcept;

        Cube getCube() const noexcept;
        operator Cube() const noexcept;

        bool empty() const noexcept;

        std::string toString() const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(min, max);
        }
    };

}

namespace std
{
    inline std::string to_string(const darmok::Rectangle & v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::Cube& v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::Triangle& v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::Polygon& v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::Sphere& v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::Plane& v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::Capsule& v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::NormalIntersection& v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::DistanceIntersection& v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::Ray& v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::Line& v)
    {
        return v.toString();
    }

    inline std::string to_string(const darmok::BoundingBox& v)
    {
        return v.toString();
    }
}