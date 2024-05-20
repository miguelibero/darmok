#include "shape.hpp"
#include <darmok/color.hpp>
#include <darmok/shape.hpp>

namespace darmok
{
    void LuaShape::bind(sol::state_view& lua) noexcept
    {
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