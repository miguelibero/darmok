#pragma once

#include <memory>
#include <string>
#include <variant>
#include <glm/glm.hpp>
#include <darmok/texture.hpp>
#include <darmok/anim.hpp>
#include "sol.hpp"
#include "math.hpp"

namespace darmok
{
    class LuaImage;

	class LuaTexture final
	{
	public:
		LuaTexture(const std::shared_ptr<Texture>& texture) noexcept;
		LuaTexture(const LuaImage& img, uint64_t flags = defaultTextureLoadFlags) noexcept;
		const std::shared_ptr<Texture>& getReal() const noexcept;
		LuaImage getImage() const noexcept;
		TextureType getType() const noexcept;
		LuaTexture& setName(const std::string& name) noexcept;

		static void configure(sol::state_view& lua) noexcept;

	private:
		std::shared_ptr<Texture> _texture;
	};	

	class LuaRenderTexture final
	{
	public:
		using Config = RenderTextureConfig;
		LuaRenderTexture(const std::shared_ptr<RenderTexture>& texture) noexcept;
		LuaRenderTexture(const Config& cfg, uint64_t flags = defaultTextureLoadFlags) noexcept;
		LuaRenderTexture(const VarUvec2& size, uint64_t flags = defaultTextureLoadFlags) noexcept;
		const std::shared_ptr<RenderTexture>& getReal() const noexcept;
		TextureType getType() const noexcept;
		LuaRenderTexture& setName(const std::string& name) noexcept;

		static void configure(sol::state_view& lua) noexcept;

	private:
		std::shared_ptr<RenderTexture> _texture;
	};

	class TextureAtlas;
	struct MeshCreationConfig;

	class LuaTextureAtlas final
	{
	public:
		LuaTextureAtlas(const std::shared_ptr<TextureAtlas>& atlas) noexcept;
		const std::shared_ptr<TextureAtlas>& getReal() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<TextureAtlas> _atlas;
	};

	struct TextureAtlasMeshCreator;
	struct TextureAtlasMeshCreationConfig;
    class LuaMesh;

	struct LuaTextureAtlasMeshCreator final
	{
		using Config = TextureAtlasMeshCreationConfig;

		LuaTextureAtlasMeshCreator(const bgfx::VertexLayout& layout, const LuaTextureAtlas& atlas) noexcept;
		~LuaTextureAtlasMeshCreator();

		Config& getConfig() noexcept;
		void setConfig(const Config& config) noexcept;
		bgfx::VertexLayout& getVertexLayout()  noexcept;
		const LuaTextureAtlas& getTextureAtlas() noexcept;

		LuaMesh createSprite(const std::string& name) const noexcept;
		std::vector<AnimationFrame> createAnimation1(const std::string& namePrefix) const noexcept;
		std::vector<AnimationFrame> createAnimation2(const std::string& namePrefix, float frameDuration) const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		LuaTextureAtlas _atlas;
		std::shared_ptr<TextureAtlasMeshCreator> _creator;
	};
}