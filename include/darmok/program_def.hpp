#pragma once

#include <darmok/program.hpp>
#include <darmok/data.hpp>
#include <darmok/optional_ref.hpp>
#include <glm/glm.hpp>
#include <map>
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

	enum class ProgramPackType
	{
		Float,
		Int,
		Uint8,
		Bool
	};

	template<typename T>
	struct ProgramPackTypeTrait
	{
		const static ProgramPackType value;
	};

	template<>
	struct ProgramPackTypeTrait<float>
	{
		const static ProgramPackType value = ProgramPackType::Float;
	};

	template<>
	struct ProgramPackTypeTrait<int>
	{
		const static ProgramPackType value = ProgramPackType::Int;
	};

	template<>
	struct ProgramPackTypeTrait<uint8_t>
	{
		const static ProgramPackType value = ProgramPackType::Uint8;
	};

	template<>
	struct ProgramPackTypeTrait<bool>
	{
		const static ProgramPackType value = ProgramPackType::Bool;
	};

	struct ProgramPackDefinition final
	{
		ProgramPackType type;
		uint8_t num;
		Data defaultValue;

		ProgramPackDefinition(ProgramPackType type, uint16_t num = 1) noexcept;
		ProgramPackDefinition(ProgramPackType type, uint16_t num, Data&& defaultValue) noexcept;

		template<typename T>
		ProgramPackDefinition(const T* defaultValue, uint16_t num) noexcept
			: ProgramPackDefinition(ProgramPackTypeTrait<T>::value, num, Data::copy(defaultValue, num))
		{
		}

		template<typename T>
		ProgramPackDefinition(T defaultValue) noexcept
			: ProgramPackDefinition(&defaultValue, 1)
		{
		}

		template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
		ProgramPackDefinition(const glm::mat<L1, L2, T, Q>& defaultValue) noexcept
			: ProgramPackDefinition(glm::value_ptr(defaultValue), L1* L2)
		{
		}

		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		ProgramPackDefinition(const glm::vec<L, T, Q>& defaultValue) noexcept
			: ProgramPackDefinition(glm::value_ptr(defaultValue), L)
		{
		}

		template<typename T>
		ProgramPackDefinition(const std::vector<T>& defaultValue) noexcept
			: ProgramPackDefinition(&defaultValue.front(), defaultValue.size())
		{
		}

		[[nodiscard]] bool operator==(const ProgramPackDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramPackDefinition& other) const noexcept;
	};

	using ProgramPackMap = std::map<std::string, ProgramPackDefinition>;

	struct ProgramUniformDefinition final
	{
	public:
		std::string name;
		bgfx::UniformType::Enum type;
		uint16_t num;
		ProgramPackMap packs;

		[[nodiscard]] bool operator==(const ProgramUniformDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramUniformDefinition& other) const noexcept;

		[[nodiscard]] bool contains(const ProgramUniformDefinition& other) const noexcept;
		[[nodiscard]] bgfx::UniformHandle createHandle() const noexcept;
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
		std::string name;
		ProgramBufferAttribType type;
		uint8_t stage;
		std::vector<bgfx::Attrib::Enum> attribs;
		ProgramPackMap packs;

		[[nodiscard]] bool operator==(const ProgramBufferDefinition& other) const noexcept;
		[[nodiscard]] bool operator!=(const ProgramBufferDefinition& other) const noexcept;
		ProgramBufferDefinition& operator+=(const ProgramBufferDefinition& other) noexcept;
		[[nodiscard]] ProgramBufferDefinition operator+(const ProgramBufferDefinition& other) const  noexcept;

		[[nodiscard]] bool contains(const ProgramBufferDefinition& other) const noexcept;
		[[nodiscard]] bool hasAttrib(bgfx::Attrib::Enum attrib) const noexcept;

		[[nodiscard]] bgfx::DynamicVertexBufferHandle createHandle() const noexcept;
	};

	using ProgramAttribMap = std::unordered_map<bgfx::Attrib::Enum, ProgramAttribDefinition>;
	using ProgramUniformMap = std::unordered_map<std::string, ProgramUniformDefinition>;
	using ProgramSamplerMap = std::unordered_map<std::string, ProgramSamplerDefinition>;
	using ProgramBufferMap = std::unordered_map<std::string, ProgramBufferDefinition>;

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
		
		[[nodiscard]] bool hasAttrib(bgfx::Attrib::Enum attrib) const noexcept;
		[[nodiscard]] bool hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribDefinition& def) const noexcept;
		
		[[nodiscard]] bool hasBuffer(std::string_view name, const ProgramDefinition& other) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramBufferDefinition> getBuffer(std::string_view name) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramBufferDefinition> getBuffer(std::string_view name, ProgramBufferAttribType type) const noexcept;

		[[nodiscard]] bool hasUniform(std::string_view name, const ProgramDefinition& other) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramUniformDefinition> getUniform(std::string_view name) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramUniformDefinition> getUniform(std::string_view name, bgfx::UniformType::Enum type) const noexcept;
		[[nodiscard]] OptionalRef<const ProgramUniformDefinition> getUniform(std::string_view name, bgfx::UniformType::Enum type, uint16_t num) const noexcept;

		[[nodiscard]] OptionalRef<const ProgramSamplerDefinition> getSampler(std::string_view name) const noexcept;

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