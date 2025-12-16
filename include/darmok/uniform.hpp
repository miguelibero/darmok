#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/protobuf/material.pb.h>

#include <variant>
#include <string>
#include <random>
#include <unordered_map>

#include <bgfx/bgfx.h>

namespace darmok
{
    struct DARMOK_EXPORT UniformHandle final
    {
    public:
        UniformHandle() noexcept;
        UniformHandle(const std::string& name, bgfx::UniformType::Enum type, uint16_t num = 1) noexcept;
        ~UniformHandle() noexcept;
        UniformHandle(const UniformHandle& other) = delete;
        UniformHandle& operator=(const UniformHandle& other) = delete;
        UniformHandle(UniformHandle&& other) noexcept;
        UniformHandle& operator=(UniformHandle&& other) noexcept;
        operator bgfx::UniformHandle() const noexcept;
        const bgfx::UniformHandle& get() const noexcept;
        bool reset() noexcept;
        operator bool() const noexcept;
        bool valid() const noexcept;
    private:
        bgfx::UniformHandle _bgfx;
    };

    struct DARMOK_EXPORT UniformValue final
    {
        using Variant = std::variant<glm::vec4, glm::mat4, glm::mat3>;
        using Definition = protobuf::UniformValue;

        UniformValue(const UniformValue& other) = default;
        UniformValue(UniformValue&& other) = default;
        UniformValue& operator=(const UniformValue& other) = default;
        UniformValue& operator=(UniformValue&& other) = default;
        bool operator==(const UniformValue& other) const = default;
        bool operator!=(const UniformValue& other) const = default;

        UniformValue(const Definition& def) noexcept;
        UniformValue(const Variant& val = glm::vec4()) noexcept;
        UniformValue(Variant&& val) noexcept;
        UniformValue& operator=(const Variant& val) noexcept;
        UniformValue& operator=(Variant&& val) noexcept;

        template<typename V>
        operator V()
        {
            return std::get<V>(_value);
        }

        template<typename V>
        operator V&&()
        {
            return std::get<V>(std::move(_value));
        }

        bgfx::UniformType::Enum getType() const noexcept;
        const void* ptr() const noexcept;
    private:
        Variant _value;
    };

    using TextureUniformKey = protobuf::TextureUniformKey;


    class BasicUniforms final
    {
    public:
        BasicUniforms() noexcept;
        BasicUniforms(const BasicUniforms& other) = delete;
        BasicUniforms& operator=(const BasicUniforms& other) = delete;
        BasicUniforms(BasicUniforms&& other) = default;
        BasicUniforms& operator=(BasicUniforms&& other) = default;

        void clear() noexcept;
        void update(float deltaTime) noexcept;
        void configure(bgfx::Encoder& encoder) const noexcept;
    private:
        std::mt19937 _randomEngine;
        std::uniform_real_distribution<float> _randomDistFloat;
        std::uniform_int_distribution<int> _randomDistInt;
        UniformHandle _timeUniform;
        UniformHandle _randomUniform;
        glm::vec4 _timeValues;
        glm::vec4 _randomValues;
    };

    using UniformValueMap = std::unordered_map<std::string, UniformValue>;

    class Texture;

    using UniformTextureMap = std::unordered_map<TextureUniformKey, std::shared_ptr<Texture>>;

    class UniformHandleContainer final
    {
    public:
        UniformHandleContainer() = default;
        UniformHandleContainer(const UniformHandleContainer& other) = delete;
        UniformHandleContainer& operator=(const UniformHandleContainer& other) = delete;
        UniformHandleContainer(UniformHandleContainer&& other) = default;
        UniformHandleContainer& operator=(UniformHandleContainer&& other) = default;

        void configure(bgfx::Encoder& encoder, const UniformValueMap& values) const;
        void configure(bgfx::Encoder& encoder, const UniformTextureMap& textures) const;
        void configure(bgfx::Encoder& encoder, const TextureUniformKey& key, const std::shared_ptr<Texture>& tex) const noexcept;
        void configure(bgfx::Encoder& encoder, const std::string& name, const UniformValue& val) const noexcept;
        void clear() noexcept;
    private:

        struct Key final
        {
            std::string name;
            bgfx::UniformType::Enum type = bgfx::UniformType::Count;

            bool operator==(const Key& other) const noexcept;
            bool operator!=(const Key& other) const noexcept;

            size_t hash() const noexcept;

            struct Hash final
            {
                size_t operator()(const Key& key) const noexcept
                {
                    return key.hash();
                }
            };
        };

        const UniformHandle& getHandle(const Key& key) const noexcept;

        mutable std::unordered_map<Key, UniformHandle, Key::Hash> _handles;
    };
}