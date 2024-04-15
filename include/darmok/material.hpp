#pragma once

#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/texture.hpp>
#include <darmok/scene.hpp>
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

    class Material final
    {
    public:
        Material(const std::shared_ptr<Texture>& diffuseTexture = nullptr) noexcept;
        ~Material();

        std::shared_ptr<Texture> getTexture(MaterialTextureType type) const noexcept;
        Material& setTexture(MaterialTextureType type, const std::shared_ptr<Texture>& texture) noexcept;

        OptionalRef<const Color> getColor(MaterialColorType type) const noexcept;
        Material& setColor(MaterialColorType type, const Color& color) noexcept;

        MaterialPrimitiveType getPrimitiveType() const noexcept;
        Material& setPrimitiveType(MaterialPrimitiveType type) noexcept;

        uint8_t getShininess() const noexcept;
        Material& setShininess(uint8_t v) noexcept;

        void bgfxConfig(bgfx::Encoder& encoder) const noexcept;

    private:
        OptionalRef<AssetContext> _assets;
        std::unordered_map<MaterialTextureType, bgfx::UniformHandle> _textureHandles;
        std::unordered_map<MaterialColorType, bgfx::UniformHandle> _colorHandles;
        bgfx::UniformHandle _mainHandle;

        std::unordered_map<MaterialTextureType, std::shared_ptr<Texture>> _textures;
        std::unordered_map<MaterialColorType, Color> _colors;

        glm::vec4 _mainData;
        MaterialPrimitiveType _primitive;

        void destroyHandles() noexcept;
    };



}