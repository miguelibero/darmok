#pragma once

#include "lua/lua.hpp"

#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>

#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem>

namespace darmok
{
	class AssetContext;
	struct Model;
	struct AssimpModelImportConfig;
	class Program;
	class Texture;
	class TextureAtlas;
	class Skeleton;
	class SkeletalAnimation;
	class Scene;
	
	class Sound;
	class Music;

	namespace protobuf
	{
		class Scene;
		class SkeletalAnimator;
	}
	using SkeletalAnimatorDefinition = protobuf::SkeletalAnimator;

	class LuaAssets final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static std::shared_ptr<Program> loadProgram(AssetContext& assets, const std::string& path);

		static std::shared_ptr<Texture> loadTexture(AssetContext& assets, const std::string& path);
		static std::shared_ptr<TextureAtlas> loadTextureAtlas(AssetContext& assets, const std::string& path);

		static std::shared_ptr<Sound> loadSound(AssetContext& assets, const std::string& path);
		static std::shared_ptr<Music> loadMusic(AssetContext& assets, const std::string& path);

		static std::shared_ptr<protobuf::Scene> loadSceneDefinition(AssetContext& assets, const std::string& path);
		static void loadScene(AssetContext& assets, Scene& scene, const std::string& path);

		static std::shared_ptr<Skeleton> loadSkeleton(AssetContext& assets, const std::string& pathe);
		static std::shared_ptr<SkeletalAnimation> loadSkeletalAnimation(AssetContext& assets, const std::string& path);
		static std::shared_ptr<SkeletalAnimatorDefinition> loadSkeletalAnimatorDefinition(AssetContext& assets, const std::string& path);
		static sol::table loadSkeletalAnimations(AssetContext& assets, const SkeletalAnimatorDefinition& def, sol::state_view lua);
	};
}