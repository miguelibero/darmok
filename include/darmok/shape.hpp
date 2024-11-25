#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/utils.hpp>
#include <darmok/viewport.hpp>
#include <array>
#include <optional>
#include <string>
#include <vector>
#include <cereal/cereal.hpp>

namespace darmok
{
    struct Line;

    struct DARMOK_EXPORT Rectangle final
    {
        glm::vec2 size;
        glm::vec2 origin;

        Rectangle(const glm::vec2& size = glm::vec2(1), const glm::vec2& origin = glm::vec2(0)) noexcept;
        [[nodiscard]] std::string toString() const noexcept;
        [[nodiscard]] std::vector<Line> toLines() const noexcept;

        [[nodiscard]] static const Rectangle& standard() noexcept;

        Rectangle& operator*=(float scale) noexcept;
        [[nodiscard]] Rectangle operator*(float scale) const noexcept;

        [[nodiscard]] bool operator==(const Rectangle& other) const noexcept;
        [[nodiscard]] bool operator!=(const Rectangle& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(size), CEREAL_NVP(origin));
        }
    };

    struct BoundingBox;

    struct DARMOK_EXPORT Cube final
    {
        glm::vec3 size;
        glm::vec3 origin;

        Cube(const glm::vec3& size = glm::vec3(1), const glm::vec3& origin = glm::vec3(0)) noexcept;
        Cube(const BoundingBox& bbox) noexcept;

        [[nodiscard]] std::string toString() const noexcept;

        Cube& operator*=(float scale) noexcept;
        [[nodiscard]] Cube operator*(float scale) const noexcept;

        [[nodiscard]] bool operator==(const Cube& other) const noexcept;
        [[nodiscard]] bool operator!=(const Cube& other) const noexcept;

        [[nodiscard]] static const Cube& standard() noexcept;

        [[nodiscard]] bool empty() const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(size), CEREAL_NVP(origin));
        }
    };

    struct TextureTriangle;

    struct DARMOK_EXPORT Triangle final
    {
        using Vertices = std::array<glm::vec3, 3>;
        Vertices vertices;

        Triangle(const glm::vec3& vert1, const glm::vec3& vert2, const glm::vec3& vert3) noexcept;
        Triangle(const Vertices& vertices = {}) noexcept;
        [[nodiscard]] std::string toString() const noexcept;

        [[nodiscard]] glm::vec3 getNormal() const;
        [[nodiscard]] glm::vec3 getTangent(const TextureTriangle& texTri) const noexcept;
        [[nodiscard]] glm::vec3 getTangent(const TextureTriangle& texTri, const glm::vec3& normal) const noexcept;

        Triangle& operator*=(float scale) noexcept;
        [[nodiscard]] Triangle operator*(float scale) const noexcept;

        [[nodiscard]] bool operator==(const Triangle& other) const noexcept;
        [[nodiscard]] bool operator!=(const Triangle& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(vertices));
        }
    };

    struct DARMOK_EXPORT TextureTriangle final
    {
        using Coordinates = std::array<glm::vec2, 3>;
        Coordinates coordinates;

        TextureTriangle(const glm::vec2& coord1, const glm::vec2& coord2, const glm::vec2& coord3) noexcept;
        TextureTriangle(const Coordinates& coordinates = {}) noexcept;
        [[nodiscard]] std::string toString() const noexcept;

        TextureTriangle& operator*=(float scale) noexcept;
        [[nodiscard]] TextureTriangle operator*(float scale) const noexcept;

        [[nodiscard]] bool operator==(const TextureTriangle& other) const noexcept;
        [[nodiscard]] bool operator!=(const TextureTriangle& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(coordinates));
        }
    };

    struct DARMOK_EXPORT Polygon final
    {
        using Triangles = std::vector<Triangle>;

        Triangles triangles;
        glm::vec3 origin;

        Polygon(const Triangles& tris = {}, const glm::vec3& origin = glm::vec3(0)) noexcept;
        [[nodiscard]] std::string toString() const noexcept;

        Polygon& operator*=(float scale) noexcept;
        [[nodiscard]] Polygon operator*(float scale) const noexcept;

        [[nodiscard]] bool operator==(const Polygon& other) const noexcept;
        [[nodiscard]] bool operator!=(const Polygon& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(triangles), CEREAL_NVP(origin));
        }
    };

    struct DARMOK_EXPORT Sphere final
    {
        float radius;
        glm::vec3 origin;

        Sphere(const glm::vec3& origin, float radius = 0.5f) noexcept;
        Sphere(float radius = 0.5f, const glm::vec3& origin = glm::vec3(0)) noexcept;
        [[nodiscard]] std::string toString() const noexcept;

        [[nodiscard]] static const Sphere& standard() noexcept;

        Sphere& operator*=(float scale) noexcept;
        [[nodiscard]] Sphere operator*(float scale) const noexcept;

        [[nodiscard]] bool operator==(const Sphere& other) const noexcept;
        [[nodiscard]] bool operator!=(const Sphere& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(radius), CEREAL_NVP(origin));
        }
    };

    struct BoundingBox;

    struct DARMOK_EXPORT Plane final
    {
        glm::vec3 normal;
        float distance;

        Plane(const glm::vec3& normal = glm::vec3(0, 1, 0), float distance = 0.F) noexcept;
        Plane(const Triangle& tri) noexcept;
        [[nodiscard]] std::string toString() const noexcept;

        [[nodiscard]] Line getNormalLine() const noexcept;

        [[nodiscard]] glm::vec3 getOrigin() const noexcept;
        [[nodiscard]] glm::mat4 getTransform(const glm::vec3& up = glm::vec3(0, 1, 0)) const noexcept;
        [[nodiscard]] glm::vec3 getAlong() const noexcept;

        [[nodiscard]] Plane operator*(const glm::mat4& transform) const noexcept;
        Plane& operator*=(const glm::mat4& transform) noexcept;

        [[nodiscard]] bool operator==(const Plane& other) const noexcept;
        [[nodiscard]] bool operator!=(const Plane& other) const noexcept;

        [[nodiscard]] static const Plane& standard() noexcept;

        float signedDistanceTo(const glm::vec3& point) const noexcept;
        bool isInFront(const glm::vec3& point) const noexcept;
        bool isInFront(const BoundingBox& bbox) const noexcept;
        bool isInFront(const Sphere& sphere) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(normal), CEREAL_NVP(distance));
        }
    };

    struct DARMOK_EXPORT Grid final
    {
        Plane plane;
        glm::vec2 separation;
        glm::uvec2 amount;

        Grid(const Plane& plane = {}, const glm::vec2& separation = glm::vec2(1.F), const glm::uvec2& amount = glm::uvec2(10)) noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(plane), CEREAL_NVP(separation), CEREAL_NVP(amount));
        }
    };


    struct DARMOK_EXPORT Capsule final
    {
        float cylinderHeight;
        float radius;
        glm::vec3 origin;

        Capsule(float cylinderHeight = 1.F, float radius = 0.5F, const glm::vec3& origin = glm::vec3(0)) noexcept;
        [[nodiscard]] std::string toString() const noexcept;
        [[nodiscard]] static const Capsule& standard() noexcept;

        Capsule& operator*=(float scale) noexcept;
        [[nodiscard]] Capsule operator*(float scale) const noexcept;

        [[nodiscard]] bool operator==(const Capsule& other) const noexcept;
        [[nodiscard]] bool operator!=(const Capsule& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(cylinderHeight), CEREAL_NVP(radius), CEREAL_NVP(origin));
        }
    };

    struct DARMOK_EXPORT NormalIntersection final
    {
        glm::vec3 position = glm::vec3(0);
        glm::vec3 normal = glm::vec3(0, 1, 0);

        [[nodiscard]] std::string toString() const noexcept;

        [[nodiscard]] bool operator==(const NormalIntersection& other) const noexcept;
        [[nodiscard]] bool operator!=(const NormalIntersection& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(position), CEREAL_NVP(normal));
        }
    };

    struct DARMOK_EXPORT DistanceIntersection final
    {
        glm::vec2 position = glm::vec3(0);
        float distance = 0.F;

        [[nodiscard]] std::string toString() const noexcept;

        [[nodiscard]] bool operator==(const DistanceIntersection& other) const noexcept;
        [[nodiscard]] bool operator!=(const DistanceIntersection& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(position), CEREAL_NVP(distance));
        }
    };

    struct Line;

    struct DARMOK_EXPORT Ray final
    {
        glm::vec3 origin;
        glm::vec3 direction;

        Ray(const glm::vec3& origin = glm::vec3(0), const glm::vec3& dir = glm::vec3(0, 0, 1)) noexcept;
        Ray(const Line& line) noexcept;

        [[nodiscard]] Ray operator*(const glm::mat4& transform) const noexcept;
        Ray& operator*=(const glm::mat4& transform) noexcept;

        [[nodiscard]] glm::vec3 operator*(float dist) const noexcept;

        [[nodiscard]] bool operator==(const Ray& other) const noexcept;
        [[nodiscard]] bool operator!=(const Ray& other) const noexcept;

        [[nodiscard]] std::string toString() const noexcept;
        [[nodiscard]] Line toLine() const noexcept;
        [[nodiscard]] Ray& reverse() noexcept;

        // returns distance to ray origin
        [[nodiscard]] std::optional<float> intersect(const Plane& plane) const noexcept;

        // returns distance to ray origin
        [[nodiscard]] std::optional<float> intersect(const Sphere& sphere) const noexcept;

        [[nodiscard]] std::optional<NormalIntersection> intersectNormal(const Sphere& sphere) const noexcept;
        [[nodiscard]] std::optional<DistanceIntersection> intersect(const Triangle& tri) const noexcept;

        [[nodiscard]] static Ray unproject(const glm::vec2& screenPosition, const glm::mat4& model, const glm::mat4& proj, const Viewport& viewport = Viewport()) noexcept;
    
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(direction), CEREAL_NVP(origin));
        }
    };

    struct DARMOK_EXPORT Line final
    {
        using Points = std::array<glm::vec3, 2>;
        Points points;

        Line(const glm::vec3& point1 = glm::vec3(0), const glm::vec3& point2 = glm::vec3(0, 0, 1)) noexcept;
        Line(const Points& points) noexcept;
        Line(const Ray& ray) noexcept;

        [[nodiscard]] std::string toString() const noexcept;
        [[nodiscard]] Ray toRay() const noexcept;

        [[nodiscard]] glm::vec3 operator*(float dist) const noexcept;

        [[nodiscard]] bool operator==(const Line& other) const noexcept;
        [[nodiscard]] bool operator!=(const Line& other) const noexcept;

        [[nodiscard]] std::optional<std::array<NormalIntersection, 2>> intersect(const Sphere& sphere) const noexcept;
        [[nodiscard]] std::optional<glm::vec3> intersect(const Triangle& tri) const noexcept;
        [[nodiscard]] glm::vec3 closestPoint(const glm::vec3& p);

        [[nodiscard]] glm::mat4 getTransform(const glm::vec3& up = glm::vec3(0, 1, 0)) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(points));
        }
    };


    struct DARMOK_EXPORT Frustum final
    {
        enum class CornerType : size_t
        {
            NearBottomLeft,
            NearBottomRight,
            NearTopLeft,
            NearTopRight,
            FarBottomLeft,
            FarBottomRight,
            FarTopLeft,
            FarTopRight,
            Count
        };

        enum class PlaneType : size_t
        {
            Near,
            Far,
            Bottom,
            Top,
            Left,
            Right,
            Count
        };

        std::array<glm::vec3, toUnderlying(CornerType::Count)> corners;

        Frustum(const glm::mat4& mtx = glm::mat4(1), bool inverse = false) noexcept;

        [[nodiscard]] const glm::vec3& getCorner(Frustum::CornerType type) const noexcept;
        [[nodiscard]] glm::vec3& getCorner(Frustum::CornerType type) noexcept;

        [[nodiscard]] Plane getPlane(Frustum::PlaneType type) const noexcept;
        [[nodiscard]] std::array<Plane, toUnderlying(PlaneType::Count)> getPlanes() const noexcept;

        [[nodiscard]] std::string toString() const noexcept;
        [[nodiscard]] glm::vec3 getCenter() const noexcept;

        [[nodiscard]] BoundingBox getBoundingBox() const noexcept;

        [[nodiscard]] std::array<glm::vec3, 4> getSlopes() const noexcept;
        [[nodiscard]] Frustum getSlice(float nearFactor, float farFactor) const noexcept;

        [[nodiscard]] glm::mat4 getAlignedProjectionMatrix() const noexcept;

        [[nodiscard]] bool canSee(const BoundingBox& bbox) const noexcept;

        [[nodiscard]] Frustum operator*(const glm::mat4& trans) const noexcept;
        Frustum& operator*=(const glm::mat4& trans) noexcept;

        [[nodiscard]] bool operator==(const Frustum& other) const noexcept;
        [[nodiscard]] bool operator!=(const Frustum& other) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(corners);
        }
    };

    struct DARMOK_EXPORT BoundingBox final
    {
        glm::vec3 min;
        glm::vec3 max;

        BoundingBox() noexcept;
        BoundingBox(const glm::vec3& min, const glm::vec3& max) noexcept;
        BoundingBox(const Cube& cube) noexcept;
        BoundingBox(const Sphere& sphere) noexcept;
        BoundingBox(const Capsule& capsule) noexcept;
        BoundingBox(const Polygon& poly) noexcept;
        BoundingBox(const Triangle& tri) noexcept;
        BoundingBox(const Frustum& frust) noexcept;

        [[nodiscard]] bool operator==(const BoundingBox& other) const noexcept;
        [[nodiscard]] bool operator!=(const BoundingBox& other) const noexcept;

        BoundingBox& operator+=(const BoundingBox& bb) noexcept;
        [[nodiscard]] BoundingBox operator+(const BoundingBox& bb) const noexcept;

        [[nodiscard]] BoundingBox operator*(const glm::mat4& trans) const noexcept;
        BoundingBox& operator*=(const glm::mat4& trans) noexcept;

        BoundingBox& operator+=(const glm::vec3& v) noexcept;
        [[nodiscard]] BoundingBox operator+(const glm::vec3& v) const noexcept;

        BoundingBox& operator-=(const glm::vec3& v) noexcept;
        [[nodiscard]] BoundingBox operator-(const glm::vec3& v) const noexcept;

        BoundingBox& operator+=(const Triangle& v) noexcept;
        [[nodiscard]] BoundingBox operator+(const Triangle& v) const noexcept;

        BoundingBox& expand(const glm::vec3& size) noexcept;
        BoundingBox& contract(const glm::vec3& size) noexcept;
        BoundingBox& snap(const float size) noexcept;

        BoundingBox& expandToPosition(const glm::vec3& pos) noexcept;

        bool isOutsideOf(const Plane& plane) const noexcept;

        [[nodiscard]] std::array<glm::vec3, 8> getCorners() const noexcept;

        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] glm::vec3 size() const noexcept;

        [[nodiscard]] std::string toString() const noexcept;
        [[nodiscard]] glm::mat4 getOrtho() const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(min), CEREAL_NVP(max));
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

    inline std::string to_string(const darmok::Frustum& v)
    {
        return v.toString();
    }
}