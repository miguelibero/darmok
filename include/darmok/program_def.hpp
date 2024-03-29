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

		[[nodiscard]] bool operator==(const ProgramAttribDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramAttribDefinition& other) const noexcept;
	};

	enum class ProgramSampler
	{
		DiffuseTexture,
	};

	class ProgramSamplerDefinition final
	{
	public:
		ProgramSamplerDefinition(std::string name, uint8_t stage = 0) noexcept;
		ProgramSamplerDefinition(std::string name, std::string defaultTextureName, uint8_t stage = 0) noexcept;

		[[nodiscard]] bool operator==(const ProgramSamplerDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramSamplerDefinition& other) const noexcept;

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] const std::string& getDefaultTextureName() const;
		[[nodiscard]] uint8_t getStage() const;

		[[nodiscard]] bgfx::UniformHandle createHandle() const noexcept;
	private:
		std::string _name;
		uint16_t _stage;
		std::string _defaultTextureName;
	};

	enum class ProgramUniform
	{
		Time,
		DiffuseColor,
		LightCount,
		AmbientLightColor
	};

	class ProgramUniformDefinition final
	{
	public:
		ProgramUniformDefinition(std::string name, bgfx::UniformType::Enum type, uint16_t num = 1) noexcept;
		ProgramUniformDefinition(std::string name, const glm::vec4& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const glm::mat4& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const glm::mat3& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const std::vector<glm::vec4>& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const std::vector<glm::mat4>& defaultValue) noexcept;
		ProgramUniformDefinition(std::string name, const std::vector<glm::mat3>& defaultValue) noexcept;

		[[nodiscard]] bool operator==(const ProgramUniformDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramUniformDefinition& other) const noexcept;

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] bgfx::UniformType::Enum getType() const;
		[[nodiscard]] const Data& getDefault() const;
		[[nodiscard]] uint16_t getNum() const;

		[[nodiscard]] bgfx::UniformHandle createHandle() const noexcept;

	private:
		std::string _name;
		bgfx::UniformType::Enum _type;
		uint16_t _num;
		Data _default;
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
	using ProgramSamplerMap = std::unordered_map<ProgramSampler, ProgramSamplerDefinition>;
	using ProgramBufferMap = std::unordered_map<ProgramBuffer, ProgramBufferDefinition>;

	struct ProgramDefinition final
	{
		ProgramAttribMap attribs;
		ProgramUniformMap uniforms;
		ProgramSamplerMap samplers;
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

		[[nodiscard]] bool hasSampler(ProgramSampler sampler, const ProgramSamplerMap& defs) const noexcept;
		[[nodiscard]] bool hasSampler(ProgramSampler sampler, const ProgramSamplerDefinition& def) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramSamplerDefinition> getSampler(ProgramSampler sampler) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramSamplerDefinition> getSampler(ProgramSampler sampler, std::string_view name, uint8_t stage = 0) const noexcept;

		static ProgramDefinition getStandard(StandardProgramType type) noexcept;
	};

	class BX_NO_VTABLE IProgramDefinitionLoader
	{
	public:
		using result_type = std::shared_ptr<ProgramDefinition>;

		virtual ~IProgramDefinitionLoader() = default;
		virtual result_type operator()(std::string_view name) = 0;
	};
}