#pragma once
#include <darmok/color.hpp>
#include <darmok/Data.hpp>

#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

#include <vector>
#include <string_view>
#include <memory>
#include <variant>
#include <unordered_map>


namespace darmok
{
    class ModelMaterialPropertyCollection;
    class ModelMaterial;
    class ModelMaterialTexture;

    enum class MaterialTextureType
    {
        Unknown,
        Diffuse,
        Specular,
        Normal,
        Count,
    };


    struct MaterialPropertyKey
    {

    };

    class MaterialPropertyCollection final
    {
    public:
        bool has(const std::string& name) const;
        const Data& get(const std::string& name) const;
        void set(const std::string& name, Data&& data);

        static MaterialPropertyCollection fromModel(const ModelMaterialPropertyCollection& model, int textureIndex);

    private:
        std::unordered_map<std::string, Data> _properties;
    };

    class MaterialTexture final
    {
    public:
        MaterialTexture(bgfx::TextureHandle handle, MaterialTextureType type = MaterialTextureType::Diffuse, MaterialPropertyCollection&& props = {});

        const bgfx::TextureHandle& getHandle() const;
        MaterialTextureType getType() const;

        void setHandle(const bgfx::TextureHandle& v);
        void setType(MaterialTextureType v);

        const MaterialPropertyCollection& getProperties() const;
        MaterialPropertyCollection& getProperties();

        static MaterialTexture fromModel(const ModelMaterialTexture& texture, const ModelMaterial& material, const std::string& basePath = {});
    private:
        bgfx::TextureHandle _handle;
        MaterialTextureType _type;
        MaterialPropertyCollection _properties;
    };

    class Material final
    {
    public:
        Material() = default;
        Material(std::vector<MaterialTexture>&& textures, MaterialPropertyCollection&& props = {});
        static std::shared_ptr<Material> fromModel(const ModelMaterial& material, const std::string& basePath = {});

        const std::vector<MaterialTexture>& getTextures(MaterialTextureType type) const;
        const MaterialPropertyCollection& getProperties() const;

        MaterialTexture& addTexture(MaterialTexture&& texture);
        MaterialPropertyCollection& getProperties();
    private:
        std::unordered_map<MaterialTextureType, std::vector<MaterialTexture>> _textures;
        MaterialPropertyCollection _properties;
    };
}