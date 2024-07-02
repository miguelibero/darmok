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
#include <darmok/program_standard.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/model_assimp.hpp>

namespace darmok
{	
	LuaAssets::LuaAssets(AssetContext& assets) noexcept
		: _assets(assets)
	{
	}

	AssetContext& LuaAssets::getReal() noexcept
	{
		return _assets.value();
	}

	const AssetContext& LuaAssets::getReal() const noexcept
	{
		return _assets.value();
	}

	LuaProgram LuaAssets::loadProgram(const std::string& name)
	{
		return LuaProgram(_assets->getProgramLoader()(name));
	}

	LuaProgram LuaAssets::loadStandardProgram(StandardProgramType type)
	{
		return LuaProgram(_assets->getStandardProgramLoader()(type));
	}

	LuaTexture LuaAssets::loadColorTexture(const Color& color)
	{
		return LuaTexture(_assets->getColorTextureLoader()(color));
	}

	LuaTextureAtlas LuaAssets::loadTextureAtlas1(const std::string& name)
	{
		return LuaTextureAtlas(_assets->getTextureAtlasLoader()(name));
	}

	LuaTextureAtlas LuaAssets::loadTextureAtlas2(const std::string& name, uint64_t textureFlags)
	{
		return LuaTextureAtlas(_assets->getTextureAtlasLoader()(name, textureFlags));
	}

	LuaTexture LuaAssets::loadTexture1(const std::string& name)
	{
		return LuaTexture(_assets->getTextureLoader()(name));
	}

	LuaTexture LuaAssets::loadTexture2(const std::string& name, uint64_t flags)
	{
		return LuaTexture(_assets->getTextureLoader()(name, flags));
	}

	std::shared_ptr<Model> LuaAssets::loadModel1(const std::string& name)
	{
		return _assets->getModelLoader()(name);
	}

	std::shared_ptr<Model> LuaAssets::loadModel2(const std::string& name, const AssimpModelLoadConfig& config)
	{
		auto& loader = _assets->getAssimpModelLoader();
		loader.setConfig(config);
		return loader(name);
	}

	std::shared_ptr<Skeleton> LuaAssets::loadSkeleton(const std::string& name)
	{
		return _assets->getSkeletonLoader()(name);
	}

	std::shared_ptr<SkeletalAnimation> LuaAssets::loadSkeletalAnimation(const std::string& name)
	{
		return _assets->getSkeletalAnimationLoader()(name);
	}

	SkeletalAnimatorConfig LuaAssets::loadSkeletalAnimatorConfig(const std::string& name)
	{
		return _assets->getSkeletalAnimatorConfigLoader()(name);
	}

	SkeletalAnimationMap LuaAssets::loadSkeletalAnimations(const SkeletalAnimatorConfig& config)
	{
		return config.loadAnimations(_assets->getSkeletalAnimationLoader());
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

		lua.new_usertype<LuaAssets>("Assets", sol::no_constructor,
			"load_model", sol::overload(
				&LuaAssets::loadModel1,
				&LuaAssets::loadModel2
			),
			"load_program", &LuaAssets::loadProgram,
			"load_standard_program", &LuaAssets::loadStandardProgram,
			"load_texture", sol::overload(&LuaAssets::loadTexture1, &LuaAssets::loadTexture2),
			"load_color_texture", &LuaAssets::loadColorTexture,
			"load_texture_atlas", sol::overload(&LuaAssets::loadTextureAtlas1, &LuaAssets::loadTextureAtlas2),
			"load_skeleton", &LuaAssets::loadSkeleton,
			"load_skeletal_animation", &LuaAssets::loadSkeletalAnimation,
			"load_skeletal_animator_config", &LuaAssets::loadSkeletalAnimatorConfig,
			"load_skeletal_animations", &LuaAssets::loadSkeletalAnimations
		);
	}
}