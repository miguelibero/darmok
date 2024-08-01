#include <darmok/math.hpp>
#include <bx/math.h>
#include <bgfx/bgfx.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace darmok
{
    bool Math::almostEqual(float a, float b, int factor) noexcept
    {
        double min_a = a - (a - std::nextafter(a, std::numeric_limits<float>::lowest())) * factor;
        double max_a = a + (std::nextafter(a, std::numeric_limits<float>::max()) - a) * factor;
        return min_a <= b && max_a >= b;
    }

    bool Math::almostZero(float a, int factor) noexcept
    {
        return almostEqual(a, 0, factor);
    }

    glm::mat4 Math::flipHandedness(const glm::mat4& mat) noexcept
    {
        static const glm::mat4 zNegate = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, -1.0f));
        return zNegate * mat * zNegate;
    }

    glm::quat Math::flipHandedness(const glm::quat& quat) noexcept
    {
        return glm::conjugate(quat);
    }

    glm::mat4 Math::perspective(float fovy, float aspect, float near, float far) noexcept
    {
        glm::mat4 v;
        bx::mtxProj(glm::value_ptr(v), bx::toDeg(fovy), aspect, near, far, bgfx::getCaps()->homogeneousDepth);
        return v;
    }

    glm::mat4 Math::perspective(float fovy, float aspect, float near) noexcept
    {
        glm::mat4 v;
        bx::mtxProjInf(glm::value_ptr(v), bx::toDeg(fovy), aspect, near, bgfx::getCaps()->homogeneousDepth);
        return v;
    }

    glm::mat4 Math::ortho(float left, float right, float bottom, float top, float near, float far) noexcept
    {
        glm::mat4 v;
        bx::mtxOrtho(glm::value_ptr(v), left, right, bottom, top, near, far, 0.F, bgfx::getCaps()->homogeneousDepth);
        return v;
    }

    glm::mat4 Math::ortho(const glm::vec2& bottomLeft, const glm::vec2& rightTop, float near, float far) noexcept
    {
        return ortho(bottomLeft.x, rightTop.x, bottomLeft.y, rightTop.y, near, far);
    }

    glm::mat4 Math::frustrum(float left, float right, float bottom, float top, float near, float far) noexcept
    {
        // TODO: check how to calculate glm::frustrum with bx
        return glm::frustum(left, right, bottom, top, near, far);
    }

    glm::mat4 Math::frustrum(const glm::vec2& bottomLeft, const glm::vec2& rightTop, float near, float far) noexcept
    {
        return frustrum(bottomLeft.x, rightTop.x, bottomLeft.y, rightTop.y, near, far);
    }

    glm::mat4 Math::translateRotateScale(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) noexcept
    {
        return transform(pos, rot, scale);
    }

    glm::mat4 Math::transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const glm::vec3& pivot) noexcept
    {
        return glm::translate(pos)
            * glm::mat4_cast(rot)
            * glm::scale(scale)
            * glm::translate(-pivot);
    }

    bool Math::decompose(const glm::mat4& trans, glm::vec3& pos, glm::quat& rot, glm::vec3& scale) noexcept
    {
        glm::vec3 skew{};
        glm::vec4 perspective{};
        glm::decompose(trans, scale, rot, pos, skew, perspective);
        // TODO: check skew == [0, 0, 0] && persp == [0, 0, 0, 1]
        return true;
    }

    float Math::distance(const glm::quat& rot1, const glm::quat& rot2) noexcept
    {
        float v = glm::dot(rot1, rot2);
        v = clamp(v, -1.0f, 1.0f);
        return 2.F * std::acos(std::abs(v));
    }

    glm::vec3 Math::moveTowards(const glm::vec3& current, const glm::vec3& target, float maxDistanceDelta) noexcept
    {
        // TODO: check this works OK, has not been tested
        auto dist = glm::distance(current, target);

        if (std::abs(dist) <= maxDistanceDelta)
        {
            return target;
        }
        float t = std::min(1.0f, maxDistanceDelta / dist);
        return lerp(current, target, t);
    }

    glm::vec3 Math::rotateTowards(const glm::vec3& current, const glm::vec3& target, float maxRadiansDelta, float maxDistanceDelta) noexcept
    {
        // TODO: check this works OK, has not been tested
        auto normCurrent = glm::normalize(current);
        auto normTarget = glm::normalize(target);

        float dot = glm::dot(normCurrent, normTarget);
        dot = clamp(dot, -1.0f, 1.0f);
        float angle = std::acos(dot);
        if (std::abs(angle) < maxRadiansDelta)
        {
            return normTarget;
        }
        float t = std::min(1.0f, maxRadiansDelta / angle);

        glm::quat currentQuat(normCurrent);
        glm::quat targetQuat(normTarget);
        currentQuat = glm::slerp(currentQuat, targetQuat, t);

        return currentQuat * glm::vec3(0.0f, 0.0f, 1.0f);
    }
        
    glm::quat Math::rotateTowards(const glm::quat& current, const glm::quat& target, float maxRadiansDelta) noexcept
    {
        auto dist = distance(current, target);
        if (std::abs(dist) < maxRadiansDelta)
        {
            return target;
        }

        float t = std::min(1.0f, maxRadiansDelta / dist);
        return glm::slerp(current, target, t);

        return target;
    }
}