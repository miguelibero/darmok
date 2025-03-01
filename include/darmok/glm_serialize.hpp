#pragma once

#include <darmok/glm.hpp>
#include <darmok/color.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/math.pb.h>

#include <glm/detail/type_quat.hpp>
#include <nlohmann/json.hpp>


namespace darmok
{
	namespace GlmSerializationUtils
	{
		protobuf::Uvec2 convert(const glm::uvec2& v);
		protobuf::Vec2 convert(const glm::vec2& v);
		protobuf::Vec3 convert(const glm::vec3& v);
		protobuf::Vec4 convert(const glm::vec4& v);
		protobuf::Mat3 convert(const glm::mat3& v);
		protobuf::Mat4 convert(const glm::mat4& v);
		protobuf::Color convert(const Color& v);
		protobuf::Color3 convert(const Color3& v);

		glm::uvec2 convert(const protobuf::Uvec2& v);
		glm::vec2 convert(const protobuf::Vec2& v);
		glm::vec3 convert(const protobuf::Vec3& v);
		glm::vec4 convert(const protobuf::Vec4& v);
		glm::mat3 convert(const protobuf::Mat3& v);
		glm::mat4 convert(const protobuf::Mat4& v);
		Color convert(const protobuf::Color& v);
		Color3 convert(const protobuf::Color3& v);
	}
}

namespace glm
{
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