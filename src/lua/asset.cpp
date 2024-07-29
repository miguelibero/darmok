#include "asset.hpp"
#include "scene.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "model.hpp"
#include "skeleton.hpp"
#include <darmok/shape.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/model_assimp.hpp>
#include <darmok/audio.hpp>

namespace darmok
{	
	std::shared_ptr<Program> LuaAssets::loadProgram(AssetContext& assets, const std::string& name)
	{
		return assets.getProgramLoader()(name);
	}

	std::shared_ptr<Texture> LuaAssets::loadColorTexture(AssetContext& assets, const Color& color)
	{
		return assets.getColorTextureLoader()(color);
	}
	
	std::shared_ptr<Texture> LuaAssets::loadTexture1(AssetContext& assets, const std::string& name)
	{
		return assets.getTextureLoader()(name);
	}

	std::shared_ptr<Texture> LuaAssets::loadTexture2(AssetContext& assets, const std::string& name, uint64_t flags)
	{
		return assets.getTextureLoader()(name, flags);
	}

	std::shared_ptr<TextureAtlas> LuaAssets::loadTextureAtlas1(AssetContext& assets, const std::string& name)
	{
		return assets.getTextureAtlasLoader()(name);
	}

	std::shared_ptr<TextureAtlas> LuaAssets::loadTextureAtlas2(AssetContext& assets, const std::string& name, uint64_t textureFlags)
	{
		return assets.getTextureAtlasLoader()(name, textureFlags);
	}

	std::shared_ptr<Sound> LuaAssets::loadSound(AssetContext& assets, const std::string& name)
	{
		return assets.getSoundLoader()(name);
	}

	std::shared_ptr<Music> LuaAssets::loadMusic(AssetContext& assets, const std::string& name)
	{
		return assets.getMusicLoader()(name);
	}

	std::shared_ptr<Model> LuaAssets::loadModel1(AssetContext& assets, const std::string& name)
	{
		return assets.getModelLoader()(name);
	}

	std::shared_ptr<Model> LuaAssets::loadModel2(AssetContext& assets, const std::string& name, const AssimpModelLoadConfig& config)
	{
		auto& loader = assets.getAssimpModelLoader();
		loader.setConfig(config);
		return loader(name);
	}

	std::shared_ptr<Skeleton> LuaAssets::loadSkeleton(AssetContext& assets, const std::string& name)
	{
		return assets.getSkeletonLoader()(name);
	}

	std::shared_ptr<SkeletalAnimation> LuaAssets::loadSkeletalAnimation(AssetContext& assets, const std::string& name)
	{
		return assets.getSkeletalAnimationLoader()(name);
	}

	SkeletalAnimatorConfig LuaAssets::loadSkeletalAnimatorConfig(AssetContext& assets, const std::string& name)
	{
		return assets.getSkeletalAnimatorConfigLoader()(name);
	}

	SkeletalAnimationMap LuaAssets::loadSkeletalAnimations(AssetContext& assets, const SkeletalAnimatorConfig& config)
	{
		return config.loadAnimations(assets.getSkeletalAnimationLoader());
	}

	void LuaAssets::bind(sol::state_view& lua) noexcept
	{
		LuaProgram::bind(lua);
		LuaTexture::bind(lua);
		LuaTextureAtlas::bind(lua);
		LuaMaterial::bind(lua);
		LuaMesh::bind(lua);
		LuaModel::bind(lua);
		LuaSkeleton::bind(lua);

		lua.new_usertype<AssetContext>("Assets", sol::no_constructor,
			"load_model", sol::overload(
				&LuaAssets::loadModel1,
				&LuaAssets::loadModel2
			),
			"load_program", &LuaAssets::loadProgram,
			"load_texture", sol::overload(&LuaAssets::loadTexture1, &LuaAssets::loadTexture2),
			"load_color_texture", &LuaAssets::loadColorTexture,
			"load_texture_atlas", sol::overload(&LuaAssets::loadTextureAtlas1, &LuaAssets::loadTextureAtlas2),
			"load_sound", &LuaAssets::loadSound,
			"load_music", &LuaAssets::loadMusic,
			"load_skeleton", &LuaAssets::loadSkeleton,
			"load_skeletal_animation", &LuaAssets::loadSkeletalAnimation,
			"load_skeletal_animator_config", &LuaAssets::loadSkeletalAnimatorConfig,
			"load_skeletal_animations", &LuaAssets::loadSkeletalAnimations
		);
	}
}