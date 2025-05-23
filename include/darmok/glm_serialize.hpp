#pragma once

#include <darmok/glm.hpp>
#include <darmok/color.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/math.pb.h>

#include <ostream>

#include <glm/detail/type_quat.hpp>
#include <glm/gtx/string_cast.hpp>
#include <nlohmann/json.hpp>
#include <fmt/format.h>


namespace darmok
{
	namespace protobuf
	{
		Uvec2 convert(const glm::uvec2& v);
		Vec2 convert(const glm::vec2& v);
		Vec3 convert(const glm::vec3& v);
		Vec4 convert(const glm::vec4& v);
		Mat3 convert(const glm::mat3& v);
		Mat4 convert(const glm::mat4& v);
		Color convert(const darmok::Color& v);
		Color3 convert(const darmok::Color3& v);

		glm::uvec2 convert(const Uvec2& v);
		glm::vec2 convert(const Vec2& v);
		glm::vec3 convert(const Vec3& v);
		glm::vec4 convert(const Vec4& v);
		glm::mat3 convert(const Mat3& v);
		glm::mat4 convert(const Mat4& v);
		darmok::Color convert(const Color& v);
		darmok::Color3 convert(const Color3& v);
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
		if (json.is_number())
		{
			vec.x = vec.y = json.get<T>();
		}
		else if (json.is_array())
		{
			if (json.size() == 1)
			{
				vec.x = vec.y = json[0].get<T>();
			}
			else
			{
				json.at(0).get_to(vec.x);
				json.at(1).get_to(vec.y);
			}
		}
		else
		{
			json.at("x").get_to(vec.x);
			json.at("y").get_to(vec.y);
		}
	}

	template<typename T, glm::qualifier Q = glm::defaultp>
	void to_json(nlohmann::json& json, const glm::vec<3, T, Q>& vec)
	{
		json = nlohmann::json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z} };
	}

	template<typename T, glm::qualifier Q = glm::defaultp>
	void from_json(const nlohmann::json& json, glm::vec<3, T, Q>& vec)
	{
		if (json.is_number())
		{
			vec.x = vec.y = vec.z = json.get<T>();
		}
		else if (json.is_array())
		{
			if (json.size() == 1)
			{
				vec.x = vec.y = vec.z = json[0].get<T>();
			}
			else
			{
				json.at(0).get_to(vec.x);
				json.at(1).get_to(vec.y);
				json.at(2).get_to(vec.z);
			}
		}
		else
		{
			json.at("x").get_to(vec.x);
			json.at("y").get_to(vec.y);
			json.at("z").get_to(vec.z);
		}

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

namespace std
{
	template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
	inline ostream& operator<<(ostream& out, const glm::vec<L, T, Q>& vec)
	{
		return out << glm::to_string(vec);
	}

	template<typename T, glm::qualifier Q = glm::defaultp>
	inline ostream& operator<<(ostream& out, const glm::qua<T, Q>& quat)
	{
		return out << glm::to_string(quat);
	}

	template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
	inline ostream& operator<<(ostream& out, const glm::mat<L1, L2, T, Q>& mat)
	{
		return out << glm::to_string(mat);
	}
}

namespace fmt
{
	template<glm::length_t L, typename T, glm::qualifier Q>
	struct formatter<glm::vec<L, T, Q>> : public formatter<std::string_view>
	{
		template <typename FormatContext>
		auto format(const glm::vec<L, T, Q>& v, FormatContext& ctx) const
		{
			return formatter<std::string_view>::format(glm::to_string(v), ctx);
		}
	};

	template<typename T, glm::qualifier Q>
	struct formatter<glm::qua<T, Q>> : public formatter<std::string_view>
	{
		template <typename FormatContext>
		auto format(const glm::qua<T, Q>& v, FormatContext& ctx) const
		{
			return formatter<std::string_view>::format(glm::to_string(v), ctx);
		}
	};

	template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q>
	struct formatter<glm::mat<L1, L2, T, Q>> : public formatter<std::string_view>
	{
		template <typename FormatContext>
		auto format(const glm::mat<L1, L2, T, Q>& v, FormatContext& ctx) const
		{
			return formatter<std::string_view>::format(glm::to_string(v), ctx);
		}
	};
}