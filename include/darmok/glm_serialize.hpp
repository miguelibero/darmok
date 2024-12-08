#pragma once

#include <darmok/glm.hpp>

#include <glm/detail/type_quat.hpp>
#include <cereal/cereal.hpp>
#include <nlohmann/json.hpp>

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

	template<typename T, glm::qualifier Q = glm::defaultp>
	void to_json(nlohmann::json& json, const glm::vec<2, T, Q>& vec)
	{
		json = nlohmann::json{ {"x", vec.x}, {"y", vec.y} };
	}

	template<typename T, glm::qualifier Q = glm::defaultp>
	void from_json(const nlohmann::json& json, glm::vec<2, T, Q>& vec)
	{
		json.at("x").get_to(vec.x);
		json.at("y").get_to(vec.y);
	}

	template<typename T, glm::qualifier Q = glm::defaultp>
	void to_json(nlohmann::json& json, const glm::vec<3, T, Q>& vec)
	{
		json = nlohmann::json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z} };
	}

	template<typename T, glm::qualifier Q = glm::defaultp>
	void from_json(const nlohmann::json& json, glm::vec<3, T, Q>& vec)
	{
		json.at("x").get_to(vec.x);
		json.at("y").get_to(vec.y);
		json.at("z").get_to(vec.z);
	}

	template<typename T, glm::qualifier Q = glm::defaultp>
	void to_json(nlohmann::json& json, const glm::vec<4, T, Q>& vec)
	{
		json = nlohmann::json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z}, {"w", vec.w} };
	}

	template<typename T, glm::qualifier Q = glm::defaultp>
	void from_json(const nlohmann::json& json, glm::vec<4, T, Q>& vec)
	{
		json.at("x").get_to(vec.x);
		json.at("y").get_to(vec.y);
		json.at("z").get_to(vec.z);
		json.at("w").get_to(vec.w);
	}

	template<typename T, glm::qualifier Q = glm::defaultp>
	void to_json(nlohmann::json& json, const glm::qua<T, Q>& quat)
	{
		json = nlohmann::json{ {"x", quat.x}, {"y", quat.y}, {"z", quat.z}, {"w", quat.w} };
	}

	template<typename T, glm::qualifier Q = glm::defaultp>
	void from_json(const nlohmann::json& json, glm::qua<T, Q>& quat)
	{
		json.at("x").get_to(quat.x);
		json.at("y").get_to(quat.y);
		json.at("z").get_to(quat.z);
		json.at("w").get_to(quat.w);
	}

	template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
	void to_json(nlohmann::json& json, const glm::mat<L1, L2, T, Q>& mat)
	{
		json = nlohmann::json::array();
		for (glm::length_t i = 0; i < L1; ++i)
		{
			to_json(json[i], mat[i]);
		}
	}

	template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
	void from_json(const nlohmann::json& json, glm::mat<L1, L2, T, Q>& mat)
	{
		for (glm::length_t i = 0; i < L1; ++i)
		{
			from_json(json[i], mat[i]);
		}
	}
}