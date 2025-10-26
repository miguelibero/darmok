#pragma once

#include <darmok/export.h>
#include <darmok/varying.hpp>
#include <darmok/data.hpp>
#include <darmok/collection.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/loader.hpp>
#include <darmok/expected.hpp>
#include <darmok/protobuf.hpp>
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

	class DARMOK_EXPORT ConstProgramDefinitionWrapper
	{
	public:
		using Definition = protobuf::Program;
		using Profile = protobuf::ProgramProfile;
		ConstProgramDefinitionWrapper(const Definition& def);

		expected<std::reference_wrapper<const Profile>, std::string> getCurrentProfile();

		using RendererProfileMap = std::unordered_map<bgfx::RendererType::Enum, std::vector<std::string>>;
		static const RendererProfileMap& getRendererProfiles() noexcept;
	private:
		const Definition& _def;
	};

	class DARMOK_EXPORT ProgramDefinitionWrapper final : public ConstProgramDefinitionWrapper
	{
	public:
		ProgramDefinitionWrapper(Definition& def);
	private:
		const Definition& _def;
	};

	class DARMOK_EXPORT ProgramSourceWrapper final
	{
	public:
		using Source = protobuf::ProgramSource;
		ProgramSourceWrapper(Source& src);

		expected<void, std::string> read(const std::filesystem::path& path);
		expected<void, std::string> read(const nlohmann::ordered_json& json, const std::filesystem::path& basePath);
	private:
		Source& _src;
	};

	struct DARMOK_EXPORT ProgramCompilerConfig final
	{
		size_t bufferSize = 4096;
		std::filesystem::path shadercPath;
		using IncludePaths = std::unordered_set<std::filesystem::path>;
		IncludePaths includePaths;
		OptionalRef<std::ostream> log;

		struct ReadConfig final
		{
			std::filesystem::path rootPath;
			std::filesystem::path basePath;
		};

		void read(const nlohmann::json& json, const ReadConfig& config);
	};

	class DARMOK_EXPORT ProgramCompiler final
	{
	public:
		using Config = ProgramCompilerConfig;
		ProgramCompiler(const Config& config) noexcept;
		expected<protobuf::Program, std::string> operator()(const protobuf::ProgramSource& src);

	private:
		Config _config;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IProgramDefinitionLoader : public ILoader<protobuf::Program>{};
	class DARMOK_EXPORT BX_NO_VTABLE IProgramSourceLoader : public ILoader<protobuf::ProgramSource>{};
	class DARMOK_EXPORT BX_NO_VTABLE IProgramDefinitionFromSourceLoader : public IFromDefinitionLoader<IProgramDefinitionLoader, protobuf::ProgramSource>{};

	using DataProgramSourceLoader = DataProtobufLoader<IProgramSourceLoader>;
	using DataProgramDefinitionLoader = DataProtobufLoader<IProgramDefinitionLoader>;

	class DARMOK_EXPORT ProgramSourceLoader final : public IProgramSourceLoader
	{
	public:
		ProgramSourceLoader(IDataLoader& dataLoader) noexcept;
		Result operator()(std::filesystem::path path) noexcept override;
	private:
		OptionalRef<IDataLoader> _dataLoader;
	};

	class DARMOK_EXPORT ProgramDefinitionFromSourceLoader final : public FromDefinitionLoader<IProgramDefinitionFromSourceLoader, IProgramSourceLoader>
	{
	public:
		ProgramDefinitionFromSourceLoader(IProgramSourceLoader& srcLoader, const ProgramCompilerConfig& compilerConfig) noexcept;
	private:
		Result create(std::shared_ptr<protobuf::ProgramSource> src) noexcept override;
		ProgramCompiler _compiler;
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
		expected<Effect, std::string> prepare(const Input& input) noexcept override;
		expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;

		const std::string& getName() const noexcept override;
	private:
		std::unique_ptr<ProgramFileImporterImpl> _impl;
	};
}