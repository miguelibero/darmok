#include "shape.hpp"
#include "glm.hpp"
#include <darmok/color.hpp>
#include <darmok/shape.hpp>

namespace darmok
{
    void LuaShape::bind(sol::state_view& lua) noexcept
    {
		lua.new_usertype<Rectangle>("Rectangle", sol::no_constructor,
			"new", sol::overload(
				[]() { return Rectangle(); },
				[](const VarLuaTable<glm::vec2>& size) { 
					return Rectangle(LuaGlm::tableGet(size)); },
				[](const VarLuaTable<glm::vec2>& size, const VarLuaTable<glm::vec2>& origin) {
					return Rectangle(LuaGlm::tableGet(size), LuaGlm::tableGet(origin)); }
			),
			"size", &Rectangle::size,
			"origin", &Rectangle::origin,
			"to_lines", &Rectangle::toLines
		);

		lua.new_usertype<Cube>("Cube", sol::no_constructor,
			"new", sol::overload(
				[]() { return Cube(); },
				[](const VarLuaTable<glm::vec3>& size) {
					return Cube(LuaGlm::tableGet(size)); },
				[](const VarLuaTable<glm::vec3>& size, const VarLuaTable<glm::vec3>& origin) {
					return Cube(LuaGlm::tableGet(size), LuaGlm::tableGet(origin)); }
			),
			"size", &Cube::size,
			"origin", &Cube::origin
		);

		lua.new_usertype<Sphere>("Sphere", sol::no_constructor,
			"new", sol::overload(
				[]() { return Sphere(); },
				[](const VarLuaTable<glm::vec3>& origin) {
					return Sphere(LuaGlm::tableGet(origin)); },
				[](float radius) {
					return Sphere(radius); },
				[](float radius, const VarLuaTable<glm::vec3>& origin) {
					return Sphere(radius, LuaGlm::tableGet(origin)); },
				[](const VarLuaTable<glm::vec3>& origin, float radius) {
					return Sphere(LuaGlm::tableGet(origin), radius); }
			),
			"radius", &Sphere::radius,
			"origin", &Sphere::origin
		);

		lua.new_usertype<Triangle>("Triangle", sol::no_constructor,
			"new", sol::overload(
				[]() { return Line(); },
				[](const Triangle::Vertices& vertices) {
					return Triangle(vertices); },
				[](const VarLuaTable<glm::vec3>& p1, const VarLuaTable<glm::vec3>& p2, const VarLuaTable<glm::vec3>& p3) {
					return Triangle(LuaGlm::tableGet(p1), LuaGlm::tableGet(p2), LuaGlm::tableGet(p3));
				}
			),
			"vertices", &Triangle::vertices
		);

		lua.new_usertype<Plane>("Plane", sol::no_constructor,
			"new", sol::overload(
				[]() { return Plane(); },
				[](const VarLuaTable<glm::vec3>& normal) {
					return Plane(LuaGlm::tableGet(normal)); },
				[](const VarLuaTable<glm::vec3>& normal, const VarLuaTable<glm::vec3>& origin) {
					return Plane(LuaGlm::tableGet(normal), LuaGlm::tableGet(origin)); }
			),
			"normal", &Plane::normal,
			"origin", &Plane::origin,
			sol::meta_function::multiplication, &Plane::operator*
		);

		lua.new_usertype<Ray>("Ray", sol::no_constructor,
			"new", sol::overload(
				[]() { return Ray(); },
				[](const VarLuaTable<glm::vec3>& dir) {
					return Ray(LuaGlm::tableGet(dir)); },
				[](const VarLuaTable<glm::vec3>& dir, const VarLuaTable<glm::vec3>& origin) {
					return Ray(LuaGlm::tableGet(dir), LuaGlm::tableGet(origin)); }
			),
			"direction", &Ray::direction,
			"origin", &Ray::origin,
			sol::meta_function::multiplication, sol::overload(
				sol::resolve<glm::vec3(float) const>(&Ray::operator*),
				sol::resolve<Ray(const glm::mat4&) const >(&Ray::operator*)
			),
			"unproject", &Ray::unproject,
			"intersect", sol::overload(
				sol::resolve<std::optional<float>(const Plane&) const>(&Ray::intersect),
				sol::resolve<std::optional<float>(const Sphere&) const>(&Ray::intersect)
			),
			"intersect_normal", &Ray::intersectNormal,
			"to_line", &Ray::toLine
		);

		lua.new_usertype<Line>("Line", sol::no_constructor,
			"new", sol::overload(
				[]() { return Line(); },
				[](const Line::Points& points) {
					return Line(points); },
				[](const VarLuaTable<glm::vec3>& p1, const VarLuaTable<glm::vec3>& p2) {
					return Line(LuaGlm::tableGet(p1), LuaGlm::tableGet(p2));
				}
			),
			"points", &Line::points,
			sol::meta_function::multiplication, &Line::operator*
		);
    }
}