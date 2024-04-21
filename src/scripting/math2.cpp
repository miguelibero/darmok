#include "math.hpp"
#include <darmok/color.hpp>
#include <darmok/math.hpp>
#include <tuple>
#include <glm/gtx/rotate_normalized_axis.hpp>

namespace darmok
{
	static glm::quat quatEuler(float x, float y, float z) noexcept
	{
		return glm::quat(glm::vec3(x, y, z));
	}

    void LuaMath::configure2(sol::state_view& lua) noexcept
    {
		auto ivec4 = configureUvec<glm::ivec4, glm::ivec4(int), glm::ivec4(int, int, int, int)>(lua, "Ivec3");
		ivec4["x"] = &glm::ivec4::x;
		ivec4["y"] = &glm::ivec4::y;
		ivec4["z"] = &glm::ivec4::z;
		ivec4["w"] = &glm::ivec4::w;

		auto ivec3 = configureUvec<glm::ivec3, glm::ivec3(int), glm::ivec3(int, int, int)>(lua, "Ivec3");
		ivec3["x"] = &glm::ivec3::x;
		ivec3["y"] = &glm::ivec3::y;
		ivec3["z"] = &glm::ivec3::z;

		auto ivec2 = configureUvec<glm::ivec2, glm::ivec2(int), glm::ivec2(int, int)>(lua, "Ivec2");
		ivec2["x"] = &glm::ivec2::x;
		ivec2["y"] = &glm::ivec2::y;

		auto color = configureUvec<Color, Color(uint8_t, uint8_t, uint8_t, uint8_t)>(lua, "Color");
		color["r"] = &Color::r;
		color["g"] = &Color::g;
		color["b"] = &Color::b;
		color["a"] = &Color::a;

		auto color3 = configureUvec<Color3, Color3(uint8_t, uint8_t, uint8_t)>(lua, "Color3");
		color3["r"] = &Color3::r;
		color3["g"] = &Color3::g;
		color3["b"] = &Color3::b;

		lua.new_usertype<glm::quat>("Quat",
			sol::constructors<glm::quat(float, float, float, float), glm::quat(const glm::vec3&)>(),
			sol::meta_function::equal_to, sol::resolve<bool(const glm::quat&, const glm::quat&)>(glm::operator==),
			sol::meta_function::addition, sol::resolve<glm::quat(const glm::quat&, const glm::quat&)>(glm::operator+),
			sol::meta_function::subtraction, sol::resolve<glm::quat(const glm::quat&, const glm::quat&)>(glm::operator-),
			sol::meta_function::unary_minus, sol::resolve<glm::quat(const glm::quat&)>(glm::operator-),
			sol::meta_function::multiplication, sol::overload(sol::resolve<glm::quat(const glm::quat&, const glm::quat&)>(glm::operator*), sol::resolve<glm::quat(const glm::quat&, const float&)>(glm::operator*)),
			sol::meta_function::division, sol::resolve<glm::quat(const glm::quat&, const float&)>(glm::operator/),
			sol::meta_function::to_string, sol::resolve<std::string(const glm::quat&)>(glm::to_string),
			"x", &glm::quat::x,
			"y", &glm::quat::y,
			"z", &glm::quat::z,
			"w", &glm::quat::w,
			"zero", sol::var(glm::quat()),
			"euler", &quatEuler,
			"slerp", sol::resolve<glm::quat(const glm::quat&, const glm::quat&, float)>(&glm::slerp),
			"euler_angles", sol::resolve<glm::vec3(const glm::quat&)>(&glm::eulerAngles),
			"angle_axis", sol::resolve<glm::quat(const float&, const glm::vec3&)>(&glm::angleAxis),
			"rotate_axis", sol::resolve<glm::quat(const glm::quat&, const float&, const glm::vec3&)>(&glm::rotateNormalizedAxis)
		);

		lua.create_named_table("Colors",
			"black", &Colors::black,
			"white", &Colors::white,
			"red", &Colors::red,
			"green", &Colors::green,
			"blue", &Colors::blue,
			"yellow", &Colors::yellow,
			"cyan", &Colors::cyan,
			"magenta", &Colors::magenta
		);

		lua.new_usertype<Quad>("Quad",
			sol::constructors<Quad(const glm::vec2&, const glm::vec2&), Quad(const glm::vec2&), Quad()>(),
			"size", &Quad::size,
			"origin", &Quad::origin,
			"to_lines", &Quad::toLines
		);

		lua.new_usertype<Cube>("Cube",
			sol::constructors<Cube(const glm::vec3&, const glm::vec3&), Cube(const glm::vec3&), Cube()>(),
			"size", &Cube::size,
			"origin", &Cube::origin
		);

		lua.new_usertype<Triangle>("Triangle",
			sol::constructors<Triangle(const Triangle::Vertices&)>(),
			"vertices", &Triangle::vertices
		);

		lua.new_usertype<Plane>("Plane",
			sol::constructors<Plane(const glm::vec3&, const glm::vec3&), Plane(const glm::vec3&), Plane()>(),
			"normal", &Plane::normal,
			"origin", &Plane::origin
		);

		lua.new_usertype<Ray>("Ray",
			sol::constructors<Ray(const glm::vec3&, const glm::vec3&), Ray(const glm::vec3&), Ray()>(),
			"direction", &Ray::direction,
			"origin", &Ray::origin,
			sol::meta_function::multiplication, &Ray::operator*,
			"unproject", &Ray::unproject,
			"intersect", sol::overload(
				sol::resolve<std::optional<float>(const Plane&) const>(&Ray::intersect),
				sol::resolve<std::optional<float>(const Sphere&) const>(&Ray::intersect)
			),
			"intersect_normal", &Ray::intersectNormal,
			"to_line", &Ray::toLine
		);

		lua.new_usertype<Line>("Line",
			sol::constructors<Line(const Line::Points&)>(),
			"points", &Line::points,
			sol::meta_function::multiplication, &Line::operator*
		);
    }
}