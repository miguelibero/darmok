#pragma once
#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>

#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

#include <vector>
#include <string_view>
#include <memory>
#include <optional>
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

    struct MaterialUniforms
    {
        bgfx::UniformHandle TextureColor;
        bgfx::UniformHandle DiffuseColor;

        static MaterialUniforms getDefault();
    };

    class Material final
    {
    public:
        Material() = default;
        Material(const std::shared_ptr<Program>& program, const MaterialUniforms& uniforms = MaterialUniforms::getDefault());

        const std::shared_ptr<Program>& getProgram() const;
        void setProgram(const std::shared_ptr<Program>& program);

        const std::vector<std::shared_ptr<Texture>>& getTextures(MaterialTextureType type) const;
        void addTexture(const std::shared_ptr<Texture>& texture, MaterialTextureType type = MaterialTextureType::Diffuse);

        std::optional<Color> getColor(MaterialColorType type);
        void setColor(MaterialColorType type, const Color& color);

        void submit(bgfx::Encoder& encoder, bgfx::ViewId viewId);

    private:
        std::shared_ptr<Program> _program;
        MaterialUniforms _uniforms;
        std::unordered_map<MaterialTextureType, std::vector<std::shared_ptr<Texture>>> _textures;
        std::unordered_map<MaterialColorType, Color> _colors;
    };
}