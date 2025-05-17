#include <darmok/glm_serialize.hpp>

namespace darmok
{
	protobuf::Uvec2 protobuf::convert(const glm::uvec2& v)
	{
		protobuf::Uvec2 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		return vec;
	}

	protobuf::Vec2 protobuf::convert(const glm::vec2& v)
	{
		protobuf::Vec2 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		return vec;
	}

	protobuf::Vec3 protobuf::convert(const glm::vec3& v)
	{
		protobuf::Vec3 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		vec.set_z(v.z);
		return vec;
	}

	protobuf::Vec4 protobuf::convert(const glm::vec4& v)
	{
		protobuf::Vec4 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		vec.set_z(v.z);
		vec.set_w(v.w);
		return vec;
	}

	protobuf::Mat3 protobuf::convert(const glm::mat3& v)
	{
		protobuf::Mat3 mat;
		*mat.mutable_col1() = convert(v[0]);
		*mat.mutable_col2() = convert(v[1]);
		*mat.mutable_col3() = convert(v[2]);
		return mat;
	}

	protobuf::Mat4 protobuf::convert(const glm::mat4& v)
	{
		protobuf::Mat4 mat;
		*mat.mutable_col1() = convert(v[0]);
		*mat.mutable_col2() = convert(v[1]);
		*mat.mutable_col3() = convert(v[2]);
		*mat.mutable_col4() = convert(v[3]);
		return mat;
	}

	protobuf::Color3 protobuf::convert(const darmok::Color3& v)
	{
		protobuf::Color3 color;
		color.set_r(v.r);
		color.set_g(v.g);
		color.set_b(v.b);
		return color;
	}

	protobuf::Color protobuf::convert(const darmok::Color& v)
	{
		protobuf::Color color;
		color.set_r(v.r);
		color.set_g(v.g);
		color.set_b(v.b);
		color.set_a(v.a);
		return color;
	}

	glm::uvec2 protobuf::convert(const protobuf::Uvec2& v)
	{
		return { v.x(), v.y() };
	}

	glm::vec2 protobuf::convert(const protobuf::Vec2& v)
	{
		return { v.x(), v.y() };
	}

	glm::vec3 protobuf::convert(const protobuf::Vec3& v)
	{
		return { v.x(), v.y(), v.z() };
	}

	glm::vec4 protobuf::convert(const protobuf::Vec4& v)
	{
		return { v.x(), v.y(), v.z(), v.w() };
	}

	glm::mat3 protobuf::convert(const protobuf::Mat3& v)
	{
		return {
			convert(v.col1()),
			convert(v.col2()),
			convert(v.col3())
		};
	}

	glm::mat4 protobuf::convert(const protobuf::Mat4& v)
	{
		return {
			convert(v.col1()),
			convert(v.col2()),
			convert(v.col3()),
			convert(v.col4())
		};
	}

	darmok::Color protobuf::convert(const protobuf::Color& v)
	{
		return { v.r(), v.g(), v.b(), v.a() };
	}

	darmok::Color3 protobuf::convert(const protobuf::Color3& v)
	{
		return { v.r(), v.g(), v.b() };
	}
}