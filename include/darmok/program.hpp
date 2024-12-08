#pragma once

#include <darmok/export.h>
#include <darmok/program_fwd.hpp>
#include <darmok/program_core.hpp>
#include <darmok/loader.hpp>
#include <darmok/varying.hpp>
#include <darmok/data.hpp>
#include <darmok/collection.hpp>

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <optional>

#include <bx/bx.h>
#include <bgfx/bgfx.h>

namespace darmok
{
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

		static ShaderHandles createShaders(const DefinesDataMap& defMap, const std::string& name);
		static bgfx::ShaderHandle findBestShader(const Defines& defines, const ShaderHandles& handles) noexcept;
		void createHandle(const Defines& defines, bgfx::ShaderHandle vertHandle, bgfx::ShaderHandle fragHandle);

		Defines _allDefines;
		ShaderHandles _vertexHandles;
		ShaderHandles _fragmentHandles;
		Handles _handles;
		VertexLayout _vertexLayout;
		std::string _name;
	};

	class DARMOK_EXPORT BX_NO_VTABLE IProgramLoader : public IFromDefinitionLoader<Program, ProgramDefinition>
	{
	};

	class DARMOK_EXPORT ProgramLoader final : public FromDefinitionLoader<IProgramLoader, IProgramDefinitionLoader>
	{
	public:
		ProgramLoader(IProgramDefinitionLoader& loader) noexcept;
	};

}