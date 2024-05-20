#pragma once

#include <memory>
#include <sol/sol.hpp>

namespace darmok
{
    class Material;
    class LuaTexture;

	class LuaMaterial final
	{
	public:
		LuaMaterial() noexcept;
		LuaMaterial(const std::shared_ptr<Material>& material) noexcept;
		LuaMaterial(const LuaTexture& texture) noexcept;
		const std::shared_ptr<Material>& getReal() const noexcept;

		uint8_t getShininess() const noexcept;
		LuaMaterial& setShininess(uint8_t v) noexcept;

		float getSpecularStrength() const noexcept;
		LuaMaterial& setSpecularStrength(float v) noexcept;

		static void bind(sol::state_view& lua) noexcept;

	private:
		std::shared_ptr<Material> _material;
	};

}