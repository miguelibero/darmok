#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/expected.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/program.pb.h>

#include <filesystem>
#include <unordered_set>

namespace slang
{
	struct ISession;
}

namespace darmok
{
	struct DARMOK_EXPORT SlangProgramCompilerConfig final
	{
		using IncludePaths = std::unordered_set<std::filesystem::path>;
		IncludePaths includePaths;
		OptionalRef<std::ostream> log;

		struct ReadConfig final
		{
			std::filesystem::path rootPath;
			std::filesystem::path basePath;
		};

		void read(const nlohmann::json& json, const ReadConfig& config) noexcept;
	};

	class SlangProgramCompilerImpl;

    class DARMOK_EXPORT SlangProgramCompiler final
	{
	public:
		using Config = SlangProgramCompilerConfig;
		using Source = protobuf::SlangProgramSource;
		SlangProgramCompiler(const Config& config) noexcept;
		~SlangProgramCompiler() noexcept;
		expected<protobuf::Program, std::string> operator()(const Source& src) noexcept;

	private:
		std::unique_ptr<SlangProgramCompilerImpl> _impl;
		Config _config;
	};

	class SlangProgramFileImporterImpl;

	class DARMOK_EXPORT SlangProgramFileImporter final : public IFileTypeImporter
	{
	public:
		SlangProgramFileImporter() noexcept;
		~SlangProgramFileImporter() noexcept;
		SlangProgramFileImporter& addIncludePath(const std::filesystem::path& path) noexcept;

		const std::string& getName() const noexcept override;
		expected<void, std::string> init(OptionalRef<std::ostream> log = nullptr) noexcept override;
		expected<Effect, std::string> prepare(const Input& input) noexcept override;
		expected<void, std::string> operator()(const Input& input, Config& config) noexcept override;
	private:
		std::unique_ptr<SlangProgramFileImporterImpl> _impl;
	};
}