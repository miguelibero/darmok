#pragma once

#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/texture.hpp>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/program_def.hpp>

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

    struct ProgramDefinition;

    class Material final
    {
    public:
        Material(const ProgramDefinition& progDef) noexcept;
        ~Material();

        void load(AssetContext& assets);

        const ProgramDefinition& getProgramDefinition() const noexcept;
        const bgfx::VertexLayout& getVertexLayout() const noexcept;

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
        ProgramDefinition _progDef;
        bgfx::VertexLayout _vertexLayout;

        std::unordered_map<std::string, bgfx::UniformHandle> _samplerHandles;
        std::unordered_map<std::string, std::shared_ptr<Texture>> _defaultTextures;
        std::unordered_map<std::string, bgfx::UniformHandle> _uniformHandles;
        std::unordered_map<std::string, Data> _defaultUniforms;

        std::unordered_map<MaterialTextureType, std::shared_ptr<Texture>> _textures;
        std::unordered_map<MaterialColorType, Color> _colors;

        uint8_t _shininess;
        MaterialPrimitiveType _primitive;
    };



}