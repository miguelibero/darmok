#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string>
#include <unordered_set>
#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/collection.hpp>
#include <darmok/data.hpp>
#include <nlohmann/json.hpp>

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

		void read(const nlohmann::json& json);
		void write(nlohmann::json& json) const;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(vertexShaders, fragmentShaders);
		}
	};
	
	struct DARMOK_EXPORT ProgramDefinition final
	{
		using Profile = ProgramProfileDefinition;

		std::string name;
		std::unordered_map<std::string, Profile> profiles;
		bgfx::VertexLayout vertexLayout;

		const Profile& getCurrentProfile() const;

		void read(const nlohmann::ordered_json& json);
		void write(nlohmann::ordered_json& json) const;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(name, profiles, vertexLayout);
		}
	private:
		static const std::unordered_map<bgfx::RendererType::Enum, std::vector<std::string>> _rendererProfiles;
	};

	class DARMOK_EXPORT Program final
	{
	public:
		using Defines = ProgramDefines;
		using ShaderHandles = std::unordered_map<Defines, bgfx::ShaderHandle>;
		using Handles = std::unordered_map<Defines, bgfx::ProgramHandle>;
		using Definition = ProgramDefinition;

		Program(const Definition& definition);
		Program(const Handles& handles, const bgfx::VertexLayout& layout) noexcept;
		~Program() noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;

		static Program loadBasic(const std::string& name);

		[[nodiscard]] bgfx::ProgramHandle getHandle(const Defines& defines = {}) const noexcept;
		[[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept;
	private:

		ShaderHandles createShaders(const ProgramProfileDefinition::Map& defMap, const std::string& name);

		ShaderHandles _vertexHandles;
		ShaderHandles _fragmentHandles;
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
	};

	class ProgramImporterImpl;

	class DARMOK_EXPORT ProgramImporter final : public IAssetTypeImporter
	{
	public:
		ProgramImporter();
		~ProgramImporter() noexcept;
		ProgramImporter& setShadercPath(const std::filesystem::path& path) noexcept;
		ProgramImporter& addIncludePath(const std::filesystem::path& path) noexcept;

		bool startImport(const Input& input, bool dry = false) override;
		Outputs getOutputs(const Input& input) override;
		Dependencies getDependencies(const Input& input) override;
		void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
		void endImport(const Input& input) override;

		const std::string& getName() const noexcept override;
	private:
		std::unique_ptr<ProgramImporterImpl> _impl;
	};
}