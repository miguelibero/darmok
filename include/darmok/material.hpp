#pragma once

#include <darmok/export.h>
#include <darmok/color.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/app.hpp>
#include <darmok/uniform.hpp>
#include <darmok/texture.hpp>
#include <darmok/serialize.hpp>
#include <darmok/asset_serialize.hpp>
#include <darmok/loader.hpp>
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

    struct DARMOK_EXPORT Material
    {
        using TextureType = protobuf::MaterialTextureType::Enum;
        using PrimitiveType = MaterialPrimitiveType;
        using Definition = protobuf::Material;

        Prog program;
        ProgramDefines programDefines;

        UniformValueMap uniformValues;
        GenericUniformTextureMap<Tex> uniformTextures;

        // PBR
        Color baseColor = {};
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

        using TextureType = MaterialTextureType;
        std::unordered_map<TextureType, Tex> textures;

        OpacityType opacityType = OpacityType::Opaque;
        bool twoSided = false;
        PrimitiveType primitiveType = PrimitiveType::Triangle;


        bool valid() const noexcept;
        static void bindMeta();

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
        using TextureType = MaterialTextureType;

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

    class DARMOK_EXPORT BX_NO_VTABLE IMaterialDefinitionLoader : public ILoader<MaterialDefinition>
    {
    };

    using CerealMaterialDefinitionLoader = CerealLoader<IMaterialDefinitionLoader>;

    class DARMOK_EXPORT BX_NO_VTABLE IMaterialLoader : public IFromDefinitionLoader<Material, MaterialDefinition>
    {
    };

    class DARMOK_EXPORT BX_NO_VTABLE MaterialLoader final : public FromDefinitionLoader<IMaterialLoader, IMaterialDefinitionLoader>
    {
    public:
        MaterialLoader(IMaterialDefinitionLoader& defLoader, IProgramLoader& progLoader, ITextureLoader& texLoader) noexcept;
    protected:
        std::shared_ptr<Resource> create(const std::shared_ptr<Definition>& def) override;
        IProgramLoader& _progLoader;
        ITextureLoader& _texLoader;
    };
}