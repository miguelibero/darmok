#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <darmok/program_core.hpp>
#include <darmok/varying.hpp>
#include <darmok/data.hpp>

namespace darmok
{
	enum class StandardProgramType
	{
		Gui,
		Unlit,
		Forward
	};

	class DARMOK_EXPORT Program final
	{
	public:
		using Defines = ProgramDefines;
		using ShaderHandles = std::unordered_map<Defines, bgfx::ShaderHandle>;
		using Handles = std::unordered_map<Defines, bgfx::ProgramHandle>;
		using Definition = ProgramDefinition;

		Program(StandardProgramType type);
		Program(const Definition& definition);
		Program(const Handles& handles, const VertexLayout& layout) noexcept;
		~Program() noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;

		[[nodiscard]] bgfx::ProgramHandle getHandle(const Defines& defines = {}) const noexcept;
		[[nodiscard]] const VertexLayout& getVertexLayout() const noexcept;
	private:
		void load(const Definition& definition);
		ShaderHandles createShaders(const ProgramProfileDefinition::Map& defMap, const std::string& name);

		ShaderHandles _vertexHandles;
		ShaderHandles _fragmentHandles;
		Handles _handles;
		VertexLayout _vertexLayout;
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