#pragma once

#include <darmok/export.h>
#include <darmok/varying.hpp>
#include <darmok/data.hpp>
#include <darmok/collection.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/loader.hpp>

#include <unordered_set>
#include <unordered_map>
#include <string>

#include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace darmok
{
	class DataView;

	using ProgramDefines = std::unordered_set<std::string>;

	struct DARMOK_EXPORT ProgramProfileDefinition final
	{
		using Defines = ProgramDefines;
		using Map = std::unordered_map<Defines, Data>;
		Map vertexShaders;
		Map fragmentShaders;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(vertexShaders),
				CEREAL_NVP(fragmentShaders)
			);
		}
	};
	
	struct DARMOK_EXPORT ProgramDefinition final
	{
		using Profile = ProgramProfileDefinition;

		std::string name;
		std::unordered_map<std::string, Profile> profiles;
		VertexLayout vertexLayout;

		const Profile& getCurrentProfile() const;

		bool empty() const noexcept;

		template<typename T>
		void loadStaticMem(const T& mem)
		{
			loadBinary(DataView(mem, sizeof(mem)));
		}

		void loadBinary(DataView data);

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(name),
				CEREAL_NVP(profiles),
				CEREAL_NVP(vertexLayout)
			);
		}

		using RendererProfileMap = std::unordered_map<bgfx::RendererType::Enum, std::vector<std::string>>;
		static const RendererProfileMap& getRendererProfiles() noexcept;
	};

	class ProgramImporterImpl;

	class DARMOK_EXPORT ProgramImporter final : public IAssetTypeImporter
	{
	public:
		ProgramImporter();
		~ProgramImporter() noexcept;
		ProgramImporter& setShadercPath(const std::filesystem::path& path) noexcept;
		ProgramImporter& addIncludePath(const std::filesystem::path& path) noexcept;

		void setLogOutput(OptionalRef<std::ostream> log) noexcept override;
		bool startImport(const Input& input, bool dry = false) override;
		Outputs getOutputs(const Input& input) override;
		Dependencies getDependencies(const Input& input) override;
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
		void endImport(const Input& input) override;

		const std::string& getName() const noexcept override;
	private:
		std::unique_ptr<ProgramImporterImpl> _impl;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IProgramDefinitionLoader : public ILoader<ProgramDefinition>
	{
	};

	using CerealProgramDefinitionLoader = CerealLoader<IProgramDefinitionLoader>;
}