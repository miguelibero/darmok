#include "math.hpp"
#include <darmok/color.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace darmok
{
    void LuaMath::configure1(sol::state_view& lua) noexcept
    {
		auto mat3 = configureMat<glm::mat3, glm::mat3(glm::vec3, glm::vec3, glm::vec3)>(lua, "mat3");
		mat3["identity"] = sol::var(glm::mat3());

		auto mat4 = configureMat<glm::mat4, glm::mat4(glm::vec4, glm::vec4, glm::vec4, glm::vec4)>(lua, "mat4");
		mat4["identity"] = sol::var(glm::mat4());

		auto vec4 = configureVec<glm::vec4, glm::vec4(float), glm::vec4(float, float, float, float), glm::vec4(const glm::ivec4&), glm::vec4(const glm::uvec4&), glm::vec3(const glm::vec3&, float)>(lua, "vec4");
		vec4["x"] = &glm::vec4::x;
		vec4["y"] = &glm::vec4::y;
		vec4["z"] = &glm::vec4::z;
		vec4["w"] = &glm::vec4::w;
		vec4["rotate"] = sol::resolve<glm::vec4(const glm::vec4&, const float&, const glm::vec3&)>(glm::rotate);
		vec4["rotate_x"] = sol::resolve<glm::vec4(const glm::vec4&, const float&)>(glm::rotateX);
		vec4["rotate_y"] = sol::resolve<glm::vec4(const glm::vec4&, const float&)>(glm::rotateY);
		vec4["rotate_z"] = sol::resolve<glm::vec4(const glm::vec4&, const float&)>(glm::rotateZ);

		auto vec3 = configureVec<glm::vec3, glm::vec3(float), glm::vec3(float, float, float),
			glm::vec3(const glm::ivec3&), glm::vec3(const glm::uvec3&), glm::vec3(const glm::vec2&, float)>(lua, "vec3");
		vec3["x"] = &glm::vec3::x;
		vec3["y"] = &glm::vec3::y;
		vec3["z"] = &glm::vec3::z;
		vec3["left"] = sol::var(glm::vec3(-1, 0, 0));
		vec3["right"] = sol::var(glm::vec3(1, 0, 0));
		vec3["up"] = sol::var(glm::vec3(0, 1, 0));
		vec3["down"] = sol::var(glm::vec3(0, -1, 0));
		vec3["forward"] = sol::var(glm::vec3(0, 0, 1));
		vec3["backward"] = sol::var(glm::vec3(0, 0, -1));
		vec3["cross"] = sol::resolve<glm::vec3(const glm::vec3&, const glm::vec3&)>(glm::cross);
		vec3["project"] = sol::resolve<glm::vec3(const glm::vec3&, const glm::mat4&, const glm::mat4&, const glm::ivec4&)>(glm::project);
		vec3["unproject"] = sol::resolve<glm::vec3(const glm::vec3&, const glm::mat4&, const glm::mat4&, const glm::ivec4&)>(glm::unProject);
		vec3["rotate"] = sol::resolve<glm::vec3(const glm::vec3&, const float&, const glm::vec3&)>(glm::rotate);
		vec3["rotate_x"] = sol::resolve<glm::vec3(const glm::vec3&, const float&)>(glm::rotateX);
		vec3["rotate_y"] = sol::resolve<glm::vec3(const glm::vec3&, const float&)>(glm::rotateY);
		vec3["rotate_z"] = sol::resolve<glm::vec3(const glm::vec3&, const float&)>(glm::rotateZ);

		auto vec2 = configureVec<glm::vec2, glm::vec2(float), glm::vec2(float, float), glm::vec2(const glm::ivec2&), glm::vec2(const glm::uvec2&)>(lua, "vec2");
		vec2["x"] = &glm::vec2::x;
		vec2["y"] = &glm::vec2::y;

		auto uvec3 = configureUvec<glm::uvec3, glm::uvec3(unsigned int), glm::uvec3(unsigned int, unsigned int, unsigned int)>(lua, "uvec3");
		uvec3["x"] = &glm::uvec3::x;
		uvec3["y"] = &glm::uvec3::y;
		uvec3["z"] = &glm::uvec3::z;

		auto uvec2 = configureUvec<glm::uvec2, glm::uvec2(unsigned int), glm::uvec2(unsigned int, unsigned int)>(lua, "uvec2");
		uvec2["x"] = &glm::uvec2::x;
		uvec2["y"] = &glm::uvec2::y;

		lua.new_usertype<LuaMath>("Math",
			sol::constructors<>(),
			"clamp", sol::overload(&glm::clamp<float>, &glm::clamp<int>)
		);
    }
}