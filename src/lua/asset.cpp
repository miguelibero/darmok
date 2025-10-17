#include "lua/asset.hpp"
#include "lua/scene.hpp"
#include "lua/mesh.hpp"
#include "lua/program.hpp"
#include "lua/texture.hpp"
#include "lua/material.hpp"
#include "lua/skeleton.hpp"
#include "lua/asset_pack.hpp"
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
	std::shared_ptr<Program> LuaAssetContext::loadProgram(IAssetContext& assets, const std::string& path)
	{
		return assets.getProgramLoader()(path).value();
	}

	std::shared_ptr<Texture> LuaAssetContext::loadTexture(IAssetContext& assets, const std::string& path)
	{
		return assets.getTextureLoader()(path).value();
	}

	std::shared_ptr<TextureAtlas> LuaAssetContext::loadTextureAtlas(IAssetContext& assets, const std::string& path)
	{
		return assets.getTextureAtlasLoader()(path).value();
	}

	std::shared_ptr<MeshDefinition> LuaAssetContext::loadMeshDefinition(IAssetContext& assets, const std::string& path)
	{
		return assets.getMeshLoader().loadDefinition(path).value();
	}

	std::shared_ptr<Sound> LuaAssetContext::loadSound(IAssetContext& assets, const std::string& path)
	{
		return assets.getSoundLoader()(path).value();
	}

	std::shared_ptr<Music> LuaAssetContext::loadMusic(IAssetContext& assets, const std::string& path)
	{
		return assets.getMusicLoader()(path).value();
	}

	std::shared_ptr<protobuf::Scene> LuaAssetContext::loadSceneDefinition(IAssetContext& assets, const std::string& path)
	{
		return assets.getSceneDefinitionLoader()(path).value();
	}

	std::shared_ptr<Skeleton> LuaAssetContext::loadSkeleton(IAssetContext& assets, const std::string& path)
	{
		return assets.getSkeletonLoader()(path).value();
	}

	std::shared_ptr<SkeletalAnimation> LuaAssetContext::loadSkeletalAnimation(IAssetContext& assets, const std::string& path)
	{
		return assets.getSkeletalAnimationLoader()(path).value();
	}

	std::shared_ptr<SkeletalAnimatorDefinition> LuaAssetContext::loadSkeletalAnimatorDefinition(IAssetContext& assets, const std::string& path)
	{
		return assets.getSkeletalAnimatorDefinitionLoader()(path).value();
	}

	sol::table LuaAssetContext::loadSkeletalAnimations(IAssetContext& assets, const SkeletalAnimatorDefinition& def, sol::this_state ts)
	{
		sol::table t = sol::state_view{ ts }.create_table();
		auto map = ConstSkeletalAnimatorDefinitionWrapper{ def }.loadAnimations(assets.getSkeletalAnimationLoader());
		for (auto& [name, anim] : map)
		{
			t[name] = anim;
		}
		return t;
	}

	void LuaAssetContext::bind(sol::state_view& lua) noexcept
	{
		LuaProgram::bind(lua);
		LuaTexture::bind(lua);
		LuaTextureAtlas::bind(lua);
		LuaMaterial::bind(lua);
		LuaMesh::bind(lua);
		LuaSkeleton::bind(lua);

		lua.new_usertype<IAssetContext>("IAssetContext", sol::no_constructor,
			"load_program", &LuaAssetContext::loadProgram,
			"load_texture", &LuaAssetContext::loadTexture,
			"load_texture_atlas", &LuaAssetContext::loadTextureAtlas,
			"load_mesh_definition", &LuaAssetContext::loadMeshDefinition,
			"load_sound", &LuaAssetContext::loadSound,
			"load_music", &LuaAssetContext::loadMusic,
			"load_model", &LuaAssetContext::loadSceneDefinition,
			"load_scene_definition", &LuaAssetContext::loadSceneDefinition,
			"load_skeleton", &LuaAssetContext::loadSkeleton,
			"load_skeletal_animation", &LuaAssetContext::loadSkeletalAnimation,
			"load_skeletal_animator_definition", &LuaAssetContext::loadSkeletalAnimatorDefinition,
			"load_skeletal_animations", &LuaAssetContext::loadSkeletalAnimations
		);

		LuaAssetPack::bind(lua);

		lua.new_usertype<AssetContext>(
			"AssetContext", sol::no_constructor,
			sol::base_classes, sol::bases<IAssetContext>()
		);
	}
}