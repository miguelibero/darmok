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

#include <nlohmann/json.hpp>
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

	struct ProgramSource final
	{
		std::string name;
		Data vertexShader;
		Data fragmentShader;
		VaryingDefinition varying;

		void read(const nlohmann::ordered_json& json, std::filesystem::path path = "");

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(name),
				CEREAL_NVP(vertexShader),
				CEREAL_NVP(fragmentShader),
				CEREAL_NVP(varying)
			);
		}
	};

	struct ProgramCompilerConfig final
	{
		size_t bufferSize = 4096;
		std::filesystem::path shadercPath;
		using IncludePaths = std::unordered_set<std::filesystem::path>;
		IncludePaths includePaths;
		OptionalRef<std::ostream> log;

		void read(const nlohmann::json& json, std::filesystem::path basePath);

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(bufferSize),
				CEREAL_NVP(shadercPath),
				CEREAL_NVP(includePaths)
			);
		}
	};

	class DARMOK_EXPORT ProgramCompiler final
	{
	public:
		using Config = ProgramCompilerConfig;
		ProgramCompiler(const Config& config) noexcept;
		ProgramDefinition operator()(const ProgramSource& src);
	private:
		Config _config;
	};

	class ProgramFileImporterImpl;

	class DARMOK_EXPORT ProgramFileImporter final : public IFileTypeImporter
	{
	public:
		ProgramFileImporter();
		~ProgramFileImporter() noexcept;
		ProgramFileImporter& setShadercPath(const std::filesystem::path& path) noexcept;
		ProgramFileImporter& addIncludePath(const std::filesystem::path& path) noexcept;

		void setLogOutput(OptionalRef<std::ostream> log) noexcept override;
		bool startImport(const Input& input, bool dry = false) override;
		Outputs getOutputs(const Input& input) override;
		Dependencies getDependencies(const Input& input) override;
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
		void endImport(const Input& input) override;

		const std::string& getName() const noexcept override;
	private:
		std::unique_ptr<ProgramFileImporterImpl> _impl;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IProgramDefinitionLoader : public ILoader<ProgramDefinition>
	{
	};

	using CerealProgramDefinitionLoader = CerealLoader<IProgramDefinitionLoader>;
}