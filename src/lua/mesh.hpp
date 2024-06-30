#pragma once

#include <bgfx/bgfx.h>
#include <memory>
#include <vector>
#include <optional>
#include <darmok/glm.hpp>
#include <sol/sol.hpp>

namespace darmok
{
    class IMesh;

	class LuaMesh final
	{
	public:
		LuaMesh(const std::shared_ptr<IMesh>& mesh) noexcept;
		std::string to_string() const noexcept;
		std::shared_ptr<IMesh> getReal() const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<IMesh> _mesh;
	};

}