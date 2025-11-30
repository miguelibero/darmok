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
	class IProgramLoader;
	class IProgramDefinitionLoader;

	class DARMOK_EXPORT Program final
	{
	public:
		using Defines = ProgramDefines;
		using ShaderHandles = std::unordered_map<Defines, bgfx::ShaderHandle>;
		using ProgramHandles = std::unordered_map<Defines, bgfx::ProgramHandle>;
		using Definition = protobuf::Program;
		using Source = protobuf::ProgramSource;
		using Standard = protobuf::StandardProgram;
		using Ref = protobuf::ProgramRef;

		struct Handles final
		{
			ShaderHandles vertexHandles;
			ShaderHandles fragmentHandles;
			ProgramHandles programHandles;
		};
		
		Program(bgfx::VertexLayout layout, Handles handles) noexcept;
		~Program() noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;
		Program(Program&& other) = default;
		Program& operator=(Program&& other) = default;

		static expected<Program, std::string> load(const Definition& def) noexcept;

		[[nodiscard]] bgfx::ProgramHandle getHandle(const Defines& defines = {}) const noexcept;
		[[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept;

		template<class T>
        [[nodiscard]] static expected<Program, std::string> fromStaticMem(const T& mem) noexcept
		{
			Definition def;
			auto result = protobuf::readStaticMem(def, mem);
			if (!result)
			{
				unexpected<std::string>{ std::move(result).error() };
			}
			return load(def);
		}

		[[nodiscard]] static Source createSource() noexcept;
		[[nodiscard]] static expected<protobuf::Varying, std::string> loadRefVarying(const Ref& ref, OptionalRef<IProgramSourceLoader> loader = nullptr) noexcept;
		[[nodiscard]] static ILoader<Program::Definition>::Result loadRefDefinition(const Ref& ref, OptionalRef<IProgramDefinitionLoader> loader = nullptr) noexcept;
		[[nodiscard]] static ILoader<Program>::Result loadRef(const Ref& ref, OptionalRef<IProgramLoader> loader = nullptr) noexcept;
	private:

		static expected<ShaderHandles, std::string> createShaders(const google::protobuf::RepeatedPtrField<protobuf::Shader>& shaders, const std::string& name) noexcept;
		static bgfx::ShaderHandle findBestShader(const Defines& defines, const ShaderHandles& handles) noexcept;
		static expected<bgfx::ProgramHandle, std::string> createHandle(const Defines& defines, bgfx::ShaderHandle vertHandle, bgfx::ShaderHandle fragHandle) noexcept;

		Defines _allDefines;
		ShaderHandles _vertexHandles;
		ShaderHandles _fragmentHandles;
		ProgramHandles _handles;
		bgfx::VertexLayout _vertexLayout;
	};

	class DARMOK_EXPORT StandardProgramLoader final
	{
	public:
		using Type = protobuf::StandardProgram::Type;
		using Definition = protobuf::Program;
		static expected<std::shared_ptr<Program>, std::string> load(Type type) noexcept;
		static expected<std::shared_ptr<Definition>, std::string> loadDefinition(Type type) noexcept;
		static std::optional<Type> getType(const std::shared_ptr<Program>& prog) noexcept;
		static std::optional<Type> getType(const std::shared_ptr<Definition>& def) noexcept;
	private:
		static std::unordered_map<Type, std::weak_ptr<Definition>> _defCache;
		static std::unordered_map<Type, std::weak_ptr<Program>> _cache;

		static expected<void, std::string> loadDefinition(Definition& def, Type type);
	};

	class DARMOK_EXPORT BX_NO_VTABLE IProgramLoader : public ILoader<Program>{};
	class DARMOK_EXPORT BX_NO_VTABLE IProgramFromDefinitionLoader : public IFromDefinitionLoader<IProgramLoader, Program::Definition>{};
	using ProgramLoader = FromDefinitionLoader<IProgramFromDefinitionLoader, IProgramDefinitionLoader>;
}
