#pragma once

#include <darmok/program.hpp>
#include <darmok/data.hpp>
#include <darmok/optional_ref.hpp>
#include <glm/glm.hpp>
#include <unordered_map>
#include <string>

namespace darmok
{
	struct ProgramAttribDefinition final
	{
		bgfx::AttribType::Enum type;
		uint8_t num;
		bool normalize;

		void updateVertexLayout(bgfx::Attrib::Enum attrib, bgfx::VertexLayout& layout) const noexcept;
		[[nodiscard]] bool operator==(const ProgramAttribDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramAttribDefinition& other) const noexcept;
		ProgramAttribDefinition& operator+=(const ProgramAttribDefinition& other) noexcept;
		[[nodiscard]] ProgramAttribDefinition operator+(const ProgramAttribDefinition& other) const  noexcept;
	};

	enum class ProgramSampler
	{
		DiffuseTexture,
	};

	struct ProgramSamplerDefinition final
	{
	public:
		std::string name;
		uint16_t stage;
		std::string defaultTexture;

		ProgramSamplerDefinition(std::string name, uint8_t stage = 0) noexcept;
		ProgramSamplerDefinition(std::string name, std::string defaultTexture, uint8_t stage = 0) noexcept;

		[[nodiscard]] bool operator==(const ProgramSamplerDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramSamplerDefinition& other) const noexcept;

		[[nodiscard]] bgfx::UniformHandle createHandle() const noexcept;

	};

	enum class ProgramUniform
	{
		Time,
		DiffuseColor,
		LightCount,
		AmbientLightColor
	};

	struct ProgramUniformDefinition final
	{
	public:
		std::string name;
		bgfx::UniformType::Enum type;
		uint16_t num;
		Data defaultValue;

		ProgramUniformDefinition(std::string name, bgfx::UniformType::Enum type, uint16_t num = 1) noexcept;
		ProgramUniformDefinition(std::string name, bgfx::UniformType::Enum type, uint16_t num, Data&& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const glm::vec4& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const glm::mat4& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const glm::mat3& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const std::vector<glm::vec4>& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const std::vector<glm::mat4>& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const std::vector<glm::mat3>& defaultValue) noexcept;

		[[nodiscard]] bool operator==(const ProgramUniformDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramUniformDefinition& other) const noexcept;

		[[nodiscard]] bgfx::UniformHandle createHandle() const noexcept;
	};

	enum class ProgramBuffer
	{
		PointLights,
	};

	// TODO: check why not all types of attribs seem to work (DX11)
	// so instead of using AttribDefinition we use this enum
	enum class ProgramBufferAttribType
	{
		Vec4,
		Uvec4,
		Int,
		Uint,
		Bool,
	};

	struct ProgramBufferDefinition final
	{
		uint8_t stage;
		ProgramBufferAttribType type;
		std::vector<bgfx::Attrib::Enum> attribs;

		[[nodiscard]] bool operator==(const ProgramBufferDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramBufferDefinition& other) const noexcept;
		ProgramBufferDefinition& operator+=(const ProgramBufferDefinition& other) noexcept;
		[[nodiscard]] ProgramBufferDefinition operator+(const ProgramBufferDefinition& other) const  noexcept;

		[[nodiscard]] bool contains(const ProgramBufferDefinition& other) const noexcept;
		[[nodiscard]] bool hasAttrib(bgfx::Attrib::Enum attrib) const noexcept;

		[[nodiscard]] bgfx::VertexLayout createVertexLayout() const noexcept;
	};

	using ProgramAttribMap = std::unordered_map<bgfx::Attrib::Enum, ProgramAttribDefinition>;
	using ProgramUniformMap = std::unordered_map<ProgramUniform, ProgramUniformDefinition>;
	using ProgramSamplerMap = std::unordered_map<ProgramSampler, ProgramSamplerDefinition>;
	using ProgramBufferMap = std::unordered_map<ProgramBuffer, ProgramBufferDefinition>;

	struct ProgramDefinition final
	{
		ProgramAttribMap attribs;
		ProgramUniformMap uniforms;
		ProgramSamplerMap samplers;
		ProgramBufferMap buffers;

		bool empty() const noexcept;

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

		[[nodiscard]] bool hasSampler(ProgramSampler sampler, const ProgramSamplerMap& defs) const noexcept;
		[[nodiscard]] bool hasSampler(ProgramSampler sampler, const ProgramSamplerDefinition& def) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramSamplerDefinition> getSampler(ProgramSampler sampler) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramSamplerDefinition> getSampler(ProgramSampler sampler, std::string_view name, uint8_t stage = 0) const noexcept;

		static ProgramDefinition createFromJson(std::string_view data);
		static ProgramDefinition createStandard(StandardProgramType type);
	};

	class BX_NO_VTABLE IProgramDefinitionLoader
	{
	public:
		using result_type = ProgramDefinition;

		virtual ~IProgramDefinitionLoader() = default;
		virtual result_type operator()(std::string_view name) = 0;
	};
}