#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <bx/bx.h>
#include <glm/gtc/quaternion.hpp>

namespace darmok
{
    struct DARMOK_EXPORT Math final
    {
        template<typename T>
        static T lerp(const T& a, const T& b, float p) noexcept
        {
            return a + (p * (b - a));
        }

		template<typename T>
		static const T& clamp(const T& v, const T& min, const T& max) noexcept
		{
			auto& x = v < max ? v : max;
			return x < min ? min : x;
		}

		template<glm::length_t L, glm::qualifier Q = glm::defaultp>
		static bool almostZero(const glm::vec<L, glm::f32, Q>& v, float threshold = 0.01F) noexcept
		{
			for (glm::length_t i = 0; i < L; ++i)
			{
				if (!almostZero(v[i], threshold))
				{
					return false;
				}
			}
			return true;
		}

		template<glm::length_t L1, glm::length_t L2, glm::qualifier Q = glm::defaultp>
		static bool almostZero(const glm::mat<L1, L2, glm::f32, Q>& v, float threshold = 0.01F) noexcept
		{
			for (glm::length_t i = 0; i < L1; ++i)
			{
				if (!almostZero(v[i], threshold))
				{
					return false;
				}
			}
			return true;
		}

		template<glm::qualifier Q = glm::defaultp>
		static bool almostZero(const glm::qua<glm::f32, Q>& v, float threshold = 0.01F) noexcept
		{
			for (glm::length_t i = 0; i < v.length(); ++i)
			{
				if (!almostZero(v[i], threshold))
				{
					return false;
				}
			}
			return true;
		}

		static bool almostZero(float a, float threshold = 0.01F) noexcept;

		template<glm::length_t L, glm::qualifier Q = glm::defaultp>
		static bool almostEqual(const glm::vec<L, glm::f32, Q>& a, const glm::vec<L, glm::f32, Q>& b, float threshold = 0.01F) noexcept
		{
			for (glm::length_t i = 0; i < L; ++i)
			{
				if (!almostEqual(a[i], b[i], threshold))
				{
					return false;
				}
			}
			return true;
		}

		template<glm::length_t L1, glm::length_t L2, glm::qualifier Q = glm::defaultp>
		static bool almostEqual(const glm::mat<L1, L2, glm::f32, Q>& a, const glm::mat<L1, L2, glm::f32, Q>& b, float threshold = 0.01F) noexcept
		{
			for (glm::length_t i = 0; i < L1; ++i)
			{
				if (!almostEqual(a[i], b[i], threshold))
				{
					return false;
				}
			}
			return true;
		}

		template<glm::qualifier Q = glm::defaultp>
		static bool almostEqual(const glm::qua<glm::f32, Q>& a, const glm::qua<glm::f32, Q>& b, float threshold = 0.01F) noexcept
		{
			for (glm::length_t i = 0; i < a.length(); ++i)
			{
				if (!almostEqual(a[i], b[i], threshold))
				{
					return false;
				}
			}
			return true;
		}

		static bool almostEqual(float a, float b, float threshold = 0.01F) noexcept;

		[[nodiscard]] static glm::mat4 flipHandedness(const glm::mat4& mat) noexcept;
		[[nodiscard]] static glm::quat flipHandedness(const glm::quat& quat) noexcept;

       /*
        * we cannot use the glm camera functions because they use opengl depth format 
        * and bgfx can run on different renderers so they have math functions with a depth parameter
        */
		static const float defaultPerspNear;
		static const float defaultPerspFar;

		static const float defaultOrthoNear;
		static const float defaultOrthoFar;


		[[nodiscard]] static float getNormalizedNearDepth() noexcept;
        [[nodiscard]] static glm::mat4 perspective(float fovy, float aspect, float near, float far) noexcept;
        [[nodiscard]] static glm::mat4 perspective(float fovy, float aspect, float near = defaultPerspNear) noexcept;
        [[nodiscard]] static glm::mat4 ortho(float left, float right, float bottom, float top, float near = defaultOrthoNear, float far = defaultOrthoFar) noexcept;
		[[nodiscard]] static glm::mat4 ortho(const glm::vec2& bottomLeft, const glm::vec2& topRight, float near = defaultOrthoNear, float far = defaultOrthoFar) noexcept;
		[[nodiscard]] static glm::vec2 projDepthRange(const glm::mat4& proj) noexcept;

		// calc orthographic depth planes so that a given world z is set on a given depth
		[[nodiscard]] static glm::vec2 orthoDepthRange(float z, float depth) noexcept;

		[[nodiscard]] static glm::mat4 frustum(float left, float right, float bottom, float top, float near = defaultPerspNear, float far = defaultPerspFar) noexcept;
		[[nodiscard]] static glm::mat4 frustum(const glm::vec2& bottomLeft, const glm::vec2& topRight, float near = defaultPerspNear, float far = defaultPerspFar) noexcept;

		// methods used in Transform to generate the matrix
        [[nodiscard]] static glm::mat4 transform(const glm::vec3& pos, const glm::quat& rot = glm::identity<glm::quat>(), const glm::vec3& scale = glm::vec3(1)) noexcept;
        static bool decompose(const glm::mat4& trans, glm::vec3& pos, glm::quat& rot, glm::vec3& scale) noexcept;
    
		static float distance(const glm::quat& rot1, const glm::quat& rot2) noexcept;
		static glm::vec3 moveTowards(const glm::vec3& current, const glm::vec3& target, float maxDistanceDelta) noexcept;
		static glm::vec3 rotateTowards(const glm::vec3& current, const glm::vec3& target, float maxRadiansDelta, float maxDistanceDelta) noexcept;
		static glm::quat rotateTowards(const glm::quat& current, const glm::quat& target, float maxRadiansDelta) noexcept;

		static glm::mat4 changeProjDepth(const glm::mat4& proj, float near = defaultPerspNear, float far = defaultPerspFar) noexcept;

		static bool almostEqualAngle(float a, float b, float threshold = 0.01F) noexcept;
		static glm::quat quatLookAt(const glm::vec3& direction, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f), float threshold = 0.01F) noexcept;
	};
}

namespace glm
{
	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	void serialize(Archive& archive, glm::vec<2, T, Q>& vec)
	{
		archive(vec.x, vec.y);
	}

	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	void serialize(Archive& archive, glm::vec<3, T, Q>& vec)
	{
		archive(vec.x, vec.y, vec.z);
	}

	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	void serialize(Archive& archive, glm::vec<4, T, Q>& vec)
	{
		archive(vec.x, vec.y, vec.z, vec.w);
	}

	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	void serialize(Archive& archive, glm::mat<3, 3, T, Q>& mat)
	{
		archive(mat[0], mat[1], mat[2]);
	}

	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	void serialize(Archive& archive, glm::mat<4, 4, T, Q>& mat)
	{
		archive(mat[0], mat[1], mat[2], mat[3]);
	}
}