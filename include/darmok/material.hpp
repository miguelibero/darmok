#pragma once

#include <darmok/export.h>
#include <darmok/color.hpp>
#include <darmok/program.hpp>
#include <darmok/app.hpp>
#include <darmok/uniform.hpp>
#include <darmok/texture.hpp>
#include <darmok/loader.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/material.pb.h>

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <array>

#include <bgfx/bgfx.h>
#include <bx/bx.h>

namespace darmok
{
    class Texture;
    class Program;
    class App;

    class DARMOK_EXPORT ConstMaterialDefinitionWrapper
    {
    public:
		using Definition = protobuf::Material;
        using TextureType = protobuf::MaterialTexture::Type;
		ConstMaterialDefinitionWrapper(const Definition& def) noexcept;

        std::optional<std::string> getTexturePath(TextureType textureType) noexcept;
        std::optional<std::string> getTexturePath(const TextureUniformKey& uniformKey) noexcept;
    private:
        const Definition& _def;

    };

    class DARMOK_EXPORT MaterialDefinitionWrapper final : public ConstMaterialDefinitionWrapper
    {
    public:
        MaterialDefinitionWrapper(Definition& def) noexcept;

        bool setTexturePath(TextureType textureType, const std::string& texturePath) noexcept;
        bool setTexturePath(const TextureUniformKey& uniformKey, const std::string& texturePath) noexcept;
    private:
        Definition& _def;

        using MaterialTextures = google::protobuf::RepeatedPtrField<darmok::protobuf::MaterialTexture>;
        bool setTexturePath(MaterialTextures& textures, MaterialTextures::iterator itr, const std::string& texturePath) noexcept;
    };

    struct DARMOK_EXPORT Material
    {
        using TextureType = protobuf::MaterialTexture::Type;
        using PrimitiveType = protobuf::Material::PrimitiveType;
        using OpacityType = protobuf::Material::OpacityType;
        using Definition = protobuf::Material;
        using TextureDefinition = protobuf::MaterialTexture;

        std::shared_ptr<Program> program;
        ProgramDefines programDefines;

        UniformValueMap uniformValues;
        UniformTextureMap uniformTextures;

        // PBR
        Color baseColor = Colors::white();
        Color3 emissiveColor = {};

        float metallicFactor = 0.F;
        float roughnessFactor = 0.5F;
        float normalScale = 1.F;
        float occlusionStrength = 0.F;

        bool multipleScattering = false;
        float whiteFurnanceFactor = false;

        // Phong
        Color3 specularColor = {};
        uint16_t shininess = 32;

        std::unordered_map<TextureType, std::shared_ptr<Texture>> textures;

        OpacityType opacityType = protobuf::Material::Opaque;
        bool twoSided = false;
        PrimitiveType primitiveType = protobuf::Material::Triangle;


        bool valid() const noexcept;

        Material() = default;
        Material(std::shared_ptr<Program> prog, std::shared_ptr<Texture> tex) noexcept;
        Material(std::shared_ptr<Program> prog, const Color& color) noexcept;
    };

    class DARMOK_EXPORT MaterialAppComponent : public ITypeAppComponent<MaterialAppComponent>
    {
    public:
        MaterialAppComponent() noexcept;
        ~MaterialAppComponent() noexcept;
        void init(App& app) override;
        void update(float deltaTime) override;
        void shutdown() override;
        void renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder, const Material& mat) const noexcept;
    private:
        using TextureType = Material::TextureType;

        std::unordered_map<TextureType, TextureUniformKey> _textureUniformKeys;
        UniformHandleContainer _uniformHandles;

        bgfx::UniformHandle _albedoLutSamplerUniform;
        bgfx::UniformHandle _baseColorUniform;
        bgfx::UniformHandle _specularColorUniform;
        bgfx::UniformHandle _metallicRoughnessNormalOcclusionUniform;
        bgfx::UniformHandle _emissiveColorUniform;
        bgfx::UniformHandle _hasTexturesUniform;
        bgfx::UniformHandle _multipleScatteringUniform;
        BasicUniforms _basicUniforms;
        std::shared_ptr<Texture> _defaultTexture;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IMaterialDefinitionLoader : public ILoader<Material::Definition>{};
    class DARMOK_EXPORT BX_NO_VTABLE IMaterialLoader : public  ILoader<Material>{};
    class DARMOK_EXPORT BX_NO_VTABLE IMaterialFromDefinitionLoader : public IFromDefinitionLoader<IMaterialLoader, Material::Definition>{};

    class DARMOK_EXPORT MaterialLoader final : public FromDefinitionLoader<IMaterialFromDefinitionLoader, IMaterialDefinitionLoader>
    {
    public:
        MaterialLoader(IMaterialDefinitionLoader& defLoader, ILoader<Program>& progLoader, ILoader<Texture>& texLoader) noexcept;
    private:
        Result create(const std::shared_ptr<Definition>& def) override;
        ILoader<Program>& _progLoader;
        ILoader<Texture>& _texLoader;
    };

    using DataMaterialDefinitionLoader = DataProtobufLoader<IMaterialDefinitionLoader>;
}