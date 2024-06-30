#pragma once

#include <memory>
#include <string>
#include <variant>
#include "glm.hpp"
#include <darmok/texture_fwd.hpp>
#include <darmok/anim.hpp>
#include <sol/sol.hpp>

namespace darmok
{
    class LuaImage;
	struct TextureConfig;
	class Texture;

	class LuaTexture final
	{
	public:
		using Config = TextureConfig;
		LuaTexture(const std::shared_ptr<Texture>& texture) noexcept;
		LuaTexture(const LuaImage& img, uint64_t flags = defaultTextureLoadFlags) noexcept;
		LuaTexture(const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		LuaTexture(const VarLuaTable<glm::uvec2>& size, uint64_t flags = defaultTextureLoadFlags) noexcept;
		LuaTexture(const VarLuaTable<glm::uvec2>& size, bgfx::TextureFormat::Enum format, uint64_t flags = defaultTextureLoadFlags) noexcept;
		std::shared_ptr<Texture> getReal() const noexcept;
		TextureType getType() const noexcept;
		LuaTexture& setName(const std::string& name) noexcept;
		const glm::uvec2& getSize() noexcept;
		bgfx::TextureFormat::Enum getFormat() const noexcept;
		uint16_t getLayerCount() const noexcept;
		uint16_t getDepth() const noexcept;
		bool hasMips() const noexcept;
		std::string to_string() const noexcept;

		static void bind(sol::state_view& lua) noexcept;

	private:
		std::shared_ptr<Texture> _texture;
	};	

	struct TextureAtlas;
	struct TextureAtlasMeshConfig;
	class LuaMesh;

	class LuaTextureAtlas final
	{
	public:
		LuaTextureAtlas(const std::shared_ptr<TextureAtlas>& atlas) noexcept;
		std::shared_ptr<TextureAtlas> getReal() const noexcept;
		LuaTexture getTexture() const noexcept;

		using MeshConfig = TextureAtlasMeshConfig;

		LuaMesh createSprite1(const std::string& name, const bgfx::VertexLayout& layout) const noexcept;
		LuaMesh createSprite2(const std::string& name, const bgfx::VertexLayout& layout, const MeshConfig& config) const noexcept;
		std::vector<AnimationFrame> createAnimation1(const bgfx::VertexLayout& layout) const noexcept;
		std::vector<AnimationFrame> createAnimation2(const bgfx::VertexLayout& layout, const std::string& namePrefix) const noexcept;
		std::vector<AnimationFrame> createAnimation3(const bgfx::VertexLayout& layout, const std::string& namePrefix, float frameDuration) const noexcept;
		std::vector<AnimationFrame> createAnimation4(const bgfx::VertexLayout& layout, const std::string& namePrefix, float frameDuration, const MeshConfig& config) const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<TextureAtlas> _atlas;
	};
}