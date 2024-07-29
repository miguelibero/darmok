#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <sol/sol.hpp>

namespace darmok
{
	class AssetContext;
	struct Model;
	struct AssimpModelLoadConfig;
	class Program;
	class Texture;
	class TextureAtlas;
	class Skeleton;
	class SkeletalAnimation;
	struct SkeletalAnimatorConfig;
	class Sound;
	class Music;

	using SkeletalAnimationMap = std::unordered_map<std::string, std::shared_ptr<SkeletalAnimation>>;

	class LuaAssets final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static std::shared_ptr<Program> loadProgram(AssetContext& assets, const std::string& name);

		static std::shared_ptr<Texture> loadTexture1(AssetContext& assets, const std::string& name);
		static std::shared_ptr<Texture> loadTexture2(AssetContext& assets, const std::string& name, uint64_t flags);
		static std::shared_ptr<Texture> loadColorTexture(AssetContext& assets, const Color& color);
		static std::shared_ptr<TextureAtlas> loadTextureAtlas1(AssetContext& assets, const std::string& name);
		static std::shared_ptr<TextureAtlas> loadTextureAtlas2(AssetContext& assets, const std::string& name, uint64_t textureFlags);

		static std::shared_ptr<Sound> loadSound(AssetContext& assets, const std::string& name);
		static std::shared_ptr<Music> loadMusic(AssetContext& assets, const std::string& name);

		static std::shared_ptr<Model> loadModel1(AssetContext& assets, const std::string& name);
		static std::shared_ptr<Model> loadModel2(AssetContext& assets, const std::string& name, const AssimpModelLoadConfig& config);

		static std::shared_ptr<Skeleton> loadSkeleton(AssetContext& assets, const std::string& name);
		static std::shared_ptr<SkeletalAnimation> loadSkeletalAnimation(AssetContext& assets, const std::string& name);
		static SkeletalAnimatorConfig loadSkeletalAnimatorConfig(AssetContext& assets, const std::string& name);
		static SkeletalAnimationMap loadSkeletalAnimations(AssetContext& assets, const SkeletalAnimatorConfig& config);
	};
}