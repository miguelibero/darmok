#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/program_fwd.hpp>
#include <darmok/varying.hpp>
#include <darmok/data.hpp>
#include <darmok/collection.hpp>

namespace darmok
{
	struct ProgramDefinition;
	using ProgramDefines = std::unordered_set<std::string>;
	
	class DARMOK_EXPORT Program final
	{
	public:
		using Defines = ProgramDefines;
		using DefinesDataMap = std::unordered_map<Defines, Data>;
		using ShaderHandles = std::unordered_map<Defines, bgfx::ShaderHandle>;
		using Handles = std::unordered_map<Defines, bgfx::ProgramHandle>;
		using Definition = ProgramDefinition;

		Program(StandardProgramType type);
		Program(const Definition& definition);
		~Program() noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;

		const std::string& getName() const noexcept;

		[[nodiscard]] bgfx::ProgramHandle getHandle(const Defines& defines = {}) const noexcept;
		[[nodiscard]] const VertexLayout& getVertexLayout() const noexcept;

		static std::optional<StandardProgramType> getStandardType(std::string_view val) noexcept;
		static Definition getStandardDefinition(StandardProgramType type) noexcept;

	private:
		static const std::unordered_map<StandardProgramType, std::string> _standardTypes;

		ShaderHandles createShaders(const DefinesDataMap& defMap, const std::string& name);

		Defines _allDefines;
		ShaderHandles _vertexHandles;
		ShaderHandles _fragmentHandles;
		Handles _handles;
		VertexLayout _vertexLayout;
		std::string _name;
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
}