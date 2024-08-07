#include <darmok/shape.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtx/string_cast.hpp>
#include <darmok/string.hpp>
#include <glm/gtx/norm.hpp>

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

    bool Cube::empty() const noexcept
    {
        return glm::length2(size) == 0.F;
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

    std::string Rectangle::toString() const noexcept
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

    std::string Cube::toString() const noexcept
    {
        return "Cube(size=" + glm::to_string(size) + ", origin=" + glm::to_string(origin) + ")";
    }

    Cube& Cube::operator*=(float scale) noexcept
    {
        size *= scale;
        origin *= scale;
        return *this;
    }

    Cube Cube::operator*(float scale) const noexcept
    {
        Cube cube(*this);
        cube *= scale;
        return cube;
    }

    Triangle::Triangle(const glm::vec3& vert1, const glm::vec3& vert2, const glm::vec3& vert3) noexcept
        : vertices{ vert1, vert2, vert3 }
    {
    }

    Triangle::Triangle(const Vertices& vertices) noexcept
        : vertices(vertices)
    {
    }

    std::string Triangle::toString() const noexcept
    {
        return "Triangle(" + glm::to_string(vertices[0]) + ", " + glm::to_string(vertices[1]) + ", " + glm::to_string(vertices[2]) + ")";
    }

    glm::vec3 Triangle::getNormal() const
    {
        auto edge1 = vertices[1] - vertices[0];
        auto edge2 = vertices[2] - vertices[0];
        auto normal = glm::cross(edge1, edge2);
        return glm::normalize(normal);
    }

    Triangle& Triangle::operator*=(float scale) noexcept
    {
        for (auto& vert : vertices)
        {
            vert *= scale;
        }
        return *this;
    }

    Triangle Triangle::operator*(float scale) const noexcept
    {
        Triangle tri = *this;
        tri *= scale;
        return tri;
    }

    Polygon::Polygon(const Triangles& tris, const glm::vec3& origin) noexcept
        : triangles(tris)
        , origin(origin)
    {
    }

    std::string Polygon::toString() const noexcept
    {
        std::string str = "Polygon(" + StringUtils::join(", ", triangles);
        if (origin != glm::vec3(0))
        {
            str += ", origin = " + glm::to_string(origin);
        }
        return str +")";
    }

    Polygon& Polygon::operator*=(float scale) noexcept
    {
        for (auto& tri : triangles)
        {
            tri *= scale;
        }
        return *this;
    }

    Polygon Polygon::operator*(float scale) const noexcept
    {
        Polygon poly = *this;
        poly *= scale;
        return poly;
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

    std::string Sphere::toString() const noexcept
    {
        return "Cube(radius=" + std::to_string(radius) + ", origin=" + glm::to_string(origin) + ")";
    }

    Sphere& Sphere::operator*=(float scale) noexcept
    {
        origin *= scale;
        radius *= scale;
        return *this;
    }

    Sphere Sphere::operator*(float scale) const noexcept
    {
        Sphere copy(*this);
        copy *= scale;
        return copy;
    }

    Plane::Plane(const glm::vec3& normal, float constant) noexcept
        : normal(normal)
        , constant(constant)
    {
    }

    glm::vec3 Plane::getOrigin() const noexcept
    {
        return normal * constant;
    }

    std::string Plane::toString() const noexcept
    {
        return "Plane(normal=" + glm::to_string(normal) + ", constant=" + std::to_string(constant) + ")";
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
        return Plane(transformNormal(normal, transform), constant);
    }

    Plane& Plane::operator*=(const glm::mat4& transform) noexcept
    {
        normal = transformNormal(normal, transform);
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

    std::string Ray::toString() const noexcept
    {
        return "Ray(direction=" + glm::to_string(direction) + ", origin=" + glm::to_string(origin) + ")";
    }

    Line Ray::toLine() const noexcept
    {
        return Line(origin, origin + direction);
    }

    std::optional<float> Ray::intersect(const Plane& plane) const noexcept
    {
        float dist;
        if (glm::intersectRayPlane(origin, direction, plane.getOrigin(), plane.normal, dist))
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

    std::string Line::toString() const noexcept
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

    std::string NormalIntersection::toString() const noexcept
    {
        return "NormalIntersection(position=" + glm::to_string(position) + ", normal=" + glm::to_string(normal) + ")";
    }

    std::string DistanceIntersection::toString() const noexcept
    {
        return "DistanceIntersection(position=" + glm::to_string(position) + ", distance=" + std::to_string(distance) + ")";
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

    Capsule::Capsule(float cylinderHeight, float radius, const glm::vec3& origin) noexcept
        : cylinderHeight(cylinderHeight)
        , radius(radius)
        , origin(origin)
    {
    }

    std::string Capsule::toString() const noexcept
    {
        return "Capsule(cylinderHeight=" + std::to_string(cylinderHeight) + ", radius=" + std::to_string(radius) + ", origin=" + glm::to_string(origin) + ")";
    }

    const Capsule& Capsule::standard() noexcept
    {
        const static Capsule v;
        return v;
    }

    Capsule& Capsule::operator*=(float scale) noexcept
    {
        cylinderHeight *= scale;
        radius *= scale;
        origin *= scale;
        return *this;
    }

    Capsule Capsule::operator*(float scale) const noexcept
    {
        Capsule copy(*this);
        copy *= scale;
        return copy;
    }

    BoundingBox::BoundingBox() noexcept
        : min(0), max(0)
    {
    }

    BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max) noexcept
        : min(min), max(max)
    {
    }

    BoundingBox& BoundingBox::operator+=(const BoundingBox& bb) noexcept
    {
        min = glm::min(min, bb.min);
        max = glm::max(max, bb.max);
        return *this;
    }

    BoundingBox BoundingBox::operator+(const BoundingBox& bb) noexcept
    {
        BoundingBox sum = *this;
        sum += bb;
        return sum;
    }

    BoundingBox BoundingBox::expand(const glm::vec3& size) const noexcept
    {
        return BoundingBox(min - size, max + size);
    }

    BoundingBox BoundingBox::contract(const glm::vec3& size) const noexcept
    {
        return BoundingBox(min + size, max - size);
    }

    Cube BoundingBox::getCube() const noexcept
    {
        auto size = max - min;
        auto origin = min + (size * 0.5F);
        return Cube(size, origin);
    }

    BoundingBox::operator Cube() const noexcept
    {
        return getCube();
    }

    bool BoundingBox::empty() const noexcept
    {
        return glm::distance2(min, max) == 0.F;
    }

    std::string BoundingBox::toString() const noexcept
    {
        return "BoundingBox(" + glm::to_string(min) + ", " + glm::to_string(max) + ")";
    }
}