#pragma once

#include <darmok/export.h>
#include <darmok/color.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/app.hpp>
#include <darmok/uniform.hpp>
#include <darmok/texture.hpp>
#include <darmok/serialize.hpp>
#include <darmok/asset_serialize.hpp>
#include <darmok/loader.hpp>

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <array>

#include <bgfx/bgfx.h>
#include <bx/bx.h>

#include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/variant.hpp>

namespace darmok
{
    class Texture;
    class Program;
    class App;

    template<typename Prog, typename Tex>
    struct DARMOK_EXPORT GenericMaterial
    {
        using TextureType = MaterialTextureType;
        using PrimitiveType = MaterialPrimitiveType;

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

    };

    struct DARMOK_EXPORT MaterialSource final : GenericMaterial<std::variant<StandardProgramType, std::shared_ptr<ProgramSource>>, std::shared_ptr<TextureSource>>
    {
        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(program),
                CEREAL_NVP(programDefines),
                CEREAL_NVP(uniformValues),
                CEREAL_NVP(uniformTextures),
                CEREAL_NVP(opacityType),
                CEREAL_NVP(twoSided),
                CEREAL_NVP(primitiveType),
                CEREAL_NVP(baseColor),
                CEREAL_NVP(emissiveColor),
                CEREAL_NVP(metallicFactor),
                CEREAL_NVP(roughnessFactor),
                CEREAL_NVP(normalScale),
                CEREAL_NVP(occlusionStrength),
                CEREAL_NVP(multipleScattering),
                CEREAL_NVP(whiteFurnanceFactor),
                CEREAL_NVP(specularColor),
                CEREAL_NVP(shininess),
                CEREAL_NVP(textures)
            );
        }
    };

    struct DARMOK_EXPORT MaterialDefinition final : GenericMaterial<std::variant<StandardProgramType, std::shared_ptr<ProgramDefinition>>, std::shared_ptr<TextureDefinition>>
    {
        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(program),
                CEREAL_NVP(programDefines),
                CEREAL_NVP(uniformValues),
                CEREAL_NVP(uniformTextures),
                CEREAL_NVP(opacityType),
                CEREAL_NVP(twoSided),
                CEREAL_NVP(primitiveType),
                CEREAL_NVP(baseColor),
                CEREAL_NVP(emissiveColor),
                CEREAL_NVP(metallicFactor),
                CEREAL_NVP(roughnessFactor),
                CEREAL_NVP(normalScale),
                CEREAL_NVP(occlusionStrength),
                CEREAL_NVP(multipleScattering),
                CEREAL_NVP(whiteFurnanceFactor),
                CEREAL_NVP(specularColor),
                CEREAL_NVP(shininess),
                CEREAL_NVP(textures)
            );
        }
    };

    struct DARMOK_EXPORT Material : GenericMaterial<std::shared_ptr<Program>, std::shared_ptr<Texture>>
    {
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