#include "lua/shape.hpp"
#include "lua/glm.hpp"
#include "lua/protobuf.hpp"
#include <darmok/shape.hpp>

namespace darmok
{
	void LuaShape::bind(sol::state_view& lua) noexcept
	{
		bindRectangle(lua);
		bindCube(lua);
		bindSphere(lua);
		bindTriangle(lua);
		bindPolygon( lua);
		bindPlane(lua);
		bindCapsule(lua);
		bindRay(lua);
		bindLine(lua);
		bindBoundingBox(lua);
		bindFrustum(lua);
		bindGrid(lua);
	}

	void LuaShape::bindRectangle(sol::state_view& lua) noexcept
	{
		LuaUtils::newProtobuf<Rectangle::Definition>(lua, "RectangleDefinition")
			.protobufProperty<protobuf::Vec2>("size")
			.protobufProperty<protobuf::Vec2>("origin");

		lua.new_usertype<Rectangle>("Rectangle",
			sol::factories(
				[]() { return Rectangle{}; },
				[](const Rectangle::Definition& def) { return Rectangle{ def }; },
				[](const VarLuaVecTable<glm::vec2>& size) {
					return Rectangle{ LuaGlm::tableGet(size) }; },
				[](const VarLuaVecTable<glm::vec2>& size, const VarLuaTable<glm::vec2>& origin) {
					return Rectangle{ LuaGlm::tableGet(size), LuaGlm::tableGet(origin) }; }
			),
			"size", &Rectangle::size,
			"origin", &Rectangle::origin,
			"to_lines", &Rectangle::toLines,
			sol::meta_function::to_string, &Rectangle::toString
		);
	}

	void LuaShape::bindCube(sol::state_view& lua) noexcept
	{
		LuaUtils::newProtobuf<Cube::Definition>(lua, "CubeDefinition")
			.protobufProperty<protobuf::Vec3>("size")
			.protobufProperty<protobuf::Vec3>("origin");

		lua.new_usertype<Cube>("Cube",
			sol::factories(
				[]() { return Cube{}; },
				[](const BoundingBox& bbox) { return Cube{ bbox }; },
				[](const Cube::Definition& def) { return Cube{ def }; },
				[](const VarLuaVecTable<glm::vec3>& size) {
					return Cube{ LuaGlm::tableGet(size) }; },
				[](const VarLuaVecTable<glm::vec3>& size, const VarLuaTable<glm::vec3>& origin) {
					return Cube{ LuaGlm::tableGet(size), LuaGlm::tableGet(origin) }; }
			),
			"size", &Cube::size,
			"origin", &Cube::origin,
			sol::meta_function::to_string, &Cube::toString
		);
	}

	void LuaShape::bindSphere(sol::state_view& lua) noexcept
	{
		LuaUtils::newProtobuf<Sphere::Definition>(lua, "SphereDefinition")
			.protobufProperty<protobuf::Vec3>("origin");

		lua.new_usertype<Sphere>("Sphere",
			sol::factories(
				[]() { return Sphere(); },
				[](const VarLuaTable<glm::vec3>& origin) {
					return Sphere{ LuaGlm::tableGet(origin) }; },
				[](const Sphere::Definition& def) {
					return Sphere{ def }; },
				[](float radius) {
					return Sphere{ radius }; },
				[](float radius, const VarLuaTable<glm::vec3>& origin) {
					return Sphere{ radius, LuaGlm::tableGet(origin) }; },
				[](const VarLuaTable<glm::vec3>& origin, float radius) {
					return Sphere{ LuaGlm::tableGet(origin), radius }; }
			),
			"radius", &Sphere::radius,
			"origin", &Sphere::origin,
			sol::meta_function::to_string, &Sphere::toString
		);
	}

	void LuaShape::bindTriangle(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Triangle>("Triangle",
			sol::factories(
				[](const Triangle::Vertices& vertices) {
					return Triangle{ vertices }; },
				[](const Triangle::Definition& def) {
					return Triangle{ def }; },
				[](const VarLuaTable<glm::vec3>& p1, const VarLuaTable<glm::vec3>& p2, const VarLuaTable<glm::vec3>& p3) {
					return Triangle{ LuaGlm::tableGet(p1), LuaGlm::tableGet(p2), LuaGlm::tableGet(p3) };
				}
			),
			"vertices", &Triangle::vertices,
			sol::meta_function::to_string, &Triangle::toString
		);
	}

	void LuaShape::bindPolygon(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Polygon>("Polygon",
			sol::factories(
				[]() {
					return Polygon{}; },
				[](const Polygon::Triangles& triangles) {
					return Polygon{ triangles }; },
				[](const Polygon::Definition& def) {
					return Polygon{ def }; },
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
	}

	void LuaShape::bindPlane(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Plane>("Plane",
			sol::factories(
				[]() { return Plane{}; },
				[](const Plane::Definition& def) { return Plane{def}; },
				[](const VarLuaTable<glm::vec3>& normal) {
					return Plane{ LuaGlm::tableGet(normal) }; },
				[](const VarLuaTable<glm::vec3>& normal, float distance) {
					return Plane{ LuaGlm::tableGet(normal), distance }; }
			),
			"normal", &Plane::normal,
			"distance", &Plane::distance,
			"signed_distance_to", &Plane::signedDistanceTo,
			"origin", sol::property(&Plane::getOrigin),
			"along", sol::property(&Plane::getAlong),
			sol::meta_function::multiplication, &Plane::operator*,
			sol::meta_function::to_string, &Plane::toString
		);
	}

	void LuaShape::bindCapsule(sol::state_view& lua) noexcept
	{
		LuaUtils::newProtobuf<Capsule::Definition>(lua, "CapsuleDefinition")
			.protobufProperty<protobuf::Vec3>("origin");

		lua.new_usertype<Capsule>("Capsule",
			sol::factories(
				[]() { return Capsule{}; },
				[](const Capsule::Definition& def) { return Capsule{def}; },
				[](float cylinderHeight) {
					return Capsule{ cylinderHeight }; },
				[](float cylinderHeight, float radius) {
					return Capsule{ cylinderHeight, radius }; }
			),
			"cylinder_height", &Capsule::cylinderHeight,
			"radius", &Capsule::radius,
			"origin", &Capsule::origin,
			sol::meta_function::to_string, &Capsule::toString
		);
	}

	
}