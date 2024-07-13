#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string>
#include <unordered_set>
#include <darmok/export.h>
#include <darmok/program_fwd.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/collection.hpp>
#include <nlohmann/json.hpp>

namespace bgfx
{
	struct EmbeddedShader;
}

namespace darmok
{
	class DataView;

	class DARMOK_EXPORT Program final
	{
	public:
		Program(const std::string& name, const bgfx::EmbeddedShader* embeddedShaders, const DataView& vertexLayout);
		Program(const bgfx::ProgramHandle& handle, const bgfx::VertexLayout& layout) noexcept;
		~Program() noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;

		[[nodiscard]] const bgfx::ProgramHandle& getHandle() const noexcept;
		[[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept;
	private:
		bgfx::ProgramHandle _handle;
		bgfx::VertexLayout _layout;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IProgramLoader
	{
	public:
		using result_type = std::shared_ptr<Program>;

		virtual ~IProgramLoader() = default;
		virtual [[nodiscard]] result_type operator()(std::string_view name) = 0;
	};

	class IDataLoader;
	class IVertexLayoutLoader;

	class DARMOK_EXPORT DataProgramLoader final : public IProgramLoader
	{
	public:
		struct Suffixes final
		{
			std::string vertex;
			std::string fragment;
			std::string vertexLayout;
		};

		static const Suffixes& getDefaultSuffixes() noexcept;

		DataProgramLoader(IDataLoader& dataLoader, IVertexLayoutLoader& vertexLayoutLoader, Suffixes suffixes = getDefaultSuffixes()) noexcept;
		[[nodiscard]] std::shared_ptr<Program> operator()(std::string_view name) override;
		bgfx::VertexLayout loadVertexLayout(std::string_view name);
	private:
		IDataLoader& _dataLoader;
		IVertexLayoutLoader& _vertexLayoutLoader;
		Suffixes _suffixes;

		bgfx::ShaderHandle loadShader(const std::string& filePath);
	};

	class ShaderImporterImpl;

	class DARMOK_EXPORT ShaderImporter final : public IAssetTypeImporter
	{
	public:
		ShaderImporter();
		~ShaderImporter() noexcept;
		ShaderImporter& setShadercPath(const std::filesystem::path& path) noexcept;
		ShaderImporter& addIncludePath(const std::filesystem::path& path) noexcept;
		void setLogOutput(OptionalRef<std::ostream> log) noexcept override;
		std::vector<std::filesystem::path> getOutputs(const Input& input) override;
		std::vector<std::filesystem::path> getDependencies(const Input& input) override;

		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
		const std::string& getName() const noexcept override;
	private:
		std::unique_ptr<ShaderImporterImpl> _impl;
	};

	using ProgramDefines = std::unordered_set<std::string>;

	class DARMOK_EXPORT ProgramGroup final
	{
	public:
		using Defines = ProgramDefines;
		std::shared_ptr<Program> getProgram(const Defines& defines) const noexcept;
		void setProgram(const std::shared_ptr<Program>& prog, const Defines& defines) noexcept;
		void read(const nlohmann::json& json, IProgramLoader& prog) noexcept;
	private:
		std::unordered_map<Defines, std::shared_ptr<Program>> _programs;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IProgramGroupLoader
	{
	public:
		using result_type = std::shared_ptr<ProgramGroup>;

		virtual ~IProgramGroupLoader() = default;
		virtual [[nodiscard]] result_type operator()(std::string_view name) = 0;
	};

	class DARMOK_EXPORT JsonProgramGroupLoader final : public IProgramGroupLoader
	{
	public:
		JsonProgramGroupLoader(IDataLoader& dataLoader, IProgramLoader& programLoader, const std::string& manifestSuffix = ".manifest.json") noexcept;
		[[nodiscard]] std::shared_ptr<ProgramGroup> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
		IProgramLoader& _progLoader;
		std::string _manifestSuffix;
	};
}