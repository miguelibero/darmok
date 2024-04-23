#pragma once

#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>

#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

#include <vector>
#include <string_view>
#include <memory>
#include <unordered_map>


namespace darmok
{
    enum class MaterialTextureType
    {
        Unknown,
        Diffuse,
        Specular,
        Normal,
        Count,
    };

    enum class MaterialColorType
    {
        Diffuse,
        Specular,
        Ambient,
        Emissive,
        Transparent,
        Reflective,
    };

    enum class MaterialPrimitiveType
    {
        Triangle,
        Line
    };

    class ITexture;

    class Material final
    {
    public:
        Material(const std::shared_ptr<ITexture>& diffuseTexture = nullptr) noexcept;
        ~Material();

        std::shared_ptr<ITexture> getTexture(MaterialTextureType type) const noexcept;
        Material& setTexture(MaterialTextureType type, const std::shared_ptr<ITexture>& texture) noexcept;

        OptionalRef<const Color> getColor(MaterialColorType type) const noexcept;
        Material& setColor(MaterialColorType type, const Color& color) noexcept;

        MaterialPrimitiveType getPrimitiveType() const noexcept;
        Material& setPrimitiveType(MaterialPrimitiveType type) noexcept;

        uint8_t getShininess() const noexcept;
        Material& setShininess(uint8_t v) noexcept;

        float getSpecularStrength() const noexcept;
        Material& setSpecularStrength(float v) noexcept;

        void bgfxConfig(bgfx::Encoder& encoder) const noexcept;

    private:
        std::unordered_map<MaterialTextureType, bgfx::UniformHandle> _textureHandles;
        std::unordered_map<MaterialColorType, bgfx::UniformHandle> _colorHandles;
        bgfx::UniformHandle _mainHandle;

        std::unordered_map<MaterialTextureType, std::shared_ptr<ITexture>> _textures;
        std::unordered_map<MaterialColorType, Color> _colors;

        glm::vec4 _mainData;
        MaterialPrimitiveType _primitive;

        void destroyHandles() noexcept;
    };



}