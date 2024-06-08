#pragma once

#include <string>
#include <memory>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/program_fwd.hpp>
#include <sol/sol.hpp>

namespace darmok
{
	class Model;
	class AssetContext;
	class LuaProgram;
	class LuaTexture;
	class LuaTextureAtlas;
	class LuaSkeleton;
	class LuaSkeletalAnimation;

	class LuaAssets final
	{
	public:
		LuaAssets(AssetContext& assets) noexcept;

		AssetContext& getReal() noexcept;
		const AssetContext& getReal() const noexcept;

		LuaProgram loadProgram(const std::string& name);
		LuaProgram loadStandardProgram(StandardProgramType type);
		LuaTexture loadTexture1(const std::string& name);
		LuaTexture loadTexture2(const std::string& name, uint64_t flags);
		LuaTexture loadColorTexture(const Color& color);
		LuaTextureAtlas loadTextureAtlas1(const std::string& name);
		LuaTextureAtlas loadTextureAtlas2(const std::string& name, uint64_t textureFlags);
		std::shared_ptr<Model> loadModel(const std::string& name);
		LuaSkeleton loadSkeleton(const std::string& name);
		LuaSkeletalAnimation loadSkeletalAnimation(const std::string& name);

		static void bind(sol::state_view& lua) noexcept;

	private:
		OptionalRef<AssetContext> _assets;
	};
}