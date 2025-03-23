#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>

#include <variant>
#include <string>
#include <random>
#include <unordered_map>

#include <bgfx/bgfx.h>

namespace darmok
{
    struct DARMOK_EXPORT UniformValue final
    {
        using Variant = std::variant<glm::vec4, glm::mat4, glm::mat3>;

        UniformValue(const UniformValue& other) = default;
        UniformValue(UniformValue&& other) = default;
        UniformValue& operator=(const UniformValue& other) = default;
        UniformValue& operator=(UniformValue&& other) = default;
        bool operator==(const UniformValue& other) const = default;
        bool operator!=(const UniformValue& other) const = default;

        UniformValue(const Variant& val) noexcept;
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

    struct TextureUniformKey final
    {
        std::string name;
        uint8_t stage = 0;

        bool operator==(const TextureUniformKey& other) const noexcept;
        bool operator!=(const TextureUniformKey& other) const noexcept;

        size_t hash() const noexcept;
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

    using UniformValueMap = std::unordered_map<std::string, UniformValue>;

    class Texture;

    using UniformTextureMap = std::unordered_map<TextureUniformKey, std::shared_ptr<Texture>>;

    class UniformHandleContainer final
    {
    public:
        ~UniformHandleContainer() noexcept;
        void configure(bgfx::Encoder& encoder, const UniformValueMap& values) const;
        void configure(bgfx::Encoder& encoder, const UniformTextureMap& textures) const;
        void configure(bgfx::Encoder& encoder, const TextureUniformKey& key, const std::shared_ptr<Texture>& tex) const noexcept;
        void configure(bgfx::Encoder& encoder, const std::string& name, const UniformValue& val) const noexcept;
        void shutdown() noexcept;
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

        bgfx::UniformHandle getHandle(const Key& key) const noexcept;

        mutable std::unordered_map<Key, bgfx::UniformHandle, Key::Hash> _handles;
    };
}

namespace std
{
    template<>
    struct hash<darmok::TextureUniformKey>
    {
        size_t operator()(const darmok::TextureUniformKey& key) const noexcept
        {
            return key.hash();
        }
    };
}