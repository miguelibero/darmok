#pragma once

#include <darmok/data.hpp>
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
	};

	enum class ProgramUniform
	{
		Time,
		DiffuseTexture,
		DiffuseColor,
		PointLight,
		DirectionalLight,
		SpotLight,
	};

	class ProgramUniformDefinition final
	{
	public:
		ProgramUniformDefinition(std::string name, bgfx::UniformType::Enum type, uint16_t num = 1) noexcept;
		ProgramUniformDefinition(std::string name, std::shared_ptr<Texture>& defaultValue, uint16_t num = 1) noexcept;
		ProgramUniformDefinition(std::string name, const glm::vec4& defaultValue, uint16_t num = 1) noexcept;
		ProgramUniformDefinition(std::string name, const glm::mat3& defaultValue, uint16_t num = 1) noexcept;
		ProgramUniformDefinition(std::string name, const glm::mat4& defaultValue, uint16_t num = 1) noexcept;

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] bgfx::UniformType::Enum getType() const;
		[[nodiscard]] const Data& getDefaultValue() const;
		[[nodiscard]] const std::shared_ptr<Texture>& getDefaultTexture() const;
		[[nodiscard]] uint16_t getNum() const;

		[[nodiscard]] bgfx::UniformHandle createHandle() const noexcept;


	private:
		std::string _name;
		bgfx::UniformType::Enum _type;
		Data _defaultValue;
		std::shared_ptr<Texture> _defaultTexture;
		uint16_t _num;
	};

	struct ProgramDefinition final
	{
		using AttribMap = std::unordered_map<bgfx::Attrib::Enum, ProgramAttribDefinition>;
		using UniformMap = std::unordered_map<ProgramUniform, ProgramUniformDefinition>;

		AttribMap attribs;
		UniformMap uniforms;

		[[nodiscard]] bgfx::VertexLayout createVertexLayout() const noexcept;
	};
}