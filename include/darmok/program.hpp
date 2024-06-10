#pragma once

#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <string>
#include <darmok/program_fwd.hpp>

namespace bgfx
{
	struct EmbeddedShader;
}

namespace darmok
{
	class DataView;

	class Program final
	{
	public:
		DLLEXPORT Program(const std::string& name, const bgfx::EmbeddedShader* embeddedShaders, const DataView& vertexLayout);
		DLLEXPORT Program(const bgfx::ProgramHandle& handle, const bgfx::VertexLayout& layout) noexcept;
		DLLEXPORT ~Program() noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;

		[[nodiscard]] DLLEXPORT [[nodiscard]] const bgfx::ProgramHandle& getHandle() const noexcept;
		[[nodiscard]] DLLEXPORT [[nodiscard]] const bgfx::VertexLayout& getVertexLayout() const noexcept;
	private:
		bgfx::ProgramHandle _handle;
		bgfx::VertexLayout _layout;
	};

	class BX_NO_VTABLE IProgramLoader
	{
	public:
		using result_type = std::shared_ptr<Program>;

		DLLEXPORT virtual ~IProgramLoader() = default;
		DLLEXPORT virtual [[nodiscard]] result_type operator()(std::string_view name) = 0;
	};

	class IDataLoader;
	class IVertexLayoutLoader;

    class DataProgramLoader final : public IProgramLoader
	{
	public:
		struct Suffixes final
		{
			std::string vertex;
			std::string fragment;
			std::string vertexLayout;
		};

		DLLEXPORT static const Suffixes& getDefaultSuffixes() noexcept;

		DLLEXPORT DataProgramLoader(IDataLoader& dataLoader, IVertexLayoutLoader& vertexLayoutLoader, Suffixes suffixes = getDefaultSuffixes()) noexcept;
		DLLEXPORT [[nodiscard]] std::shared_ptr<Program> operator()(std::string_view name) override;
	private:
		IDataLoader& _dataLoader;
		IVertexLayoutLoader& _vertexLayoutLoader;
		Suffixes _suffixes;

		bgfx::ShaderHandle loadShader(const std::string& filePath);
	};
}