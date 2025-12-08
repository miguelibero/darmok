#include "lua/shape.hpp"
#include "lua/glm.hpp"
#include "lua/protobuf.hpp"
#include "lua/scene_serialize.hpp"
#include <darmok/shape.hpp>
#include <darmok/glm_serialize.hpp>

namespace darmok
{
	void LuaShape::bindRay(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Ray>("Ray",
			sol::factories(
				[]() { return Ray{}; },
				[](const VarLuaTable<glm::vec3>& origin, const VarLuaTable<glm::vec3>& dir) {
					return Ray{ LuaGlm::tableGet(origin), LuaGlm::tableGet(dir) };
				}
			),
			"from_positions", [](const VarLuaTable<glm::vec3>& vsrc, const VarLuaTable<glm::vec3>& vdst) {
				auto src = LuaGlm::tableGet(vsrc);
				auto dst = LuaGlm::tableGet(vdst);
				return Ray{ src, dst - src };
			},
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
			"reverse", &Ray::reverse,
			"to_line", &Ray::toLine,
			sol::meta_function::to_string, &Ray::toString
		);
	}

	void LuaShape::bindLine(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Line>("Line",
			sol::factories(
				[]() { return Line{}; },
				[](const Line::Points& points) {
					return Line{ points }; },
					[](const VarLuaTable<glm::vec3>& p1, const VarLuaTable<glm::vec3>& p2) {
					return Line{ LuaGlm::tableGet(p1), LuaGlm::tableGet(p2) };
				}
			),
			"points", &Line::points,
			"to_ray", &Line::toRay,
			sol::meta_function::multiplication, &Line::operator*,
			sol::meta_function::to_string, &Line::toString
		);
	}

	void LuaShape::bindBoundingBox(sol::state_view& lua) noexcept
	{
		auto def = lua.new_usertype<BoundingBox::Definition>("BoundingBoxDefinition",
			sol::factories([]() {
				return BoundingBox::createDefinition();
			}),
			"get_entity_component", [](LuaEntityDefinition& entity)
			{
				return entity.getComponent<BoundingBox::Definition>();
			},
			sol::meta_function::to_string, [](const BoundingBox::Definition& def)
			{
				return BoundingBox{ def }.toString();
			}
		);
		LuaProtobufBinding{ std::move(def) }
			.convertProtobufProperty<glm::vec3, protobuf::Vec3>("min")
			.convertProtobufProperty<glm::vec3, protobuf::Vec3>("max");


		lua.new_usertype<BoundingBox>("BoundingBox",
			sol::factories(
				[]() { return BoundingBox(); },
				[](const Cube& cube) { return BoundingBox{ cube }; },
				[](const Sphere& sphere) { return BoundingBox{ sphere }; },
				[](const Capsule& capsule) { return BoundingBox{ capsule }; },
				[](const Polygon& poly) { return BoundingBox{ poly }; },
				[](const Triangle& tri) { return BoundingBox{ tri }; },
				[](const Frustum& frust) { return BoundingBox{ frust }; },
				[](const BoundingBox::Definition& def) { return BoundingBox{ def }; },
				[](const VarLuaTable<glm::vec3>& min, const VarLuaTable<glm::vec3>& max) {
					return BoundingBox{ LuaGlm::tableGet(min), LuaGlm::tableGet(max) };
				}
			),
			"expand", sol::overload(
				[](BoundingBox& bb, const VarLuaTable<glm::vec3>& v) {
					return bb.expand(LuaGlm::tableGet(v));
				},
				[](BoundingBox& bb, float v) {
					return bb.expand(glm::vec3(v));
				}
			),
			"contract", sol::overload(
				[](BoundingBox& bb, const VarLuaTable<glm::vec3>& v) {
					return bb.contract(LuaGlm::tableGet(v));
				},
				[](BoundingBox& bb, float v) {
					return bb.contract(glm::vec3(v));
				}
			),
			"snap", &BoundingBox::snap,
			"expand_to_position", sol::overload(
				[](BoundingBox& bb, const VarLuaTable<glm::vec3>& v) {
					return bb.expandToPosition(LuaGlm::tableGet(v));
				}
			),
			"min", &BoundingBox::min,
			"max", &BoundingBox::max,
			"size", sol::property(&BoundingBox::size),
			"corners", sol::property(&BoundingBox::getCorners),
			"ortho", sol::property(&BoundingBox::getOrtho),
			sol::meta_function::to_string, &BoundingBox::toString
		);
	}

	void LuaShape::bindFrustum(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Frustum>("Frustum",
			sol::factories(
				[]() { return Frustum(); },
				[](const VarLuaTable<glm::mat4>& proj) {
					return Frustum(LuaGlm::tableGet(proj));
				}
			),
			sol::meta_function::to_string, &Frustum::toString,
			sol::meta_function::multiplication, &Frustum::operator*,
			"center", sol::property(&Frustum::getCenter),
			"bounding_box", sol::property(&Frustum::getBoundingBox),
			"slopes", sol::property(&Frustum::getSlopes),
			"aligned_proj_matrix", sol::property(&Frustum::getAlignedProjectionMatrix),
			"get_slice", &Frustum::getSlice
		);
	}

	void LuaShape::bindGrid(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Grid>("Grid",
			sol::factories(
				[]() { return Grid(); },
				[](const VarLuaTable<glm::vec2>& sep) {
					return Grid(LuaGlm::tableGet(sep));
				},
				[](const VarLuaTable<glm::vec2>& sep, const VarLuaTable<glm::uvec2>& amount) {
					return Grid(LuaGlm::tableGet(sep), LuaGlm::tableGet(amount));
				},
				[](const VarLuaTable<glm::vec2>& sep, const VarLuaTable<glm::uvec2>& amount, const VarLuaTable<glm::vec3>& normal) {
					return Grid(LuaGlm::tableGet(sep), LuaGlm::tableGet(amount), LuaGlm::tableGet(normal));
				},
				[](const VarLuaTable<glm::vec2>& sep, const VarLuaTable<glm::uvec2>& amount, const VarLuaTable<glm::vec3>& normal, const VarLuaTable<glm::vec3>& origin) {
					return Grid(LuaGlm::tableGet(sep), LuaGlm::tableGet(amount), LuaGlm::tableGet(normal), LuaGlm::tableGet(origin));
				}
			),
			sol::meta_function::to_string, &Grid::toString,
			"along", sol::property(&Grid::getAlong),
			"origin", &Grid::origin,
			"normal", &Grid::normal,
			"separation", &Grid::separation,
			"amount", &Grid::amount
		);
	}
}