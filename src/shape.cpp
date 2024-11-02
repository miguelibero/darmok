#include <darmok/shape.hpp>
#include <darmok/string.hpp>
#include <darmok/math.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/norm.hpp>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <cmath>

namespace darmok
{
    const Rectangle& Rectangle::standard() noexcept
    {
        const static Rectangle v;
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

    bool Rectangle::operator==(const Rectangle& other) const noexcept
    {
        return size == other.size && origin == other.origin;
    }

    bool Rectangle::operator!=(const Rectangle& other) const noexcept
    {
        return !operator==(other);
    }

    Cube::Cube(const glm::vec3& size, const glm::vec3& origin) noexcept
        : size(size)
        , origin(origin)
    {
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

    bool Cube::operator==(const Cube& other) const noexcept
    {
        return size == other.size && origin == other.origin;
    }

    bool Cube::operator!=(const Cube& other) const noexcept
    {
        return !operator==(other);
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
        return normal;
    }

    glm::vec3 Triangle::getTangent(const TextureTriangle& texTri) const noexcept
    {
        glm::vec3 edge1 = vertices[1] - vertices[0];
        glm::vec3 edge2 = vertices[2] - vertices[0];
        glm::vec2 deltaUV1 = texTri.coordinates[1] - texTri.coordinates[0];
        glm::vec2 deltaUV2 = texTri.coordinates[2] - texTri.coordinates[0];

        glm::vec3 tangent;
        float r = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
        if (r != 0.0F)
        {
            tangent.x = (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x) / r;
            tangent.y = (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y) / r;
            tangent.z = (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z) / r;
        }
        tangent = glm::normalize(tangent);
        return tangent;
    }

    glm::vec3 Triangle::getTangent(const TextureTriangle& texTri, const glm::vec3& normal) const noexcept
    {
        auto tangent = getTangent(texTri);
        auto d = glm::dot(normal, tangent);
        if (!std::isnan(d))
        {
            tangent = glm::normalize(tangent - normal * d);
        }
        return tangent;
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

    bool Triangle::operator==(const Triangle& other) const noexcept
    {
        return vertices == other.vertices;
    }

    bool Triangle::operator!=(const Triangle& other) const noexcept
    {
        return !operator==(other);
    }

    TextureTriangle::TextureTriangle(const glm::vec2& coord1, const glm::vec2& coord2, const glm::vec2& coord3) noexcept
        : coordinates{ coord1, coord2, coord3 }
    {
    }

    TextureTriangle::TextureTriangle(const Coordinates& coordinates) noexcept
        : coordinates(coordinates)
    {
    }

    std::string TextureTriangle::toString() const noexcept
    {
        return "TextureTriangle(" + glm::to_string(coordinates[0]) + ", " + glm::to_string(coordinates[1]) + ", " + glm::to_string(coordinates[2]) + ")";
    }

    TextureTriangle& TextureTriangle::operator*=(float scale) noexcept
    {
        for (auto& coord : coordinates)
        {
            coord *= scale;
        }
        return *this;
    }

    TextureTriangle TextureTriangle::operator*(float scale) const noexcept
    {
        TextureTriangle tri = *this;
        tri *= scale;
        return tri;
    }

    bool TextureTriangle::operator==(const TextureTriangle& other) const noexcept
    {
        return coordinates == other.coordinates;
    }

    bool TextureTriangle::operator!=(const TextureTriangle& other) const noexcept
    {
        return !operator==(other);
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

    bool Polygon::operator==(const Polygon& other) const noexcept
    {
        return triangles == other.triangles;
    }

    bool Polygon::operator!=(const Polygon& other) const noexcept
    {
        return !operator==(other);
    }

    const Sphere& Sphere::standard() noexcept
    {
        const static Sphere v;
        return v;
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

    bool Sphere::operator==(const Sphere& other) const noexcept
    {
        return origin == other.origin && radius == other.radius;
    }

    bool Sphere::operator!=(const Sphere& other) const noexcept
    {
        return !operator==(other);
    }

    Plane::Plane(const glm::vec3& normal, float distance) noexcept
        : normal(normal)
        , distance(distance)
    {
    }

    Plane::Plane(const Triangle& tri) noexcept
        : normal(glm::normalize(tri.getNormal()))
        , distance(glm::dot(normal, tri.vertices[0]))
    {
    }

    float Plane::signedDistanceTo(const glm::vec3& point) const noexcept
    {
        return glm::dot(glm::normalize(normal), point) - distance;
    }

    bool Plane::isInFront(const glm::vec3& point) const noexcept
    {
        return signedDistanceTo(point) > 0;
    }

    bool Plane::isInFront(const BoundingBox& bbox) const noexcept
    {
        for (auto& corner : bbox.getCorners())
        {
            if (signedDistanceTo(corner) <= 0.F)
            {
                return false;
            }
        }
        return true;
    }

    bool Plane::isInFront(const Sphere& sphere) const noexcept
    {
        return signedDistanceTo(sphere.origin) > sphere.radius;
    }

    const Plane& Plane::standard() noexcept
    {
        const static Plane v;
        return v;
    }

    glm::vec3 Plane::getOrigin() const noexcept
    {
        auto len = glm::length(normal);
        return normal * distance / (len * len);
    }

    glm::mat4 Plane::getTransform(const glm::vec3& up) const noexcept
    {
        auto mtx = glm::mat4(1);
        mtx = glm::translate(mtx, getOrigin());
        mtx *= glm::mat4_cast(Math::quatLookAt(normal, up));
        return mtx;
    }

    std::string Plane::toString() const noexcept
    {
        return "Plane(" + glm::to_string(normal) + ", " + std::to_string(distance) + ")";
    }

    Line Plane::getNormalLine() const noexcept
    {
        auto origin = getOrigin();
        return Line(origin, origin + normal);
    }

    Plane Plane::operator*(const glm::mat4& transform) const noexcept
    {
        Plane copy(*this);
        copy *= transform;
        return copy;
    }

    Plane& Plane::operator*=(const glm::mat4& transform) noexcept
    {
        auto origin = getOrigin();
        auto torigin = transform * glm::vec4(origin, 1.F);
        origin = torigin / torigin.w;

        normal = transform * glm::vec4(normal, 0.F);
        distance = glm::dot(normal, origin);

        return *this;
    }

    bool Plane::operator==(const Plane& other) const noexcept
    {
        return distance == other.distance && normal == other.normal;
    }

    bool Plane::operator!=(const Plane& other) const noexcept
    {
        return !operator==(other);
    }

    Ray::Ray(const glm::vec3& origin, const glm::vec3& dir) noexcept
        : origin(origin)
        , direction(dir)
    {
    }

    Ray::Ray(const Line& line) noexcept
        : origin(line.points[1] - line.points[0])
        , direction(line.points[0])
    {
    }

    Ray Ray::operator*(const glm::mat4& transform) const noexcept
    {
        Ray copy(*this);
        copy *= transform;
        return copy;
    }

    Ray& Ray::operator*=(const glm::mat4& transform) noexcept
    {
        direction = transform * glm::vec4(direction, 0);
        origin = transform * glm::vec4(origin, 1);
        return *this;
    }

    std::string Ray::toString() const noexcept
    {
        return "Ray(origin=" + glm::to_string(origin) + ", direction=" + glm::to_string(direction) + ")";
    }

    Line Ray::toLine() const noexcept
    {
        return Line(origin, origin + direction);
    }

    Ray& Ray::reverse() noexcept
    {
        origin = origin + direction;
        direction *= -1.F;
        return *this;
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

    Ray Ray::unproject(const glm::vec2& screenPosition, const glm::mat4& model, const glm::mat4& proj, const Viewport& viewport) noexcept
    {
        auto near = glm::unProject(glm::vec3(screenPosition, 0), model, proj, viewport.getValues());
        auto far = glm::unProject(glm::vec3(screenPosition, 1), model, proj, viewport.getValues());

        return Ray(near, glm::normalize(far - near));
    }

    glm::vec3 Ray::operator*(float dist) const noexcept
    {
        return origin + (direction * dist);
    }

    bool Ray::operator==(const Ray& other) const noexcept
    {
        return origin == other.origin && direction == other.direction;
    }

    bool Ray::operator!=(const Ray& other) const noexcept
    {
        return !operator==(other);
    }

    Line::Line(const glm::vec3& point1, const glm::vec3& point2) noexcept
    : points{ point1, point2 }
    {
    }

    Line::Line(const Points& points) noexcept
        : points(points)
    {
    }

    Line::Line(const Ray& ray) noexcept
        : points{ ray.origin, ray.origin + ray.direction }
    {
    }

    std::string Line::toString() const noexcept
    {
        return "Line(" + glm::to_string(points[0]) + ", " + glm::to_string(points[1]) + ")";
    }

    Ray Line::toRay() const noexcept
    {
        return Ray(*this);
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

    bool NormalIntersection::operator==(const NormalIntersection& other) const noexcept
    {
        return position == other.position && normal == other.normal;
    }

    bool NormalIntersection::operator!=(const NormalIntersection& other) const noexcept
    {
        return !operator==(other);
    }

    std::string DistanceIntersection::toString() const noexcept
    {
        return "DistanceIntersection(position=" + glm::to_string(position) + ", distance=" + std::to_string(distance) + ")";
    }

    bool DistanceIntersection::operator==(const DistanceIntersection& other) const noexcept
    {
        return position == other.position && distance == other.distance;
    }

    bool DistanceIntersection::operator!=(const DistanceIntersection& other) const noexcept
    {
        return !operator==(other);
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

    glm::mat4 Line::getTransform(const glm::vec3& up) const noexcept
    {
        auto diff = points[1] - points[0];
        auto len = glm::length(diff);
        auto mtx = glm::mat4(len);
        mtx = glm::translate(mtx, points[0]);
        mtx *= glm::mat4_cast(Math::quatLookAt(diff / len, up));
        return mtx;
    }

    bool Line::operator==(const Line& other) const noexcept
    {
        return points == other.points;
    }

    bool Line::operator!=(const Line& other) const noexcept
    {
        return !operator==(other);
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

    bool Capsule::operator==(const Capsule& other) const noexcept
    {
        return cylinderHeight == other.cylinderHeight && radius == other.radius && origin == other.origin;
    }

    bool Capsule::operator!=(const Capsule& other) const noexcept
    {
        return !operator==(other);
    }

    BoundingBox::BoundingBox() noexcept
        : min(0), max(0)
    {
    }

    BoundingBox::BoundingBox(const glm::vec3& min, const glm::vec3& max) noexcept
        : min(min), max(max)
    {
    }

    BoundingBox::BoundingBox(const Cube& cube) noexcept
    {
        auto half = cube.size * 0.5F;
        min = cube.origin - half;
        max = cube.origin + half;
    }

    BoundingBox::BoundingBox(const Sphere& sphere) noexcept
    {
        auto half = glm::vec3(sphere.radius);
        min = sphere.origin - half;
        max = sphere.origin + half;
    }

    BoundingBox::BoundingBox(const Capsule& capsule) noexcept
    {
        auto half = glm::vec3(capsule.radius);
        half.y += capsule.cylinderHeight * 0.5F;
        min = capsule.origin - half;
        max = capsule.origin + half;
    }

    BoundingBox::BoundingBox(const Polygon& poly) noexcept
        : min(bx::kFloatInfinity)
        , max(-bx::kFloatInfinity)
    {
        if (poly.triangles.empty())
        {
            max = min = glm::vec3(0.F);
        }
        else
        {
            for (auto& tri : poly.triangles)
            {
                operator+=(tri);
            }
        }
    }

    BoundingBox::BoundingBox(const Triangle& tri) noexcept
        : min(bx::kFloatInfinity)
        , max(-bx::kFloatInfinity)
    {
        for (auto& v : tri.vertices)
        {
            expandToPosition(v);
        }
    }

    BoundingBox::BoundingBox(const Frustum& frust) noexcept
        : min(bx::kFloatInfinity)
        , max(-bx::kFloatInfinity)
    {
        for (auto& corner : frust.corners)
        {
            expandToPosition(corner);
        }
    }

    BoundingBox& BoundingBox::operator+=(const BoundingBox& bb) noexcept
    {
        min = glm::min(min, bb.min);
        max = glm::max(max, bb.max);
        return *this;
    }

    BoundingBox BoundingBox::operator+(const BoundingBox& bb) const noexcept
    {
        BoundingBox sum = *this;
        sum += bb;
        return sum;
    }

    BoundingBox& BoundingBox::operator+=(const glm::vec3& v) noexcept
    {
        min += v;
        max += v;
        return *this;
    }

    BoundingBox BoundingBox::operator+(const glm::vec3& v) const noexcept
    {
        BoundingBox r = *this;
        r += v;
        return r;
    }

    BoundingBox& BoundingBox::operator-=(const glm::vec3& v) noexcept
    {
        min -= v;
        max -= v;
        return *this;
    }

    BoundingBox BoundingBox::operator-(const glm::vec3& v) const noexcept
    {
        BoundingBox r = *this;
        r -= v;
        return r;
    }

    BoundingBox BoundingBox::operator*(const glm::mat4& trans) const noexcept
    {
        BoundingBox r(*this);
        r *= trans;
        return r;
    }

    BoundingBox& BoundingBox::operator+=(const Triangle& tri) noexcept
    {
        for (auto& v : tri.vertices)
        {
            expandToPosition(v);
        }
        return *this;
    }


    BoundingBox BoundingBox::operator+(const Triangle& tri) const noexcept
    {
        BoundingBox r(*this);
        r += tri;
        return r;
    }


    std::array<glm::vec3, 8> BoundingBox::getCorners() const noexcept
    {
        return {
            glm::vec3(min.x, min.y, min.z),
            glm::vec3(min.x, max.y, min.z),
            glm::vec3(min.x, min.y, max.z),
            glm::vec3(min.x, max.y, max.z),
            glm::vec3(max.x, min.y, min.z),
            glm::vec3(max.x, max.y, min.z),
            glm::vec3(max.x, min.y, max.z),
            glm::vec3(max.x, max.y, max.z)
        };
    }

    BoundingBox& BoundingBox::operator*=(const glm::mat4& trans) noexcept
    {
        auto corners = getCorners();

        min = glm::vec3(bx::kFloatInfinity);
        max = glm::vec3(-bx::kFloatInfinity);

        for (auto& corner : corners)
        {
            corner = trans * glm::vec4(corner, 1.0f);
            expandToPosition(corner);
        }

        return *this;
    }

    BoundingBox& BoundingBox::expand(const glm::vec3& size) noexcept
    {
        min -= size;
        max += size;
        return *this;
    }

    BoundingBox& BoundingBox::contract(const glm::vec3& size) noexcept
    {
        min += size;
        max -= size;
        return *this;
    }

    BoundingBox& BoundingBox::snap(const float snapSize) noexcept
    {
        auto f = (max - min) / snapSize;
        min = glm::floor(min / f) * f;
        max = glm::ceil(max / f) * f;
        return *this;
    }

    BoundingBox& BoundingBox::expandToPosition(const glm::vec3& pos) noexcept
    {
        min = glm::min(min, pos);
        max = glm::max(max, pos);
        return *this;
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

    glm::vec3 BoundingBox::size() const noexcept
    {
        return max - min;
    }

    std::string BoundingBox::toString() const noexcept
    {
        return "BoundingBox(" + glm::to_string(min) + ", " + glm::to_string(max) + ")";
    }

    glm::mat4 BoundingBox::getOrtho() const noexcept
    {
        return Math::ortho(min.x, max.x, min.y, max.y, min.z, max.z);
    }

    bool BoundingBox::operator==(const BoundingBox& other) const noexcept
    {
        return min == other.min && max == other.max;
    }

    bool BoundingBox::operator!=(const BoundingBox& other) const noexcept
    {
        return !operator==(other);
    }

    Frustum::Frustum(const glm::mat4& mtx, bool inverse) noexcept
    {
        auto nd = Math::getNormalizedNearDepth();

        corners = {
            glm::vec3(-1, -1, nd),
            glm::vec3( 1, -1, nd),
            glm::vec3(-1,  1, nd),
            glm::vec3( 1,  1, nd),
            glm::vec3(-1, -1, 1),
            glm::vec3( 1, -1, 1), 
            glm::vec3(-1,  1, 1),
            glm::vec3( 1,  1, 1)
        };

        auto invProj = inverse ? mtx : glm::inverse(mtx);

        for (auto& corner : corners)
        {
            auto tcorner = invProj * glm::vec4(corner, 1.0);
            corner = tcorner / tcorner.w;
        }
    }

    std::array<glm::vec3, 4> Frustum::getSlopes() const noexcept
    {
        return {
            getCorner(CornerType::FarBottomLeft) - getCorner(CornerType::NearBottomLeft),
            getCorner(CornerType::FarBottomRight) - getCorner(CornerType::NearBottomRight),
            getCorner(CornerType::FarTopLeft) - getCorner(CornerType::NearTopLeft),
            getCorner(CornerType::FarTopRight) - getCorner(CornerType::NearTopRight)
        };
    }

    Frustum Frustum::getSlice(float nearFactor, float farFactor) const noexcept
    {
        auto slopes = getSlopes();
        Frustum frust;

        auto& botLeft = getCorner(CornerType::NearBottomLeft);
        auto& botRight = getCorner(CornerType::NearBottomRight);
        auto& topLeft = getCorner(CornerType::NearTopLeft);
        auto& topRight = getCorner(CornerType::NearTopRight);

        frust.getCorner(CornerType::NearBottomLeft)     = botLeft   + (slopes[0] * nearFactor);
        frust.getCorner(CornerType::NearBottomRight)    = botRight  + (slopes[1] * nearFactor);
        frust.getCorner(CornerType::NearTopLeft)        = topLeft   + (slopes[2] * nearFactor);
        frust.getCorner(CornerType::NearTopRight)       = topRight  + (slopes[3] * nearFactor);

        frust.getCorner(CornerType::FarBottomLeft)      = botLeft   + (slopes[0] * farFactor);
        frust.getCorner(CornerType::FarBottomRight)     = botRight  + (slopes[1] * farFactor);
        frust.getCorner(CornerType::FarTopLeft)         = topLeft   + (slopes[2] * farFactor);
        frust.getCorner(CornerType::FarTopRight)        = topRight  + (slopes[3] * farFactor);

        return frust;
    }

    glm::mat4 Frustum::getAlignedProjectionMatrix() const noexcept
    {
        auto& nbl = getCorner(CornerType::NearBottomLeft);
        auto& nbr = getCorner(CornerType::NearBottomRight);
        auto& ntl = getCorner(CornerType::NearTopLeft);
        auto& fbl = getCorner(CornerType::FarBottomLeft);
        auto left = nbl.x;
        auto right = nbr.x;
        auto bottom = nbl.y;
        auto top = ntl.y;
        auto near = nbl.z;
        auto far = fbl.z;
        return Math::frustum(left, right, bottom, top, near, far);
    }

    bool Frustum::canSee(const BoundingBox& bbox) const noexcept
    {
        for (auto& plane : getPlanes())
        {
            if (plane.isInFront(bbox))
            {
                return false;
            }
        }
        return true;
    }

    const glm::vec3& Frustum::getCorner(Frustum::CornerType type) const noexcept
    {
        return corners[toUnderlying(type)];
    }

    glm::vec3& Frustum::getCorner(Frustum::CornerType type) noexcept
    {
        return corners[toUnderlying(type)];
    }

    Plane Frustum::getPlane(Frustum::PlaneType type) const noexcept
    {
        auto createPlane = [this](CornerType a, CornerType b, CornerType c)
        {
            return Plane(Triangle(getCorner(a), getCorner(b), getCorner(c)));
        };

        switch (type)
        {
        case PlaneType::Near:
            return createPlane(CornerType::NearTopRight, CornerType::NearBottomRight, CornerType::NearBottomLeft);
        case PlaneType::Far:
            return createPlane(CornerType::FarTopLeft, CornerType::FarBottomLeft, CornerType::FarBottomRight);
        case PlaneType::Bottom:
            return createPlane(CornerType::FarBottomLeft, CornerType::NearBottomLeft, CornerType::NearBottomRight);
        case PlaneType::Top:
            return createPlane(CornerType::FarTopRight, CornerType::NearTopRight, CornerType::NearTopLeft);
        case PlaneType::Left:
            return createPlane(CornerType::NearTopLeft, CornerType::NearBottomLeft, CornerType::FarBottomLeft);
        case PlaneType::Right:
            return createPlane(CornerType::FarTopRight, CornerType::FarBottomRight, CornerType::NearBottomRight);
        }
        return {};
    }

    std::array<Plane, toUnderlying(Frustum::PlaneType::Count)> Frustum::getPlanes() const noexcept
    {
        return {
            getPlane(PlaneType::Near),
            getPlane(PlaneType::Far),
            getPlane(PlaneType::Bottom),
            getPlane(PlaneType::Top),
            getPlane(PlaneType::Left),
            getPlane(PlaneType::Right),
        };
    }

    BoundingBox Frustum::getBoundingBox() const noexcept
    {
        BoundingBox bb(
            glm::vec3(bx::kFloatInfinity),
            glm::vec3(-bx::kFloatInfinity)
        );

        for (auto& corner : corners)
        {
            bb.min = glm::min(bb.min, corner);
            bb.max = glm::max(bb.max, corner);
        }

        return bb;
    }

    glm::vec3 Frustum::getCenter() const noexcept
    {
        glm::vec3 center = glm::vec3(0.0f);
        for (auto& corner : corners)
        {
            center += corner;
        }
        return center / float(corners.size());
    }

    std::string Frustum::toString() const noexcept
    {
        std::string str = "Frustum(";
        auto planes = getPlanes();
        str += StringUtils::join(", ", planes.begin(), planes.end(), [](auto& plane) {
            return plane.toString();
        });
        return str + ")";
    }

    Frustum Frustum::operator*(const glm::mat4& trans) const noexcept
    {
        Frustum r(*this);
        r *= trans;
        return r;
    }

    Frustum& Frustum::operator*=(const glm::mat4& trans) noexcept
    {
        for (auto& corner : corners)
        {
            corner = trans * glm::vec4(corner, 1.0f);
        }
        return *this;
    }

    bool Frustum::operator==(const Frustum& other) const noexcept
    {
        return corners == other.corners;
    }

    bool Frustum::operator!=(const Frustum& other) const noexcept
    {
        return !operator==(other);
    }
}