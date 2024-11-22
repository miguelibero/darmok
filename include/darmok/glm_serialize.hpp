#pragma once

#include <darmok/glm.hpp>
#include <glm/detail/type_quat.hpp>
#include <cereal/cereal.hpp>

namespace glm
{
	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	static void serialize(Archive& archive, glm::vec<2, T, Q>& vec)
	{
		archive(
			CEREAL_NVP_("x", vec.x)
			, CEREAL_NVP_("y", vec.y)
		);
	}

	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	static void serialize(Archive& archive, glm::vec<3, T, Q>& vec)
	{
		archive(
			CEREAL_NVP_("x", vec.x)
			, CEREAL_NVP_("y", vec.y)
			, CEREAL_NVP_("z", vec.z)
		);
	}

	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	static void serialize(Archive& archive, glm::vec<4, T, Q>& vec)
	{
		archive(
			CEREAL_NVP_("x", vec.x)
			, CEREAL_NVP_("y", vec.y)
			, CEREAL_NVP_("z", vec.z)
			, CEREAL_NVP_("w", vec.w)
		);
	}

	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	static void serialize(Archive& archive, glm::qua<T, Q>& quat)
	{
		archive(
			CEREAL_NVP_("x", quat.x)
			, CEREAL_NVP_("y", quat.y)
			, CEREAL_NVP_("z", quat.z)
			, CEREAL_NVP_("w", quat.w)
		);
	}

	template<class Archive, glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
	static void serialize(Archive& archive, glm::mat<L1, L2, T, Q>& mat)
	{
		for (glm::length_t i = 0; i < L1; ++i)
		{
			archive(mat[i]);
		}
	}
}