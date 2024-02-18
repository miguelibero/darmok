#pragma once
#include <darmok/color.hpp>
#include <darmok/data.hpp>

#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

#include <vector>
#include <string_view>
#include <memory>
#include <optional>
#include <unordered_map>


namespace darmok
{
    class Texture;

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

    class MaterialPropertyCollection final
    {
    public:
        bool has(const std::string& name) const;
        const Data& get(const std::string& name) const;
        void set(const std::string& name, Data&& data);
    private:
        std::unordered_map<std::string, Data> _properties;
    };

    class MaterialTexture final
    {
    public:
        MaterialTexture(const std::shared_ptr<Texture>& texture, MaterialTextureType type = MaterialTextureType::Diffuse, MaterialPropertyCollection&& props = {});

        const std::shared_ptr<Texture>& getTexture() const;
        MaterialTextureType getType() const;

        void setTexture(const std::shared_ptr<Texture>& v);
        void setType(MaterialTextureType v);

        const MaterialPropertyCollection& getProperties() const;
        MaterialPropertyCollection& getProperties();

    private:
        std::shared_ptr<Texture> _texture;
        MaterialTextureType _type;
        MaterialPropertyCollection _properties;
    };

    struct MaterialUniforms
    {
        bgfx::UniformHandle TextureColor;
        bgfx::UniformHandle DiffuseColor;
    };

    class Material final
    {
    public:
        Material() = default;
        Material(std::vector<MaterialTexture>&& textures, MaterialPropertyCollection&& props = {});

        const std::vector<MaterialTexture>& getTextures(MaterialTextureType type) const;
        const MaterialPropertyCollection& getProperties() const;

        MaterialTexture& addTexture(MaterialTexture&& texture);
        MaterialPropertyCollection& getProperties();

        std::optional<Color> getColor(MaterialColorType type);
        void setColor(MaterialColorType type, const Color& color);

        void configure(bgfx::Encoder& encoder, const MaterialUniforms& uniforms);

    private:
        std::unordered_map<MaterialTextureType, std::vector<MaterialTexture>> _textures;
        MaterialPropertyCollection _properties;
        std::unordered_map<MaterialColorType, Color> _colors;
    };
}