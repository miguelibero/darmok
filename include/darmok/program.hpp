#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string>
#include <unordered_set>
#include <darmok/export.h>
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
	using ProgramDefines = std::unordered_set<std::string>;

	struct DARMOK_EXPORT ProgramShaderDefinition final
	{
		std::string vertex;
		std::string fragment;
		ProgramDefines defines;

		void read(const nlohmann::ordered_json& json);
		void write(nlohmann::ordered_json& json);

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(vertex, fragment, defines);
		}
	};
	
	struct DARMOK_EXPORT ProgramDefinition final
	{
		using Shader = ProgramShaderDefinition;

		std::vector<Shader> shaders;
		bgfx::VertexLayout vertexLayout;

		void read(const nlohmann::ordered_json& json);
		void write(nlohmann::ordered_json& json);

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(shaders, vertexLayout);
		}
	};

	class DARMOK_EXPORT Program final
	{
	public:
		using Defines = ProgramDefines;
		using Handles = std::unordered_map<Defines, bgfx::ProgramHandle>;

		Program(const bgfx::EmbeddedShader* embeddedShaders, const DataView& definitionData);
		Program(const Handles& handles, const bgfx::VertexLayout& layout) noexcept;
		~Program() noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;

		static Program loadBasic(const std::string& name);

		[[nodiscard]] bgfx::ProgramHandle getHandle(const Defines& defines) const noexcept;
		[[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept;
	private:
		Handles _handles;
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
		DataProgramLoader(IDataLoader& dataLoader) noexcept;
		[[nodiscard]] std::shared_ptr<Program> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
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
		bool startImport(const Input& input, bool dry = false) override;
		void setLogOutput(OptionalRef<std::ostream> log) noexcept override;
		std::vector<std::filesystem::path> getOutputs(const Input& input) override;
		std::vector<std::filesystem::path> getDependencies(const Input& input) override;

		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
		const std::string& getName() const noexcept override;
		void endImport(const Input& input) override;
	private:
		std::unique_ptr<ShaderImporterImpl> _impl;
	};

	class ProgramImporterImpl;

	class DARMOK_EXPORT ProgramImporter final : public IAssetTypeImporter
	{
	public:
		ProgramImporter();
		~ProgramImporter() noexcept;

		bool startImport(const Input& input, bool dry = false) override;
		std::vector<std::filesystem::path> getOutputs(const Input& input) override;
		std::vector<std::filesystem::path> getDependencies(const Input& input) override;
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
		void endImport(const Input& input) override;

		const std::string& getName() const noexcept override;
	private:
		std::unique_ptr<ProgramImporterImpl> _impl;
	};
}