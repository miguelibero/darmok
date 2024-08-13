#pragma once

#include <darmok/export.h>
#include <darmok/varying.hpp>
#include <darmok/data.hpp>
#include <darmok/collection.hpp>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>

namespace darmok
{
	class DataView;

	using ProgramDefines = std::unordered_set<std::string>;

	enum class ProgramDefinitionFormat
	{
		Binary,
		Json,
		Xml
	};

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
		VertexLayout vertexLayout;

		const Profile& getCurrentProfile() const;

		using Format = ProgramDefinitionFormat;

		bool empty() const noexcept;

		void read(const std::filesystem::path& path);
		void write(const std::filesystem::path& path) const noexcept;
		void read(std::istream& in, Format format = Format::Binary);
		void write(std::ostream& out, Format format = Format::Binary) const noexcept;
		void read(const nlohmann::ordered_json& json);
		void write(nlohmann::ordered_json& json) const;
		static Format getPathFormat(const std::filesystem::path& path) noexcept;

		template<typename T>
		void loadStaticMem(const T& mem)
		{
			load(DataView(mem, sizeof(mem)));
		}

		void load(DataView data);

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(name, profiles, vertexLayout);
		}
	private:
		static const std::unordered_map<bgfx::RendererType::Enum, std::vector<std::string>> _rendererProfiles;
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
}