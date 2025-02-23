#pragma once

#include <darmok/export.h>
#include <darmok/varying.hpp>
#include <darmok/data.hpp>
#include <darmok/collection.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/loader.hpp>
#include <darmok/expected.hpp>
#include <darmok/protobuf/program.pb.h>

#include <unordered_set>
#include <unordered_map>
#include <string>
#include <filesystem>

#include <nlohmann/json.hpp>

namespace darmok
{
	class DataView;

	using ProgramDefines = std::unordered_set<std::string>;

	using ProgramSource = protobuf::ProgramSource;
	using ProgramDefinition = protobuf::Program;
	using ProgramDefinitionProfile = protobuf::ProgramProfile;

	namespace ProgramCoreUtils
	{
		using RendererProfileMap = std::unordered_map<bgfx::RendererType::Enum, std::vector<std::string>>;

		const RendererProfileMap& getRendererProfiles() noexcept;
		const ProgramDefinitionProfile& getCurrentProfile(const ProgramDefinition& def);
	};

	struct ProgramCompilerConfig final
	{
		size_t bufferSize = 4096;
		std::filesystem::path shadercPath;
		using IncludePaths = std::unordered_set<std::filesystem::path>;
		IncludePaths includePaths;
		OptionalRef<std::ostream> log;

		void read(const nlohmann::json& json, std::filesystem::path basePath);
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