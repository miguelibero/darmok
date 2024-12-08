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
	std::shared_ptr<Program> LuaAssets::loadProgram(AssetContext& assets, const std::filesystem::path& path)
	{
		return assets.getProgramLoader()(path);
	}

	std::shared_ptr<Texture> LuaAssets::loadTexture(AssetContext& assets, const std::filesystem::path& path)
	{
		return assets.getTextureLoader()(path);
	}

	std::shared_ptr<TextureAtlas> LuaAssets::loadTextureAtlas(AssetContext& assets, const std::filesystem::path& path)
	{
		return assets.getTextureAtlasLoader()(path);
	}

	std::shared_ptr<Sound> LuaAssets::loadSound(AssetContext& assets, const std::filesystem::path& path)
	{
		return assets.getSoundLoader()(path);
	}

	std::shared_ptr<Music> LuaAssets::loadMusic(AssetContext& assets, const std::filesystem::path& path)
	{
		return assets.getMusicLoader()(path);
	}

	std::shared_ptr<Model> LuaAssets::loadModel1(AssetContext& assets, const std::filesystem::path& path)
	{
		return assets.getModelLoader()(path);
	}

	std::shared_ptr<Model> LuaAssets::loadModel2(AssetContext& assets, const std::filesystem::path& path, const AssimpModelLoadConfig& config)
	{
		auto& loader = assets.getAssimpModelLoader();
		loader.setConfig(config);
		return loader(path);
	}

	std::shared_ptr<Skeleton> LuaAssets::loadSkeleton(AssetContext& assets, const std::filesystem::path& path)
	{
		return assets.getSkeletonLoader()(path);
	}

	std::shared_ptr<SkeletalAnimation> LuaAssets::loadSkeletalAnimation(AssetContext& assets, const std::filesystem::path& path)
	{
		return assets.getSkeletalAnimationLoader()(path);
	}

	std::shared_ptr<SkeletalAnimatorConfig> LuaAssets::loadSkeletalAnimatorConfig(AssetContext& assets, const std::filesystem::path& path)
	{
		return assets.getSkeletalAnimatorConfigLoader()(path);
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
			"load_texture", &LuaAssets::loadTexture,
			"load_texture_atlas", &LuaAssets::loadTextureAtlas,
			"load_sound", &LuaAssets::loadSound,
			"load_music", &LuaAssets::loadMusic,
			"load_skeleton", &LuaAssets::loadSkeleton,
			"load_skeletal_animation", &LuaAssets::loadSkeletalAnimation,
			"load_skeletal_animator_config", &LuaAssets::loadSkeletalAnimatorConfig,
			"load_skeletal_animations", &LuaAssets::loadSkeletalAnimations
		);
	}
}