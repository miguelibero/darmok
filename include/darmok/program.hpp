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
		using ShaderHandles = std::unordered_map<Defines, bgfx::ShaderHandle>;
		using Handles = std::unordered_map<Defines, bgfx::ProgramHandle>;
		using Definition = ProgramDefinition;

		Program(const Definition& def);
		~Program() noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;

		[[nodiscard]] bgfx::ProgramHandle getHandle(const Defines& defines = {}) const noexcept;
		[[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept;

	private:

		expected<void, std::string> createShaders(const ProgramDefinitionProfile& profile, const std::string& name);
		static bgfx::ShaderHandle findBestShader(const Defines& defines, const ShaderHandles& handles) noexcept;
		void createHandle(const Defines& defines, bgfx::ShaderHandle vertHandle, bgfx::ShaderHandle fragHandle);

		Defines _allDefines;
		ShaderHandles _vertexHandles;
		ShaderHandles _fragmentHandles;
		Handles _handles;
		bgfx::VertexLayout _vertexLayout;
	};

	class DARMOK_EXPORT StandardProgramLoader final
	{
	public:
		static std::shared_ptr<Program> load(StandardProgramType type);
		static std::shared_ptr<ProgramDefinition> loadDefinition(StandardProgramType type);
		static std::optional<StandardProgramType> getType(const std::shared_ptr<Program>& prog) noexcept;
		static std::optional<StandardProgramType> getType(const std::shared_ptr<ProgramDefinition>& def) noexcept;
	private:
		static std::unordered_map<StandardProgramType, std::weak_ptr<ProgramDefinition>> _defCache;
		static std::unordered_map<StandardProgramType, std::weak_ptr<Program>> _cache;

		static expected<void, std::string> loadDefinition(ProgramDefinition& def, StandardProgramType type);
	};

	class DARMOK_EXPORT BX_NO_VTABLE IProgramLoader : public IFromDefinitionLoader<Program, ProgramDefinition>
	{
	};

	using ProgramLoader = FromDefinitionLoader<IProgramLoader, IProgramDefinitionLoader>;
}