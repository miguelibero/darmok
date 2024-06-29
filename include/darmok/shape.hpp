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
        std::string to_string() const noexcept;
        std::vector<Line> toLines() const noexcept;

        static const Rectangle& standard() noexcept;
    };

    struct DARMOK_EXPORT Cuboid final
    {
        glm::vec3 size;
        glm::vec3 origin;

        Cuboid(const glm::vec3& size = glm::vec3(1), const glm::vec3& origin = glm::vec3(0)) noexcept;
        std::string to_string() const noexcept;

        static const Cuboid& standard() noexcept;
    };

    struct DARMOK_EXPORT Triangle final
    {
        using Vertices = std::array<glm::vec3, 3>;
        Vertices vertices;

        Triangle(const glm::vec3& vert1, const glm::vec3& vert2, const glm::vec3& vert3) noexcept;
        Triangle(const Vertices& vertices) noexcept;
        std::string to_string() const noexcept;
        glm::vec3 getNormal() const;
    };

    struct DARMOK_EXPORT Sphere final
    {
        float radius;
        glm::vec3 origin;

        Sphere(const glm::vec3& origin, float radius = 0.5f) noexcept;
        Sphere(float radius = 0.5f, const glm::vec3& origin = glm::vec3(0)) noexcept;
        std::string to_string() const noexcept;

        static const Sphere& standard() noexcept;
    };

    struct DARMOK_EXPORT Plane final
    {
        glm::vec3 normal;
        float constant;

        Plane(const glm::vec3& normal = glm::vec3(0, 1, 0), float constant = 0.F) noexcept;
        std::string to_string() const noexcept;

        glm::vec3 getOrigin() const noexcept;

        Plane operator*(const glm::mat4& transform) const noexcept;
        Plane& operator*=(const glm::mat4& transform) noexcept;

        static const Plane& standard() noexcept;
    };

    struct DARMOK_EXPORT Capsule final
    {
        float cylinderHeight;
        float radius;
        glm::vec3 origin;

        Capsule(float cylinderHeight = 1.F, float radius = 0.5F, const glm::vec3& origin = glm::vec3(0)) noexcept;
        std::string to_string() const noexcept;
        static const Capsule& standard() noexcept;
    };

    struct DARMOK_EXPORT NormalIntersection final
    {
        glm::vec3 position;
        glm::vec3 normal;

        std::string to_string() const noexcept;
    };

    struct DARMOK_EXPORT DistanceIntersection final
    {
        glm::vec2 position;
        float distance;

        std::string to_string() const noexcept;
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
        
        std::string to_string() const noexcept;
        Line toLine() const noexcept;

        // returns distance to ray origin
        std::optional<float> intersect(const Plane& plane) const noexcept;
        
        // returns distance to ray origin
        std::optional<float> intersect(const Sphere& sphere) const noexcept;

        std::optional<NormalIntersection> intersectNormal(const Sphere& sphere) const noexcept;
        std::optional<DistanceIntersection> intersect(const Triangle& tri) const noexcept;

        static Ray unproject(const glm::vec2& screenPosition, const glm::mat4& model, const glm::mat4& proj, const glm::ivec4& viewport) noexcept;

    };

    struct DARMOK_EXPORT Line final
    {
        using Points = std::array<glm::vec3, 2>;
        Points points;

        Line(const glm::vec3& point1 = glm::vec3(0), const glm::vec3& point2 = glm::vec3(0, 1, 0)) noexcept;
        Line(const Points& points) noexcept;

        std::string to_string() const noexcept;

        glm::vec3 operator*(float dist) const noexcept;

        std::optional<std::array<NormalIntersection, 2>> intersect(const Sphere& sphere) const noexcept;
        std::optional<glm::vec3> intersect(const Triangle& tri) const noexcept;
        glm::vec3 closestPoint(const glm::vec3& p);
    };
}