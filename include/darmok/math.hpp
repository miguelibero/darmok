#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <bx/bx.h>

namespace darmok
{
	struct Viewport;

    struct DARMOK_EXPORT Math final
    {
        template<typename T>
        static T lerp(const T& a, const T& b, float p) noexcept
        {
            return a + (b * p);
        }

		template<typename T>
		static const T& clamp(const T& v, const T& min, const T& max) noexcept
		{
			auto& x = v < max ? v : max;
			return x < min ? min : x;
		}

		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static T vecMax(const glm::vec<L, T, Q>& v) noexcept
		{
			using vec = glm::vec<L, T, Q>;
			using val = T;
			val result = std::numeric_limits<val>::min();
			for (glm::length_t i = 0; i < vec::length(); i++)
			{
				auto j = v[i];
				if (j > result)
				{
					result = j;
				}
			}
			return result;
		}

		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static T vecMin(const glm::vec<L, T, Q>& v) noexcept
		{
			using vec = glm::vec<L, T, Q>;
			using val = T;
			val result = std::numeric_limits<val>::max();
			for (glm::length_t i = 0; i < vec::length(); i++)
			{
				auto j = v[i];
				if (j < result)
				{
					result = j;
				}
			}
			return result;
		}

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
		static [[nodiscard]] glm::mat4 ortho(const Viewport& vp, const glm::vec2& center = glm::vec2(0.5f), float near = -bx::kFloatLargest, float far = bx::kFloatLargest) noexcept;
		
		static [[nodiscard]] glm::mat4 frustrum(float left, float right, float bottom, float top, float near = -bx::kFloatLargest, float far = bx::kFloatLargest) noexcept;
		static [[nodiscard]] glm::mat4 frustrum(const glm::vec2& bottomLeft, const glm::vec2& rightTop, float near = -bx::kFloatLargest, float far = bx::kFloatLargest) noexcept;
        
		// methods used in Transform to generate the matrix
        static [[nodiscard]] glm::mat4 translateRotateScale(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale) noexcept;
        static [[nodiscard]] glm::mat4 transform(const glm::vec3& pos, const glm::quat& rot, const glm::vec3& scale, const glm::vec3& pivot = glm::vec3(0)) noexcept;
        static bool decompose(const glm::mat4& trans, glm::vec3& pos, glm::quat& rot, glm::vec3& scale, glm::vec3& pivot) noexcept;
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