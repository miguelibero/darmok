#pragma once

#include <darmok/glm.hpp>
#include <darmok/color.hpp>
#include <darmok/convert.hpp>
#include <darmok/protobuf/math.pb.h>

#include <ostream>

#include <glm/detail/type_quat.hpp>
#include <glm/gtx/string_cast.hpp>
#include <nlohmann/json.hpp>
#include <fmt/format.h>

namespace darmok
{
	template<>
	struct Converter<protobuf::Uvec2, glm::uvec2>
	{
		static protobuf::Uvec2 run(const glm::uvec2& v) noexcept;
	};

	template<>
	struct Converter<protobuf::Vec2, glm::vec2>
	{
		static protobuf::Vec2 run(const glm::vec2& v) noexcept;
	};

	template<>
	struct Converter<protobuf::Vec3, glm::vec3>
	{
		static protobuf::Vec3 run(const glm::vec3& v) noexcept;
	};

	template<>
	struct Converter<protobuf::Vec4, glm::vec4>
	{
		static protobuf::Vec4 run(const glm::vec4& v) noexcept;
	};

	template<>
	struct Converter<protobuf::Quat, glm::quat>
	{
		static protobuf::Quat run(const glm::quat& v) noexcept;
	};

	template<>
	struct Converter<protobuf::Mat3, glm::mat3>
	{
		static protobuf::Mat3 run(const glm::mat3& v) noexcept;
	};

	template<>
	struct Converter<protobuf::Mat4, glm::mat4>
	{
		static protobuf::Mat4 run(const glm::mat4& v) noexcept;
	};

	template<>
	struct Converter<protobuf::Color, Color>
	{
		static protobuf::Color run(const Color& v) noexcept;
	};

	template<>
	struct Converter<protobuf::Color3, Color3>
	{
		static protobuf::Color3 run(const Color3& v) noexcept;
	};

	template<>
	struct Converter<glm::uvec2, protobuf::Uvec2>
	{
		static glm::uvec2 run(const protobuf::Uvec2& v) noexcept;
	};

	template<>
	struct Converter<glm::vec2, protobuf::Vec2>
	{
		static glm::vec2 run(const protobuf::Vec2& v) noexcept;
	};

	template<>
	struct Converter<glm::vec3, protobuf::Vec3>
	{
		static glm::vec3 run(const protobuf::Vec3& v) noexcept;
	};

	template<>
	struct Converter<glm::vec4, protobuf::Vec4>
	{
		static glm::vec4 run(const protobuf::Vec4& v) noexcept;
	};

	template<>
	struct Converter<glm::quat, protobuf::Quat>
	{
		static glm::quat run(const protobuf::Quat& v) noexcept;
	};

	template<>
	struct Converter<glm::mat3, protobuf::Mat3>
	{
		static glm::mat3 run(const protobuf::Mat3& v) noexcept;
	};

	template<>
	struct Converter<glm::mat4, protobuf::Mat4>
	{
		static glm::mat4 run(const protobuf::Mat4& v) noexcept;
	};

	template<>
	struct Converter<Color, protobuf::Color>
	{
		static Color run(const protobuf::Color& v) noexcept;
	};

	template<>
	struct Converter<Color3, protobuf::Color3>
	{
		static Color3 run(const protobuf::Color3& v) noexcept;
	};
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