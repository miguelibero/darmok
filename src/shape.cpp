#include <darmok/shape.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/string_cast.hpp>

namespace darmok
{
    const Rectangle& Rectangle::standard() noexcept
    {
        const static Rectangle v;
        return v;
    }

    const Cube& Cube::standard() noexcept
    {
        const static Cube v;
        return v;
    }

    const Sphere& Sphere::standard() noexcept
    {
        const static Sphere v;
        return v;
    }

    const Plane& Plane::standard() noexcept
    {
        const static Plane v;
        return v;
    }

    Rectangle::Rectangle(const glm::vec2& size, const glm::vec2& origin) noexcept
        : size(size)
        , origin(origin)
    {
    }

    std::string Rectangle::to_string() const noexcept
    {
        return "Rectangle(size=" + glm::to_string(size) + ", origin=" + glm::to_string(origin) + ")";
    }

    std::vector<Line> Rectangle::toLines() const noexcept
    {
        glm::vec3 v0(origin, 0);
        glm::vec3 v1 = v0 + glm::vec3(size.x, 0, 0);
        glm::vec3 v2 = v0 + glm::vec3(size, 0);
        glm::vec3 v3 = v0 + glm::vec3(0, size.y, 0);
        return { { v0, v1 }, {v1, v2}, { v2, v3 }, { v3, v0 } };
    }

    Cube::Cube(const glm::vec3& size, const glm::vec3& origin) noexcept
        : size(size)
        , origin(origin)
    {
    }

    std::string Cube::to_string() const noexcept
    {
        return "Cube(size=" + glm::to_string(size) + ", origin=" + glm::to_string(origin) + ")";
    }

    Triangle::Triangle(const glm::vec3& vert1, const glm::vec3& vert2, const glm::vec3& vert3) noexcept
        : vertices{ vert1, vert2, vert3 }
    {
    }

    Triangle::Triangle(const Vertices& vertices) noexcept
        : vertices(vertices)
    {
    }

    std::string Triangle::to_string() const noexcept
    {
        return "Triangle(" + glm::to_string(vertices[0]) + ", " + glm::to_string(vertices[1]) + ", " + glm::to_string(vertices[2]) + ")";
    }

    Sphere::Sphere(const glm::vec3& origin, float radius) noexcept
        : radius(radius)
        , origin(origin)
    {
    }

    Sphere::Sphere(float radius, const glm::vec3& origin) noexcept
        : radius(radius)
        , origin(origin)
    {
    }

    std::string Sphere::to_string() const noexcept
    {
        return "Cube(radius=" + std::to_string(radius) + ", origin=" + glm::to_string(origin) + ")";
    }

    Plane::Plane(const glm::vec3& normal, const glm::vec3& origin) noexcept
        : normal(normal)
        , origin(origin)
    {
    }

    std::string Plane::to_string() const noexcept
    {
        return "Plane(normal=" + glm::to_string(normal) + ", origin=" + glm::to_string(origin) + ")";
    }

    glm::vec3 transformNormal(const glm::vec3& normal, const glm::mat4& transform) noexcept
    {
        return glm::normalize(transform * glm::vec4(normal, 0));
    }

    glm::vec3 transformPosition(const glm::vec3& position, const glm::mat4& transform) noexcept
    {
        return transform * glm::vec4(position, 1);
    }

    Plane Plane::operator*(const glm::mat4& transform) const noexcept
    {
        return Plane(transformNormal(normal, transform), transformPosition(origin, transform));
    }

    Plane& Plane::operator*=(const glm::mat4& transform) noexcept
    {
        normal = transformNormal(normal, transform);
        origin = transformPosition(origin, transform);
        return *this;
    }

    Ray::Ray(const glm::vec3& dir, const glm::vec3& origin) noexcept
        : direction(dir)
        , origin(origin)
    {
    }

    Ray Ray::operator*(const glm::mat4& transform) const noexcept
    {
        return Ray(transformNormal(direction, transform), transformPosition(origin, transform));
    }

    Ray& Ray::operator*=(const glm::mat4& transform) noexcept
    {
        direction = transformNormal(direction, transform);
        origin = transformPosition(origin, transform);
        return *this;
    }

    std::string Ray::to_string() const noexcept
    {
        return "Ray(direction=" + glm::to_string(direction) + ", origin=" + glm::to_string(origin) + ")";
    }

    Line Ray::toLine() const noexcept
    {
        return Line({ origin, origin + direction });
    }

    std::optional<float> Ray::intersect(const Plane& plane) const noexcept
    {
        float dist;
        if (glm::intersectRayPlane(origin, direction, plane.origin, plane.normal, dist))
        {
            return dist;
        }
        return std::nullopt;
    }

    std::optional<float> Ray::intersect(const Sphere& sphere) const noexcept
    {
        float dist;
        if (glm::intersectRaySphere(origin, direction, sphere.origin, sphere.radius, dist))
        {
            return dist;
        }
        return std::nullopt;
    }

    std::optional<NormalIntersection> Ray::intersectNormal(const Sphere& sphere) const noexcept
    {
        NormalIntersection intersect;
        if (glm::intersectRaySphere(origin, direction, sphere.origin, sphere.radius, intersect.position, intersect.normal))
        {
            return intersect;
        }
        return std::nullopt;
    }

    std::optional<DistanceIntersection> Ray::intersect(const Triangle& tri) const noexcept
    {
        DistanceIntersection intersect;
        if (glm::intersectRayTriangle(origin, direction, tri.vertices[0], tri.vertices[1], tri.vertices[2], intersect.position, intersect.distance))
        {
            return intersect;
        }
        return std::nullopt;
    }

    Ray Ray::unproject(const glm::vec2& screenPosition, const glm::mat4& model, const glm::mat4& proj, const glm::ivec4& viewport) noexcept
    {
        auto near = glm::unProject(glm::vec3(screenPosition, 0), model, proj, viewport);
        auto far = glm::unProject(glm::vec3(screenPosition, 1), model, proj, viewport);

        return Ray(glm::normalize(far - near), near);
    }

    glm::vec3 Ray::operator*(float dist) const noexcept
    {
        return origin + (direction * dist);
    }

    Line::Line(const glm::vec3& point1, const glm::vec3& point2) noexcept
    : points{ point1, point2 }
    {
    }

    Line::Line(const Points& points) noexcept
        : points(points)
    {
    }

    std::string Line::to_string() const noexcept
    {
        return "Line(" + glm::to_string(points[0]) + ", " + glm::to_string(points[1]) + ")";
    }

    glm::vec3 Line::operator*(float dist) const noexcept
    {
        return points[0] + ( points[1] * dist );
    }

    std::optional<std::array<NormalIntersection, 2>> Line::intersect(const Sphere& sphere) const noexcept
    {
        std::array<NormalIntersection, 2> intersect;
        if (glm::intersectLineSphere(points[0], points[1], sphere.origin, sphere.radius,
            intersect[0].position, intersect[0].normal, intersect[1].position, intersect[1].normal))
        {
            return intersect;
        }
        return std::nullopt;
    }

    std::optional<glm::vec3> Line::intersect(const Triangle& tri) const noexcept
    {
        glm::vec3 pos;
        if (glm::intersectLineTriangle(points[0], points[1], tri.vertices[0], tri.vertices[1], tri.vertices[2], pos))
        {
            return pos;
        }
        return std::nullopt;
    }

    glm::vec3 Line::closestPoint(const glm::vec3& p)
    {
        return glm::closestPointOnLine(p, points[0], points[1]);
    }

    std::string NormalIntersection::to_string() const noexcept
    {
        return "NormalIntersection(position=" + glm::to_string(position) + ", normal=" + glm::to_string(normal) + ")";
    }

    std::string DistanceIntersection::to_string() const noexcept
    {
        return "DistanceIntersection(position=" + glm::to_string(position) + ", distance=" + std::to_string(distance) + ")";
    }
}