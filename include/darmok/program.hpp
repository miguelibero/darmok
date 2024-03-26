#pragma once

#include <darmok/data.hpp>
#include <darmok/optional_ref.hpp>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>
#include <string_view>

namespace darmok
{
	class Texture;

	class Program final
	{
	public:
		Program(const bgfx::ProgramHandle& handle) noexcept;
		Program(const Program& other) = delete;
		Program& operator=(const Program& other) = delete;
		~Program() noexcept;
		[[nodiscard]] const bgfx::ProgramHandle& getHandle() const noexcept;
	private:
		bgfx::ProgramHandle _handle;
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

		[[nodiscard]] bool operator==(const ProgramAttribDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramAttribDefinition& other) const noexcept;
	};

	enum class ProgramUniform
	{
		Time,
		DiffuseTexture,
		DiffuseColor,
		LightCount,
		AmbientLightColor
	};

	class ProgramUniformDefinition final
	{
	public:
		ProgramUniformDefinition(std::string name, bgfx::UniformType::Enum type, uint16_t num = 1) noexcept;
		ProgramUniformDefinition(std::string name, std::shared_ptr<Texture>& defaultValue, uint16_t num = 1) noexcept;
		ProgramUniformDefinition(std::string name, const glm::vec4& defaultValue, uint16_t num = 1) noexcept;
		ProgramUniformDefinition(std::string name, const glm::mat3& defaultValue, uint16_t num = 1) noexcept;
		ProgramUniformDefinition(std::string name, const glm::mat4& defaultValue, uint16_t num = 1) noexcept;

		[[nodiscard]] bool operator==(const ProgramUniformDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramUniformDefinition& other) const noexcept;

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] bgfx::UniformType::Enum getType() const;
		[[nodiscard]] const Data& getDefaultValue() const;
		[[nodiscard]] const std::shared_ptr<Texture>& getDefaultTexture() const;
		[[nodiscard]] uint16_t getNum() const;

		[[nodiscard]] bgfx::UniformHandle createHandle() const noexcept;

	private:
		std::string _name;
		bgfx::UniformType::Enum _type;
		uint16_t _num;
		Data _defaultValue;
		std::shared_ptr<Texture> _defaultTexture;
	};

	enum class ProgramBuffer
	{
		PointLights,
	};

	using ProgramAttribMap = std::unordered_map<bgfx::Attrib::Enum, ProgramAttribDefinition>;

	struct ProgramBufferDefinition final
	{
		uint8_t stage;
		ProgramAttribMap attribs;

		[[nodiscard]] bool operator==(const ProgramBufferDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramBufferDefinition& other) const noexcept;
		ProgramBufferDefinition& operator+=(const ProgramBufferDefinition& other) noexcept;
		[[nodiscard]] ProgramBufferDefinition operator+(const ProgramBufferDefinition& other) const  noexcept;

		[[nodiscard]] bool contains(const ProgramBufferDefinition& other) const noexcept;
		[[nodiscard]] bool hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribMap& defs) const noexcept;
		[[nodiscard]] bool hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribDefinition& def) const noexcept;

		[[nodiscard]] bgfx::VertexLayout createVertexLayout() const noexcept;
	};

	using ProgramUniformMap = std::unordered_map<ProgramUniform, ProgramUniformDefinition>;
	using ProgramBufferMap = std::unordered_map<ProgramBuffer, ProgramBufferDefinition>;

	struct ProgramDefinition final
	{
		ProgramAttribMap attribs;
		ProgramUniformMap uniforms;
		ProgramBufferMap buffers;

		[[nodiscard]] bool operator==(const ProgramDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramDefinition& other) const noexcept;
		ProgramDefinition& operator+=(const ProgramDefinition& other) noexcept;
		[[nodiscard]] ProgramDefinition operator+(const ProgramDefinition& other) const  noexcept;

		[[nodiscard]] bgfx::VertexLayout createVertexLayout() const noexcept;
		[[nodiscard]] bool contains(const ProgramDefinition& other) const noexcept;
		[[nodiscard]] bool hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribMap& defs) const noexcept;
		[[nodiscard]] bool hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribDefinition& def) const noexcept;
		[[nodiscard]] bool hasBuffer(ProgramBuffer buffer, const ProgramBufferMap& defs) const noexcept;
		[[nodiscard]] bool hasBuffer(ProgramBuffer buffer, const ProgramBufferDefinition& def) const noexcept;
		[[nodiscard]] bool hasUniform(ProgramUniform uniform, const ProgramUniformMap& defs) const noexcept;
		[[nodiscard]] bool hasUniform(ProgramUniform uniform, const ProgramUniformDefinition& def) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramUniformDefinition> getUniform(ProgramUniform uniform) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramUniformDefinition> getUniform(ProgramUniform uniform, std::string_view name) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramUniformDefinition> getUniform(ProgramUniform uniform, std::string_view name, bgfx::UniformType::Enum type, uint16_t num = 1) const noexcept;
	};
}