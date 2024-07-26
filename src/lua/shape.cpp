#include "shape.hpp"
#include "glm.hpp"
#include <darmok/color.hpp>
#include <darmok/shape.hpp>

namespace darmok
{
    void LuaShape::bind(sol::state_view& lua) noexcept
    {
		lua.new_usertype<Rectangle>("Rectangle",
			sol::factories(
				[]() { return Rectangle(); },
				[](const VarLuaVecTable<glm::vec2>& size) {
					return Rectangle(LuaGlm::tableGet(size)); },
				[](const VarLuaVecTable<glm::vec2>& size, const VarLuaTable<glm::vec2>& origin) {
					return Rectangle(LuaGlm::tableGet(size), LuaGlm::tableGet(origin)); }
			),
			"size", &Rectangle::size,
			"origin", &Rectangle::origin,
			"to_lines", &Rectangle::toLines,
			sol::meta_function::to_string, &Rectangle::toString
		);

		lua.new_usertype<Cube>("Cube",
			sol::factories(
				[]() { return Cube(); },
				[](const VarLuaVecTable<glm::vec3>& size) {
					return Cube(LuaGlm::tableGet(size)); },
				[](const VarLuaVecTable<glm::vec3>& size, const VarLuaTable<glm::vec3>& origin) {
					return Cube(LuaGlm::tableGet(size), LuaGlm::tableGet(origin)); }
			),
			"size", &Cube::size,
			"origin", &Cube::origin,
			sol::meta_function::to_string, &Cube::toString
		);

		lua.new_usertype<Sphere>("Sphere",
			sol::factories(
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
			"origin", &Sphere::origin,
			sol::meta_function::to_string, &Sphere::toString
		);

		lua.new_usertype<Triangle>("Triangle",
			sol::factories(
				[](const Triangle::Vertices& vertices) {
					return Triangle(vertices); },
				[](const VarLuaTable<glm::vec3>& p1, const VarLuaTable<glm::vec3>& p2, const VarLuaTable<glm::vec3>& p3) {
					return Triangle(LuaGlm::tableGet(p1), LuaGlm::tableGet(p2), LuaGlm::tableGet(p3));
				}
			),
			"vertices", &Triangle::vertices,
			sol::meta_function::to_string, &Triangle::toString
		);

		lua.new_usertype<Polygon>("Polygon",
			sol::factories(
				[]() {
					return Polygon(); },
				[](const Polygon::Triangles& triangles) {
					return Polygon(triangles); },
				[](const sol::table& table) {
					Polygon poly;
					size_t count = table.size();
					poly.triangles.resize(count);
					for (size_t i = 0; i < count; i++)
					{
						sol::table elm = table[i + 1];
						auto& tri = poly.triangles[i];
						LuaGlm::tableInit(tri.vertices[0], elm[1]);
						LuaGlm::tableInit(tri.vertices[1], elm[2]);
						LuaGlm::tableInit(tri.vertices[2], elm[3]);
					}
					return poly;
				}
			),
			"triangles", &Polygon::triangles,
			"origin", &Polygon::origin,
			sol::meta_function::to_string, &Polygon::toString
		);

		lua.new_usertype<Plane>("Plane",
			sol::factories(
				[]() { return Plane(); },
				[](const VarLuaTable<glm::vec3>& normal) {
					return Plane(LuaGlm::tableGet(normal)); },
				[](const VarLuaTable<glm::vec3>& normal, float constant) {
					return Plane(LuaGlm::tableGet(normal), constant); }
			),
			"normal", &Plane::normal,
			"constant", &Plane::constant,
			"origin", sol::property(&Plane::getOrigin),
			sol::meta_function::multiplication, &Plane::operator*,
			sol::meta_function::to_string, &Plane::toString
		);

		lua.new_usertype<Capsule>("Capsule",
			sol::factories(
				[]() { return Capsule(); },
				[](float cylinderHeight) {
					return Capsule(cylinderHeight); },
					[](float cylinderHeight, float radius) {
					return Capsule(cylinderHeight, radius); }
			),
			"cylinder_height", &Capsule::cylinderHeight,
			"radius", &Capsule::radius,
			"origin", &Capsule::origin,
			sol::meta_function::to_string, &Capsule::toString
		);

		lua.new_usertype<Ray>("Ray",
			sol::factories(
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
			"to_line", &Ray::toLine,
			sol::meta_function::to_string, &Ray::toString
		);

		lua.new_usertype<Line>("Line",
			sol::factories(
				[]() { return Line(); },
				[](const Line::Points& points) {
					return Line(points); },
				[](const VarLuaTable<glm::vec3>& p1, const VarLuaTable<glm::vec3>& p2) {
					return Line(LuaGlm::tableGet(p1), LuaGlm::tableGet(p2));
				}
			),
			"points", &Line::points,
			sol::meta_function::multiplication, &Line::operator*,
			sol::meta_function::to_string, &Line::toString
		);

		lua.new_usertype<BoundingBox>("BoundingBox",
			sol::factories(
				[]() { return BoundingBox(); },
				[](const VarLuaTable<glm::vec3>& min, const VarLuaTable<glm::vec3>& max) {
					return BoundingBox(LuaGlm::tableGet(min), LuaGlm::tableGet(max));
				}
			),
			"min", &BoundingBox::min,
			"max", &BoundingBox::max,
			"cube", sol::property(&BoundingBox::getCube),
			sol::meta_function::to_string, &BoundingBox::toString
		);
    }
}