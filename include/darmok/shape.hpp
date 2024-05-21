#pragma once

#include <glm/glm.hpp>
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace darmok
{
    struct Line;

    struct Rectangle final
    {
        glm::vec2 size;
        glm::vec2 origin;

        DLLEXPORT Rectangle(const glm::vec2& size = glm::vec2(1), const glm::vec2& origin = glm::vec2(0)) noexcept;
        DLLEXPORT std::string to_string() const noexcept;
        DLLEXPORT std::vector<Line> toLines() const noexcept;

        DLLEXPORT static const Rectangle& standard() noexcept;
    };

    struct Cube final
    {
        glm::vec3 size;
        glm::vec3 origin;

        DLLEXPORT Cube(const glm::vec3& size = glm::vec3(1), const glm::vec3& origin = glm::vec3(0)) noexcept;
        DLLEXPORT std::string to_string() const noexcept;

        DLLEXPORT static const Cube& standard() noexcept;
    };


    struct Triangle final
    {
        using Vertices = std::array<glm::vec3, 3>;
        Vertices vertices;

        DLLEXPORT Triangle(const glm::vec3& vert1, const glm::vec3& vert2, const glm::vec3& vert3) noexcept;
        DLLEXPORT Triangle(const Vertices& vertices) noexcept;
        DLLEXPORT std::string to_string() const noexcept;
    };

    struct Sphere final
    {
        float radius;
        glm::vec3 origin;

        DLLEXPORT Sphere(float radius = 0.5f, const glm::vec3& origin = glm::vec3(0)) noexcept;
        DLLEXPORT std::string to_string() const noexcept;

        DLLEXPORT static const Sphere& standard() noexcept;
    };

    struct Plane final
    {
        glm::vec3 normal;
        glm::vec3 origin;

        DLLEXPORT Plane(const glm::vec3& normal = glm::vec3(1), const glm::vec3& origin = glm::vec3(0)) noexcept;
        DLLEXPORT std::string to_string() const noexcept;

        DLLEXPORT Plane operator*(const glm::mat4& transform) const noexcept;
        DLLEXPORT Plane& operator*=(const glm::mat4& transform) noexcept;

        DLLEXPORT static const Plane& standard() noexcept;
    };

    struct NormalIntersection final
    {
        glm::vec3 position;
        glm::vec3 normal;

        DLLEXPORT std::string to_string() const noexcept;
    };

    struct DistanceIntersection final
    {
        glm::vec2 position;
        float distance;

        DLLEXPORT std::string to_string() const noexcept;
    };

    struct Line;

    struct Ray final
    {
        glm::vec3 direction;
        glm::vec3 origin;

        DLLEXPORT Ray(const glm::vec3& origin = glm::vec3(0), const glm::vec3& dir = glm::vec3(0, 1, 0)) noexcept;
        
        DLLEXPORT Ray operator*(const glm::mat4& transform) const noexcept;
        DLLEXPORT Ray& operator*=(const glm::mat4& transform) noexcept;

        DLLEXPORT glm::vec3 operator*(float dist) const noexcept;
        
        DLLEXPORT std::string to_string() const noexcept;
        DLLEXPORT Line toLine() const noexcept;

        // returns distance to ray origin
        DLLEXPORT std::optional<float> intersect(const Plane& plane) const noexcept;
        
        // returns distance to ray origin
        DLLEXPORT std::optional<float> intersect(const Sphere& sphere) const noexcept;

        DLLEXPORT std::optional<NormalIntersection> intersectNormal(const Sphere& sphere) const noexcept;
        DLLEXPORT std::optional<DistanceIntersection> intersect(const Triangle& tri) const noexcept;

        DLLEXPORT static Ray unproject(const glm::vec2& screenPosition, const glm::mat4& model, const glm::mat4& proj, const glm::ivec4& viewport) noexcept;
    };

    struct Line final
    {
        using Points = std::array<glm::vec3, 2>;
        Points points;

        DLLEXPORT Line(const glm::vec3& point1 = glm::vec3(0), const glm::vec3& point2 = glm::vec3(0, 1, 0)) noexcept;
        DLLEXPORT Line(const Points& points) noexcept;

        DLLEXPORT std::string to_string() const noexcept;

        DLLEXPORT glm::vec3 operator*(float dist) const noexcept;

        DLLEXPORT std::optional<std::array<NormalIntersection, 2>> intersect(const Sphere& sphere) const noexcept;
        DLLEXPORT std::optional<glm::vec3> intersect(const Triangle& tri) const noexcept;
    };

}