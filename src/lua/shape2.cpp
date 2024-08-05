#include "shape.hpp"
#include "glm.hpp"
#include <darmok/shape.hpp>

namespace darmok
{
    void LuaShape::bindRay(sol::state_view& lua) noexcept
	{
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
	}

	void LuaShape::bindLine(sol::state_view& lua) noexcept
	{
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
	}

	void LuaShape::bindBoundingBox(sol::state_view& lua) noexcept
	{
		lua.new_usertype<BoundingBox>("BoundingBox",
			sol::factories(
				[]() { return BoundingBox(); },
				[](const VarLuaTable<glm::vec3>& min, const VarLuaTable<glm::vec3>& max) {
					return BoundingBox(LuaGlm::tableGet(min), LuaGlm::tableGet(max));
				}
			),
			"expand", sol::overload(
				[](const BoundingBox& bb, const VarLuaTable<glm::vec3>& v) {
					return bb.expand(LuaGlm::tableGet(v));
				},
				[](const BoundingBox& bb, float v) {
					return bb.expand(glm::vec3(v));
				}
			),
			"contract", sol::overload(
				[](const BoundingBox& bb, const VarLuaTable<glm::vec3>& v) {
					return bb.contract(LuaGlm::tableGet(v));
				},
				[](const BoundingBox& bb, float v) {
					return bb.contract(glm::vec3(v));
				}
			),
			"min", &BoundingBox::min,
			"max", &BoundingBox::max,
			"cube", sol::property(&BoundingBox::getCube),
			sol::meta_function::to_string, &BoundingBox::toString
		);
    }
}