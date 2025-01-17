#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/glm_serialize.hpp>

#include <unordered_map>
#include <variant>
#include <string>
#include <optional>
#include <random>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <cereal/cereal.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/unordered_map.hpp>

namespace darmok
{
    using UniformValue = std::variant<glm::vec4, glm::mat4, glm::mat3>;

    class Uniform final
    {
    public:
        using Value = UniformValue;
        Uniform() noexcept;
        Uniform(const std::string& name, const Value& value, bool autoInit = true) noexcept;
        Uniform(const std::string& name, bgfx::UniformType::Enum type, bool autoInit = true) noexcept;
        ~Uniform() noexcept;
        
        Uniform(const Uniform& other) noexcept;
        Uniform& operator=(const Uniform& other) noexcept;
        Uniform(Uniform&& other) noexcept;
        Uniform& operator=(Uniform&& other) noexcept;

        void init() noexcept;
        void shutdown() noexcept;

        bool operator==(const Uniform& other) const noexcept;
        bool operator!=(const Uniform& other) const noexcept;

        Uniform& operator=(const UniformValue& value) noexcept;
        operator const UniformValue& () const noexcept;
        operator const glm::vec4& () const;
        operator const glm::mat3& () const;
        operator const glm::mat4& () const;

        static bgfx::UniformType::Enum getType(const Value& value) noexcept;

        Uniform& setType(bgfx::UniformType::Enum type) noexcept;
        Uniform& set(const Value& value) noexcept;
        const Value& get() const noexcept;

        const Uniform& configure(bgfx::Encoder& encoder) const;
        Uniform& configure(bgfx::Encoder& encoder) noexcept;

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("name", _name),
                CEREAL_NVP_("value", _value),
                CEREAL_NVP_("type", _type)
            );
        }

    private:
        std::string _name;
        Value _value;
        bgfx::UniformHandle _handle;
        bgfx::UniformType::Enum _type;

        void doConfigure(bgfx::Encoder& encoder) const noexcept;
    };

    class UniformContainer final
    {
    public:
        UniformContainer(bool autoInit = true) noexcept;
        ~UniformContainer() noexcept;

        void init() noexcept;
        void shutdown() noexcept;

        UniformContainer& set(const std::string& name, std::optional<UniformValue> value) noexcept;
        const UniformContainer& configure(bgfx::Encoder& encoder) const;
        UniformContainer& configure(bgfx::Encoder& encoder) noexcept;

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("uniforms", _uniforms)
            );
        }

    private:
        std::unordered_map<std::string, Uniform> _uniforms;
        bool _autoInit;
        bool _initialized;
    };

    class BasicUniforms final
    {
    public:
        BasicUniforms() noexcept;
        ~BasicUniforms() noexcept;
        void init() noexcept;
        void shutdown() noexcept;
        void update(float deltaTime) noexcept;
        void configure(bgfx::Encoder& encoder) const noexcept;
    private:
        std::mt19937 _randomEngine;
        std::uniform_real_distribution<float> _randomDistFloat;
        std::uniform_int_distribution<int> _randomDistInt;
        bgfx::UniformHandle _timeUniform;
        bgfx::UniformHandle _randomUniform;
        glm::vec4 _timeValues;
        glm::vec4 _randomValues;
    };
}