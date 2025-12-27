#include <darmok/glm_serialize.hpp>

namespace darmok
{
	protobuf::Uvec2 Converter<protobuf::Uvec2, glm::uvec2>::run(const glm::uvec2& v) noexcept
	{
		protobuf::Uvec2 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		return vec;
	}

	protobuf::Vec2 Converter<protobuf::Vec2, glm::vec2>::run(const glm::vec2& v) noexcept
	{
		protobuf::Vec2 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		return vec;
	}

	protobuf::Vec3 Converter<protobuf::Vec3, glm::vec3>::run(const glm::vec3& v) noexcept
	{
		protobuf::Vec3 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		vec.set_z(v.z);
		return vec;
	}

	protobuf::Vec4 Converter<protobuf::Vec4, glm::vec4>::run(const glm::vec4& v) noexcept
	{
		protobuf::Vec4 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		vec.set_z(v.z);
		vec.set_w(v.w);
		return vec;
	}

	protobuf::Quat Converter<protobuf::Quat, glm::quat>::run(const glm::quat& v) noexcept
	{
		protobuf::Quat quat;
		quat.set_x(v.x);
		quat.set_y(v.y);
		quat.set_z(v.z);
		quat.set_w(v.w);
		return quat;
	}

	protobuf::Mat3 Converter<protobuf::Mat3, glm::mat3>::run(const glm::mat3& v) noexcept
	{
		protobuf::Mat3 mat;
		*mat.mutable_col1() = convert<protobuf::Vec3>(v[0]);
		*mat.mutable_col2() = convert<protobuf::Vec3>(v[1]);
		*mat.mutable_col3() = convert<protobuf::Vec3>(v[2]);
		return mat;
	}

	protobuf::Mat4 Converter<protobuf::Mat4, glm::mat4>::run(const glm::mat4& v) noexcept
	{
		protobuf::Mat4 mat;
		*mat.mutable_col1() = convert<protobuf::Vec4>(v[0]);
		*mat.mutable_col2() = convert<protobuf::Vec4>(v[1]);
		*mat.mutable_col3() = convert<protobuf::Vec4>(v[2]);
		*mat.mutable_col4() = convert<protobuf::Vec4>(v[3]);
		return mat;
	}

	protobuf::Color3 Converter<protobuf::Color3, Color3>::run(const Color3& v) noexcept
	{
		protobuf::Color3 color;
		color.set_r(v.r);
		color.set_g(v.g);
		color.set_b(v.b);
		return color;
	}

	protobuf::Color Converter<protobuf::Color, Color>::run(const Color& v) noexcept
	{
		protobuf::Color color;
		color.set_r(v.r);
		color.set_g(v.g);
		color.set_b(v.b);
		color.set_a(v.a);
		return color;
	}

	glm::uvec2 Converter<glm::uvec2, protobuf::Uvec2>::run(const protobuf::Uvec2& v) noexcept
	{
		return { v.x(), v.y() };
	}

	glm::vec2 Converter<glm::vec2, protobuf::Vec2>::run(const protobuf::Vec2& v) noexcept
	{
		return { v.x(), v.y() };
	}

	glm::vec3 Converter<glm::vec3, protobuf::Vec3>::run(const protobuf::Vec3& v) noexcept
	{
		return { v.x(), v.y(), v.z() };
	}

	glm::vec4 Converter<glm::vec4, protobuf::Vec4>::run(const protobuf::Vec4& v) noexcept
	{
		return { v.x(), v.y(), v.z(), v.w() };
	}

	glm::quat Converter<glm::quat, protobuf::Quat>::run(const protobuf::Quat& v) noexcept
	{
		return { v.w(), v.x(), v.y(), v.z() };
	}

	glm::mat3 Converter<glm::mat3, protobuf::Mat3>::run(const protobuf::Mat3& v) noexcept
	{
		return {
			convert<glm::vec3>(v.col1()),
			convert<glm::vec3>(v.col2()),
			convert<glm::vec3>(v.col3())
		};
	}

	glm::mat4 Converter<glm::mat4, protobuf::Mat4>::run(const protobuf::Mat4& v) noexcept
	{
		return {
			convert<glm::vec4>(v.col1()),
			convert<glm::vec4>(v.col2()),
			convert<glm::vec4>(v.col3()),
			convert<glm::vec4>(v.col4())
		};
	}

	Color Converter<Color, protobuf::Color>::run(const protobuf::Color& v) noexcept
	{
		return { v.r(), v.g(), v.b(), v.a() };
	}

	Color3 Converter<Color3, protobuf::Color3>::run(const protobuf::Color3& v) noexcept
	{
		return { v.r(), v.g(), v.b() };
	}
}