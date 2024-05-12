#include "math.hpp"
#include <darmok/color.hpp>
#include <glm/gtx/rotate_normalized_axis.hpp>

namespace darmok
{
    void LuaMath::configure2(sol::state_view& lua) noexcept
    {
		auto uvec3 = configureUvec<glm::uvec3, glm::uvec3(unsigned int), glm::uvec3(unsigned int, unsigned int, unsigned int)>(lua, "Uvec3");
		uvec3["x"] = &glm::uvec3::x;
		uvec3["y"] = &glm::uvec3::y;
		uvec3["z"] = &glm::uvec3::z;

		auto uvec2 = configureUvec<glm::uvec2, glm::uvec2(unsigned int), glm::uvec2(unsigned int, unsigned int)>(lua, "Uvec2");
		uvec2["x"] = &glm::uvec2::x;
		uvec2["y"] = &glm::uvec2::y;

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
    }
}