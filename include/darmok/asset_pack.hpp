#pragma once

#include <darmok/export.h>
#include <darmok/protobuf/asset.pb.h>
#include <darmok/protobuf.hpp>
#include <darmok/program.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{	
	template<class Interface>
	class DARMOK_EXPORT AssetPackLoader final : public Interface
	{
	private:
		const protobuf::AssetPack& _assets;
	public:
		using Result = Interface::Result;
		using Resource = Interface::Resource;

		AssetPackLoader(const protobuf::AssetPack& assets)
			: _assets{ assets }
		{
		}

		[[nodiscard]] Result operator()(std::filesystem::path path) override
		{
			auto typeId = protobuf::getTypeId<Resource>();
			auto itr = _assets.groups().find(typeId);
			if(itr == _assets.groups().end())
			{
				return unexpected{ "type not found" };
			}
			auto itr2 = itr->second.assets().find(path.string());	
			if(itr2 == itr->second.assets().end())
			{
				return unexpected{ "path not found" };
			}
			auto def = std::make_shared<Resource>();
			if(!itr2->second.UnpackTo(def.get()))
			{
				return unexpected{ "failed to unpack" };
			}
			return def;
		}
	};

	struct DARMOK_EXPORT AssetPackFallbacks final
	{
		OptionalRef<IProgramLoader> program;
		OptionalRef<ITextureLoader> texture;
		OptionalRef<IMeshLoader> mesh;
		OptionalRef<IMaterialLoader> material;
		OptionalRef<IArmatureLoader> armature;
	};

	struct DARMOK_EXPORT AssetPackConfig final
	{
		AssetPackFallbacks fallbacks = {};
		ProgramCompilerConfig programCompilerConfig;
		OptionalRef<bx::AllocatorI> alloc;
	};

	class DARMOK_EXPORT AssetPack final
	{
	public:
		using Definition = protobuf::AssetPack;
		AssetPack(const Definition& def, const AssetPackConfig& config);

		template<class T>
		[[nodiscard]] expected<std::shared_ptr<T>, std::string> load(const std::filesystem::path& path) noexcept;

	private:
		const Definition& _def;
		bx::DefaultAllocator _defaultAlloc;

		AssetPackLoader<IProgramDefinitionLoader> _progDefLoader;
		AssetPackLoader<ITextureDefinitionLoader> _texDefLoader;
		AssetPackLoader<IMeshDefinitionLoader> _meshDefLoader;
		AssetPackLoader<IMaterialDefinitionLoader> _matDefLoader;
		AssetPackLoader<IArmatureDefinitionLoader> _armDefLoader;

		AssetPackLoader<IProgramSourceLoader> _progSrcLoader;
		ProgramDefinitionFromSourceLoader _progDefFromSrcLoader;
		AssetPackLoader<ITextureSourceLoader> _texSrcLoader;
		TextureDefinitionFromSourceLoader _texDefFromSrcLoader;
		AssetPackLoader<IMeshSourceLoader> _meshSrcLoader;
		MeshDefinitionFromSourceLoader _meshDefFromSrcLoader;

		MultiLoader<IProgramDefinitionLoader> _multiProgramDefLoader;
		MultiLoader<ITextureDefinitionLoader> _multiTextureDefLoader;
		MultiLoader<IMeshDefinitionLoader> _multiMeshDefLoader;

		MultiLoader<ILoader<Program>> _multiProgramLoader;
		MultiLoader<ILoader<Texture>> _multiTextureLoader;
		MultiLoader<ILoader<IMesh>> _multiMeshLoader;
		MultiLoader<ILoader<Material>> _multiMaterialLoader;
		MultiLoader<ILoader<Armature>> _multiArmatureLoader;

		ProgramLoader _programLoader;
		TextureLoader _textureLoader;
		MeshLoader _meshLoader;
		MaterialLoader _materialLoader;
		ArmatureLoader _armatureLoader;
	};
}