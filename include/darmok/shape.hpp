#pragma once

#include <glm/glm.hpp>
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace darmok
{
    struct Line;

    struct Quad
    {
        static const Quad& standard() noexcept;

        glm::vec2 size;
        glm::vec2 origin;

        Quad(const glm::vec2& size = glm::vec2(1), const glm::vec2& origin = glm::vec2(0)) noexcept;
        std::string to_string() const noexcept;
        std::vector<Line> toLines() const noexcept;
    };

    struct Cube
    {
        static const Cube& standard() noexcept;

        glm::vec3 size;
        glm::vec3 origin;

        Cube(const glm::vec3& size = glm::vec3(1), const glm::vec3& origin = glm::vec3(0)) noexcept;
        std::string to_string() const noexcept;
    };


    struct Triangle
    {
        using Vertices = std::array<glm::vec3, 3>;
        Vertices vertices;

        Triangle(const glm::vec3& vert1, const glm::vec3& vert2, const glm::vec3& vert3) noexcept;
        Triangle(const Vertices& vertices) noexcept;
        std::string to_string() const noexcept;
    };

    struct Sphere
    {
        static const Sphere& standard() noexcept;

        float radius;
        glm::vec3 origin;

        Sphere(float radius = 0.5f, const glm::vec3& origin = glm::vec3(0)) noexcept;
        std::string to_string() const noexcept;
    };

    struct Plane
    {
        static const Plane& standard() noexcept;

        glm::vec3 normal;
        glm::vec3 origin;

        Plane(const glm::vec3& normal = glm::vec3(1), const glm::vec3& origin = glm::vec3(0)) noexcept;
        std::string to_string() const noexcept;
    };

    struct NormalIntersection
    {
        glm::vec3 position;
        glm::vec3 normal;

        std::string to_string() const noexcept;
    };

    struct DistanceIntersection
    {
        glm::vec2 position;
        float distance;

        std::string to_string() const noexcept;
    };

    struct Line;

    struct Ray
    {
        glm::vec3 direction;
        glm::vec3 origin;

        Ray(const glm::vec3& origin = glm::vec3(0), const glm::vec3& dir = glm::vec3(0, 1, 0)) noexcept;
        
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

    struct Line
    {
        using Points = std::array<glm::vec3, 2>;
        Points points;

        Line(const glm::vec3& point1 = glm::vec3(0), const glm::vec3& point2 = glm::vec3(0, 1, 0)) noexcept;
        Line(const Points& points = { glm::vec3(0), glm::vec3(0, 1, 0) }) noexcept;

        std::string to_string() const noexcept;

        glm::vec3 operator*(float dist) const noexcept;

        std::optional<std::array<NormalIntersection, 2>> intersect(const Sphere& sphere) const noexcept;
        std::optional<glm::vec3> intersect(const Triangle& tri) const noexcept;
    };

}