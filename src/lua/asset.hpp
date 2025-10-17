#pragma once

#include "lua/lua.hpp"

#include <string>
#include <memory>
#include <filesystem>

namespace darmok
{
	class IAssetContext;
	class Program;
	class Texture;
	class TextureAtlas;
	class Skeleton;
	class SkeletalAnimation;
	class Sound;
	class Music;

	namespace protobuf
	{
		class Scene;
		class SkeletalAnimator;
		class Mesh;
	}
	using SkeletalAnimatorDefinition = protobuf::SkeletalAnimator;
	using MeshDefinition = protobuf::Mesh;

	class LuaAssetContext final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static std::shared_ptr<Program> loadProgram(IAssetContext& assets, const std::string& path);
		static std::shared_ptr<Texture> loadTexture(IAssetContext& assets, const std::string& path);
		static std::shared_ptr<TextureAtlas> loadTextureAtlas(IAssetContext& assets, const std::string& path);
		static std::shared_ptr<MeshDefinition> loadMeshDefinition(IAssetContext& assets, const std::string& path);

		static std::shared_ptr<Sound> loadSound(IAssetContext& assets, const std::string& path);
		static std::shared_ptr<Music> loadMusic(IAssetContext& assets, const std::string& path);

		static std::shared_ptr<protobuf::Scene> loadSceneDefinition(IAssetContext& assets, const std::string& path);

		static std::shared_ptr<Skeleton> loadSkeleton(IAssetContext& assets, const std::string& pathe);
		static std::shared_ptr<SkeletalAnimation> loadSkeletalAnimation(IAssetContext& assets, const std::string& path);
		static std::shared_ptr<SkeletalAnimatorDefinition> loadSkeletalAnimatorDefinition(IAssetContext& assets, const std::string& path);
		static sol::table loadSkeletalAnimations(IAssetContext& assets, const SkeletalAnimatorDefinition& def, sol::this_state ts);
	};
}