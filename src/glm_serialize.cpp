#include <darmok/glm_serialize.hpp>

namespace darmok
{
	protobuf::Uvec2 GlmSerializationUtils::convert(const glm::uvec2& v)
	{
		protobuf::Uvec2 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		return vec;
	}

	protobuf::Vec2 GlmSerializationUtils::convert(const glm::vec2& v)
	{
		protobuf::Vec2 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		return vec;
	}

	protobuf::Vec3 GlmSerializationUtils::convert(const glm::vec3& v)
	{
		protobuf::Vec3 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		vec.set_z(v.z);
		return vec;
	}

	protobuf::Vec4 GlmSerializationUtils::convert(const glm::vec4& v)
	{
		protobuf::Vec4 vec;
		vec.set_x(v.x);
		vec.set_y(v.y);
		vec.set_z(v.z);
		vec.set_w(v.w);
		return vec;
	}

	protobuf::Mat3 GlmSerializationUtils::convert(const glm::mat3& v)
	{
		protobuf::Mat3 mat;
		*mat.mutable_col1() = convert(v[0]);
		*mat.mutable_col2() = convert(v[1]);
		*mat.mutable_col3() = convert(v[2]);
		return mat;
	}

	protobuf::Mat4 GlmSerializationUtils::convert(const glm::mat4& v)
	{
		protobuf::Mat4 mat;
		*mat.mutable_col1() = convert(v[0]);
		*mat.mutable_col2() = convert(v[1]);
		*mat.mutable_col3() = convert(v[2]);
		*mat.mutable_col4() = convert(v[3]);
		return mat;
	}

	protobuf::Color3 GlmSerializationUtils::convert(const Color3& v)
	{
		protobuf::Color3 color;
		color.set_r(v.r);
		color.set_g(v.g);
		color.set_b(v.b);
		return color;
	}

	protobuf::Color GlmSerializationUtils::convert(const Color& v)
	{
		protobuf::Color color;
		color.set_r(v.r);
		color.set_g(v.g);
		color.set_b(v.b);
		color.set_a(v.a);
		return color;
	}

	glm::uvec2 GlmSerializationUtils::convert(const protobuf::Uvec2& v)
	{
		return { v.x(), v.y() };
	}

	glm::vec2 GlmSerializationUtils::convert(const protobuf::Vec2& v)
	{
		return { v.x(), v.y() };
	}

	glm::vec3 GlmSerializationUtils::convert(const protobuf::Vec3& v)
	{
		return { v.x(), v.y(), v.z() };
	}

	glm::vec4 GlmSerializationUtils::convert(const protobuf::Vec4& v)
	{
		return { v.x(), v.y(), v.z(), v.w() };
	}

	glm::mat3 GlmSerializationUtils::convert(const protobuf::Mat3& v)
	{
		return {
			convert(v.col1()),
			convert(v.col2()),
			convert(v.col3())
		};
	}

	glm::mat4 GlmSerializationUtils::convert(const protobuf::Mat4& v)
	{
		return {
			convert(v.col1()),
			convert(v.col2()),
			convert(v.col3()),
			convert(v.col4())
		};
	}

	Color GlmSerializationUtils::convert(const protobuf::Color& v)
	{
		return { v.r(), v.g(), v.b(), v.a() };
	}

	Color3 GlmSerializationUtils::convert(const protobuf::Color3& v)
	{
		return { v.r(), v.g(), v.b() };
	}
}