#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/program_fwd.hpp>
#include <sol/sol.hpp>

namespace bgfx
{
	struct VertexLayout;
}

namespace darmok
{
	struct Model;
	struct AssimpModelLoadConfig;
	class AssetContext;
	class Program;
	class Texture;
	class TextureAtlas;
	class Skeleton;
	class SkeletalAnimation;
	struct SkeletalAnimatorConfig;

	using SkeletalAnimationMap = std::unordered_map<std::string, std::shared_ptr<SkeletalAnimation>>;

	class LuaAssets final
	{
	public:
		LuaAssets(AssetContext& assets) noexcept;

		AssetContext& getReal() noexcept;
		const AssetContext& getReal() const noexcept;

		std::shared_ptr<Program> loadProgram(const std::string& name);
		std::shared_ptr<Program> loadStandardProgram(StandardProgramType type);
		std::shared_ptr<Texture> loadTexture1(const std::string& name);
		std::shared_ptr<Texture> loadTexture2(const std::string& name, uint64_t flags);
		std::shared_ptr<Texture> loadColorTexture(const Color& color);
		std::shared_ptr<TextureAtlas> loadTextureAtlas1(const std::string& name);
		std::shared_ptr<TextureAtlas> loadTextureAtlas2(const std::string& name, uint64_t textureFlags);
		std::shared_ptr<Model> loadModel1(const std::string& name);
		std::shared_ptr<Model> loadModel2(const std::string& name, const AssimpModelLoadConfig& config);
		std::shared_ptr<Skeleton> loadSkeleton(const std::string& name);
		std::shared_ptr<SkeletalAnimation> loadSkeletalAnimation(const std::string& name);
		SkeletalAnimatorConfig loadSkeletalAnimatorConfig(const std::string& name);
		SkeletalAnimationMap loadSkeletalAnimations(const SkeletalAnimatorConfig& config);

		static void bind(sol::state_view& lua) noexcept;

	private:
		OptionalRef<AssetContext> _assets;
	};
}