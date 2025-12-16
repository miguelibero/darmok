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

    struct DARMOK_EXPORT MaterialRenderConfig final
    {
        using TextureType = protobuf::MaterialTexture::Type;

        std::unordered_map<TextureType, TextureUniformKey> textureUniformKeys;
        UniformHandleContainer uniformHandles;

        UniformHandle albedoLutSamplerUniform;
        UniformHandle baseColorUniform;
        UniformHandle specularColorUniform;
        UniformHandle metallicRoughnessNormalOcclusionUniform;
        UniformHandle emissiveColorUniform;
        UniformHandle hasTexturesUniform;
        UniformHandle multipleScatteringUniform;
        BasicUniforms basicUniforms;
        std::shared_ptr<Texture> defaultTexture;

        MaterialRenderConfig() = default;
        MaterialRenderConfig(const MaterialRenderConfig& other) = delete;
        MaterialRenderConfig& operator=(const MaterialRenderConfig& other) = delete;
        MaterialRenderConfig(MaterialRenderConfig&& other) = default;
        MaterialRenderConfig& operator=(MaterialRenderConfig&& other) = default;

        static MaterialRenderConfig createDefault() noexcept;

        void reset() noexcept;
    };

    struct DARMOK_EXPORT Material final
    {
        using TextureType = protobuf::MaterialTexture::Type;
        using PrimitiveType = protobuf::Material::PrimitiveType;
        using DepthTest = protobuf::Material::DepthTest;
        using OpacityType = protobuf::Material::OpacityType;
        using Definition = protobuf::Material;
        using TextureDefinition = protobuf::MaterialTexture;
        using RenderConfig = MaterialRenderConfig;

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

        DepthTest depthTest = Definition::DepthLess;
        OpacityType opacityType = Definition::Opaque;
        bool twoSided = false;
        PrimitiveType primitiveType = Definition::Triangle;
        bool writeDepth = true;

        bool valid() const noexcept;

        Material() = default;
        Material(std::shared_ptr<Program> prog, std::shared_ptr<Texture> tex) noexcept;
        Material(std::shared_ptr<Program> prog, const Color& color) noexcept;

        expected<void, std::string> load(const Definition& def, IProgramLoader& progLoader, ITextureLoader& texLoader) noexcept;

        [[nodiscard]] static Definition createDefinition() noexcept;
        void renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder, OptionalRef<const RenderConfig> config = nullptr) const noexcept;
        static uint16_t getDepthTestFlag(Definition::DepthTest) noexcept;
    };

    class DARMOK_EXPORT MaterialAppComponent : public ITypeAppComponent<MaterialAppComponent>
    {
    public:
        using RenderConfig = MaterialRenderConfig;

        expected<void, std::string> init(App& app) noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        void renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder, const Material& material) const noexcept;
    private:
        std::optional<RenderConfig> _renderConfig;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IMaterialDefinitionLoader : public ILoader<Material::Definition>{};
    class DARMOK_EXPORT BX_NO_VTABLE IMaterialLoader : public  ILoader<Material>{};
    class DARMOK_EXPORT BX_NO_VTABLE IMaterialFromDefinitionLoader : public IFromDefinitionLoader<IMaterialLoader, Material::Definition>{};

    class DARMOK_EXPORT MaterialLoader final : public FromDefinitionLoader<IMaterialFromDefinitionLoader, IMaterialDefinitionLoader>
    {
    public:
        MaterialLoader(IMaterialDefinitionLoader& defLoader, IProgramLoader& progLoader, ITextureLoader& texLoader) noexcept;
    private:
        Result create(std::shared_ptr<Definition> def) noexcept override;
        IProgramLoader& _progLoader;
        ITextureLoader& _texLoader;
    };

    using DataMaterialDefinitionLoader = DataProtobufLoader<IMaterialDefinitionLoader>;
}