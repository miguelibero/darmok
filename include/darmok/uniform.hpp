#pragma once

#include <darmok/export.h>
#include <unordered_map>
#include <variant>
#include <string>
#include <optional>
#include <darmok/glm.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

namespace darmok
{
    using UniformValue = std::variant<glm::vec4, glm::mat4, glm::mat3>;

    class Uniform final
    {
    public:
        using Value = UniformValue;
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
        Uniform& set(const UniformValue& value) noexcept;
        const UniformValue& get() const noexcept;

        const Uniform& configure(bgfx::Encoder& encoder) const;
        Uniform& configure(bgfx::Encoder& encoder) noexcept;
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

        void init() noexcept;
        void shutdown() noexcept;

        UniformContainer& set(const std::string& name, std::optional<UniformValue> value) noexcept;
        const UniformContainer& configure(bgfx::Encoder& encoder) const;
        UniformContainer& configure(bgfx::Encoder& encoder) noexcept;
    private:
        std::unordered_map<std::string, Uniform> _uniforms;
        bool _autoInit;
        bool _initialized;
    };
}