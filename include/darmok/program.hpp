#pragma once

#include <darmok/data.hpp>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <unordered_map>
#include <memory>
#include <string_view>

namespace darmok
{
	class Program final
	{
	public:
		Program(const bgfx::ProgramHandle& handle) noexcept;
		~Program() noexcept;
		const bgfx::ProgramHandle& getHandle() const noexcept;
	private:
		bgfx::ProgramHandle _handle;

		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;
	};

	class BX_NO_VTABLE IProgramLoader
	{
	public:
		virtual ~IProgramLoader() = default;
		virtual std::shared_ptr<Program> operator()(std::string_view vertexName, std::string_view fragmentName = "") = 0;
	};

	struct ProgramAttribDefinition final
	{
		bgfx::AttribType::Enum type;
		uint8_t num;
		bool normalize;
	};

	enum class ProgramUniform
	{
		Time,
		DiffuseTexture,
		DiffuseColor,
	};

	struct ProgramUniformDefinition
	{
		std::string name;
		bgfx::UniformType::Enum type;
		Data defaultValue;
		uint16_t num;

		bgfx::UniformHandle createHandle() const noexcept;
	};

	struct ProgramDefinition final
	{
		typedef std::unordered_map<bgfx::Attrib::Enum, ProgramAttribDefinition> AttribMap;
		typedef std::unordered_map<ProgramUniform, ProgramUniformDefinition> UniformMap;

		AttribMap attribs;
		UniformMap uniforms;

		bgfx::VertexLayout createVertexLayout() const noexcept;
	};
}