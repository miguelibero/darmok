#pragma once

#include <memory>
#include <glm/glm.hpp>
#include "sol.hpp"

namespace darmok
{
    class Image;
	
	class LuaImage final
	{
	public:
		LuaImage(const std::shared_ptr<Image>& img) noexcept;
		const std::shared_ptr<Image>& getReal() const noexcept;
		bool empty() const noexcept;
		glm::uvec2 getSize() const noexcept;
		uint32_t getDepth() const noexcept;
		bool isCubeMap() const noexcept;
		uint8_t getMipCount() const noexcept;
		uint16_t getLayerCount() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Image> _img;
	};
}