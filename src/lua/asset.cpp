#include "asset.hpp"
#include "scene.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "skeleton.hpp"
#include <darmok/shape.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/scene_assimp.hpp>
#include <darmok/audio.hpp>
#include <darmok/scene_serialize.hpp>

namespace darmok
{	
	std::shared_ptr<Program> LuaAssets::loadProgram(AssetContext& assets, const std::string& path)
	{
		return assets.getProgramLoader()(path).value();
	}

	std::shared_ptr<Texture> LuaAssets::loadTexture(AssetContext& assets, const std::string& path)
	{
		return assets.getTextureLoader()(path).value();
	}

	std::shared_ptr<TextureAtlas> LuaAssets::loadTextureAtlas(AssetContext& assets, const std::string& path)
	{
		return assets.getTextureAtlasLoader()(path).value();
	}

	std::shared_ptr<Sound> LuaAssets::loadSound(AssetContext& assets, const std::string& path)
	{
		return assets.getSoundLoader()(path).value();
	}

	std::shared_ptr<Music> LuaAssets::loadMusic(AssetContext& assets, const std::string& path)
	{
		return assets.getMusicLoader()(path).value();
	}

	void LuaAssets::loadScene(AssetContext& assets, Scene& scene, const std::string& path)
	{
		auto result = assets.getSceneLoader()(scene, path);
		if (!result)
		{
			throw std::runtime_error{ "Failed to load scene: " + result.error() };
		}
	}

	std::shared_ptr<Skeleton> LuaAssets::loadSkeleton(AssetContext& assets, const std::string& path)
	{
		return assets.getSkeletonLoader()(path).value();
	}

	std::shared_ptr<SkeletalAnimation> LuaAssets::loadSkeletalAnimation(AssetContext& assets, const std::string& path)
	{
		return assets.getSkeletalAnimationLoader()(path).value();
	}

	std::shared_ptr<SkeletalAnimatorDefinition> LuaAssets::loadSkeletalAnimatorDefinition(AssetContext& assets, const std::string& path)
	{
		return assets.getSkeletalAnimatorDefinitionLoader()(path).value();
	}

	SkeletalAnimationMap LuaAssets::loadSkeletalAnimations(AssetContext& assets, const SkeletalAnimatorDefinition& def)
	{
		return SkeletalAnimatorUtils::loadAnimations(def, assets.getSkeletalAnimationLoader());
	}

	void LuaAssets::bind(sol::state_view& lua) noexcept
	{
		LuaProgram::bind(lua);
		LuaTexture::bind(lua);
		LuaTextureAtlas::bind(lua);
		LuaMaterial::bind(lua);
		LuaMesh::bind(lua);
		LuaSkeleton::bind(lua);

		lua.new_usertype<AssetContext>("Assets", sol::no_constructor,
			"load_scene", &LuaAssets::loadScene,
			"load_program", &LuaAssets::loadProgram,
			"load_texture", &LuaAssets::loadTexture,
			"load_texture_atlas", &LuaAssets::loadTextureAtlas,
			"load_sound", &LuaAssets::loadSound,
			"load_music", &LuaAssets::loadMusic,
			"load_skeleton", &LuaAssets::loadSkeleton,
			"load_skeletal_animation", &LuaAssets::loadSkeletalAnimation,
			"load_skeletal_animator_definition", &LuaAssets::loadSkeletalAnimatorDefinition
			//"load_skeletal_animations", &LuaAssets::loadSkeletalAnimations
		);
	}
}