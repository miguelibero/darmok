#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <bx/bx.h>

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
		static bool almostZero(const glm::vec<L, glm::f32, Q>& v, int factor = 1) noexcept
		{
			for (glm::length_t i = 0; i < L; ++i)
			{
				if (!almostEqual(v[i], 0.F, factor))
				{
					return false;
				}
			}
			return true;
		}

		static bool almostEqual(float a, float b, int factor = 1) noexcept;
		static bool almostZero(float a, int factor = 1) noexcept;
		static [[nodiscard]] glm::mat4 flipHandedness(const glm::mat4& mat) noexcept;
		static [[nodiscard]] glm::quat flipHandedness(const glm::quat& quat) noexcept;

       /*
        * we cannot use the glm camera functions because they use opengl depth format 
        * and bgfx can run on different renderers so they have math functions with a depth parameter
        */
        static [[nodiscard]] glm::mat4 perspective(float fovy, float aspect, float near, float far) noexcept;
        static [[nodiscard]] glm::mat4 perspective(float fovy, float aspect, float near = 0.f) noexcept;
        static [[nodiscard]] glm::mat4 ortho(float left, float right, float bottom, float top, float near = -bx::kFloatLargest, float far = bx::kFloatLargest) noexcept;
		static [[nodiscard]] glm::mat4 ortho(const glm::vec2& bottomLeft, const glm::vec2& rightTop, float near = -bx::kFloatLargest, float far = bx::kFloatLargest) noexcept;
		
		static [[nodiscard]] glm::mat4 frustrum(float left, float right, float bottom, float top, float near = -bx::kFloatLargest, float far = bx::kFloatLargest) noexcept;
		static [[nodiscard]] glm::mat4 frustrum(const glm::vec2& bottomLeft, const glm::vec2& rightTop, float near = -bx::kFloatLargest, float far = bx::kFloatLargest) noexcept;
        
		// methods used in Transform to generate the matrix
        static [[nodiscard]] glm::mat4 transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) noexcept;
        static bool decompose(const glm::mat4& trans, glm::vec3& pos, glm::quat& rot, glm::vec3& scale) noexcept;
    
		static float distance(const glm::quat& rot1, const glm::quat& rot2) noexcept;
		static glm::vec3 moveTowards(const glm::vec3& current, const glm::vec3& target, float maxDistanceDelta) noexcept;
		static glm::vec3 rotateTowards(const glm::vec3& current, const glm::vec3& target, float maxRadiansDelta, float maxDistanceDelta) noexcept;
		static glm::quat rotateTowards(const glm::quat& current, const glm::quat& target, float maxRadiansDelta) noexcept;

		static glm::quat dirQuat(const glm::vec3& dir, const glm::vec3& forward = glm::vec3(0, 0, 1)) noexcept;
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