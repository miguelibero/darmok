#include <darmok/math.hpp>
#include <bx/math.h>
#include <bgfx/bgfx.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <algorithm>

namespace darmok
{
    bool Math::almostEqual(float a, float b, float threshold) noexcept
    {
        return std::abs(a - b) < threshold;
    }

    bool Math::almostZero(float a, float threshold) noexcept
    {
        return almostEqual(a, 0, threshold);
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

    float Math::getNormalizedNearDepth() noexcept
    {
        auto hd = bgfx::getCaps()->homogeneousDepth;
        return hd ? -1.F : 0.F;
    }

    const float Math::defaultPerspNear = 0.1F;
    const float Math::defaultPerspFar = 1000.F;

    const float Math::defaultOrthoNear = -0.1F;
    const float Math::defaultOrthoFar = 1000.F;

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

    glm::mat4 Math::ortho(const glm::vec2& bottomLeft, const glm::vec2& topRight, float near, float far) noexcept
    {
        return ortho(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, near, far);
    }

    glm::vec2 Math::projDepthRange(const glm::mat4& proj) noexcept
    {
        auto c = proj[2][2];
        auto d = proj[3][2];
        if (!bgfx::getCaps()->homogeneousDepth)
        {
            return glm::vec2( -d / c, -d / (c - 1.F));
        }
        return glm::vec2( -d / (c - 1.F), -d / (c + 1.F));
    }

    glm::vec2 Math::orthoDepthRange(float z, float depth) noexcept
    {
        if (z == 0.F || depth == 0.F)
        {
            return glm::vec2(0.F, 1.F);
        }
        if (!bgfx::getCaps()->homogeneousDepth)
        {
            return glm::vec2(0.F, z / depth);
        }
        return glm::vec2(0.F, 2.F * z / (depth + 1.F));
    }

    glm::mat4 Math::frustum(float left, float right, float bottom, float top, float near, float far) noexcept
    {
        auto proj = glm::frustum(left, right, bottom, top, near, far);
        if (!bgfx::getCaps()->homogeneousDepth)
        {
            proj = glm::translate(glm::vec3(0, 0, 0.5)) * glm::scale(glm::vec3(1, 1, 0.5)) * proj;
        }
        return proj;
    }

    glm::mat4 Math::frustum(const glm::vec2& bottomLeft, const glm::vec2& topRight, float near, float far) noexcept
    {
        return frustum(bottomLeft.x, topRight.x, bottomLeft.y, topRight.y, near, far);
    }

    glm::mat4 Math::changeProjDepth(const glm::mat4& proj, float near, float far) noexcept
    {
        float fov = 2.0f * atan(1.0f / proj[1][1]);
        float aspectRatio = proj[1][1] / proj[0][0];
        return glm::perspective(fov, aspectRatio, near, far);
    }

    bool Math::almostEqualAngle(float a, float b, float threshold) noexcept
    {
        auto p = glm::two_pi<float>();
        return almostEqual(fmod(a, p), fmod(b, p), threshold);
    }

    glm::quat Math::quatLookAt(const glm::vec3& direction, const glm::vec3& up, float threshold) noexcept
    {
        auto normDir = glm::normalize(direction);
        auto normUp = glm::normalize(up);
        glm::vec3 fixedUp = normUp;
        float angle = glm::angle(normDir, fixedUp);
        auto p = glm::pi<float>();
        if (angle < threshold || angle > p - threshold)
        {
            fixedUp = glm::vec3(0.0f, 0.0f, 1.0f);
            angle = glm::angle(normDir, fixedUp);
            if (angle < threshold || angle > p - threshold)
            {
                fixedUp = glm::vec3(1.0f, 0.0f, 0.0f);
            }
        }
        return glm::quatLookAt(normDir, fixedUp);
    }

    glm::vec3 Math::getAlongNormal(const glm::vec3& normal) noexcept
    {
        static const glm::vec3 yaxis(0, 1, 0);
        static const glm::vec3 zaxis(0, 0, 1);
        auto n = glm::normalize(normal);
        auto candidate = (n != yaxis) ? yaxis : zaxis;
        return glm::cross(n, candidate);
    }

    glm::mat4 Math::transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) noexcept
    {
        return glm::translate(pos)
            * glm::mat4_cast(rot)
            * glm::scale(scale);
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
        auto delta = target - current;
        auto dist = glm::length(delta);

        if (std::abs(dist) <= maxDistanceDelta)
        {
            return target;
        }
        float t = std::min(1.0f, maxDistanceDelta / dist);
        return current + lerp(glm::vec3(0), delta, t);
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