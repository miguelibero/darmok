#pragma once

#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>
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

    enum class StandardMaterialType
    {
        Sprite,
        Unlit,
        ForwardPhong,
    };

    enum class MaterialPrimitiveType
    {
        Triangle,
        Line
    };

    class Material final
    {
    public:
        Material(const std::shared_ptr<Program>& program, const ProgramDefinition& progDef) noexcept;
        ~Material() noexcept;

        Material(const Material& other) noexcept;
        Material& operator=(const Material& other) noexcept;

        Material(Material&& other) noexcept;
        Material& operator=(Material&& other) noexcept;

        const std::shared_ptr<Program>& getProgram() const noexcept;
        Material& setProgram(const std::shared_ptr<Program>& program, const ProgramDefinition& progDef) noexcept;
        const ProgramDefinition& getProgramDefinition() const noexcept;
        const bgfx::VertexLayout& getVertexLayout() const noexcept;

        std::shared_ptr<Texture> getTexture(MaterialTextureType type, uint8_t textureUnit = 0) const noexcept;
        Material& setTexture(MaterialTextureType type, const std::shared_ptr<Texture>& texture, uint8_t textureUnit = 0) noexcept;

        OptionalRef<const Color> getColor(MaterialColorType type) const noexcept;
        Material& setColor(MaterialColorType type, const Color& color) noexcept;

        MaterialPrimitiveType getPrimitiveType() const noexcept;
        Material& setPrimitiveType(MaterialPrimitiveType type) noexcept;

        uint8_t getShininess() const noexcept;
        Material& setShininess(uint8_t v) noexcept;

        void submit(bgfx::Encoder& encoder, bgfx::ViewId viewId = 0, uint32_t depth = 0) const;

        static std::shared_ptr<Material> createStandard(StandardMaterialType type) noexcept;

    private:
        void clearUniforms() noexcept;
        bgfx::UniformHandle getUniformHandle(ProgramUniform uniform) const noexcept;

        void submitTextures(bgfx::Encoder& encoder) const;
        void submitColors(bgfx::Encoder& encoder) const;

        std::shared_ptr<Program> _program;
        ProgramDefinition _progDef;
        bgfx::VertexLayout _vertexLayout;
        std::unordered_map<ProgramUniform, bgfx::UniformHandle> _uniforms;
        std::unordered_map<MaterialTextureType, std::unordered_map<uint8_t, std::shared_ptr<Texture>>> _textures;
        std::unordered_map<MaterialColorType, Color> _colors;
        uint8_t _shininess;
        MaterialPrimitiveType _primitive;
    };



}