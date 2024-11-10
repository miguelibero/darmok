#pragma once

#include <darmok/glm.hpp>
#include <cereal/cereal.hpp>

namespace glm
{
	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	static void serialize(Archive& archive, glm::vec<2, T, Q>& vec)
	{
		archive(
			cereal::make_nvp("x", vec.x)
			, cereal::make_nvp("y", vec.y)
		);
	}

	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	static void serialize(Archive& archive, glm::vec<3, T, Q>& vec)
	{
		archive(
			cereal::make_nvp("x", vec.x)
			, cereal::make_nvp("y", vec.y)
			, cereal::make_nvp("z", vec.z)
		);
	}

	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	static void serialize(Archive& archive, glm::vec<4, T, Q>& vec)
	{
		archive(
			cereal::make_nvp("x", vec.x)
			, cereal::make_nvp("y", vec.y)
			, cereal::make_nvp("z", vec.z)
			, cereal::make_nvp("w", vec.w)
		);
	}

	template<class Archive, typename T, glm::qualifier Q = glm::defaultp>
	static void serialize(Archive& archive, glm::qua<T, Q>& quat)
	{
		archive(
			cereal::make_nvp("x", quat.x)
			, cereal::make_nvp("y", quat.y)
			, cereal::make_nvp("z", quat.z)
			, cereal::make_nvp("w", quat.w)
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