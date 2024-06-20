#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string>
#include <darmok/export.h>
#include <darmok/program_fwd.hpp>

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
}