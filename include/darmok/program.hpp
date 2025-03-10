#pragma once

#include <darmok/export.h>
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
		using Definition = protobuf::Program;

		Program(const Definition& def);
		~Program() noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;

		[[nodiscard]] bgfx::ProgramHandle getHandle(const Defines& defines = {}) const noexcept;
		[[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept;

	private:

		expected<void, std::string> createShaders(const protobuf::ProgramProfile& profile, const std::string& name);
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
		using Type = protobuf::StandardProgramType;
		using Definition = protobuf::Program;
		static std::shared_ptr<Program> load(Type::Enum type);
		static std::shared_ptr<Definition> loadDefinition(Type::Enum type);
		static std::optional<Type::Enum> getType(const std::shared_ptr<Program>& prog) noexcept;
		static std::optional<Type::Enum> getType(const std::shared_ptr<Definition>& def) noexcept;
	private:
		static std::unordered_map<Type::Enum, std::weak_ptr<Definition>> _defCache;
		static std::unordered_map<Type::Enum, std::weak_ptr<Program>> _cache;

		static expected<void, std::string> loadDefinition(Definition& def, Type::Enum type);
	};

	class DARMOK_EXPORT BX_NO_VTABLE IProgramLoader : public IFromDefinitionLoader<Program, Program::Definition>
	{
	};

	using ProgramLoader = FromDefinitionLoader<IProgramLoader, IProgramDefinitionLoader>;
}