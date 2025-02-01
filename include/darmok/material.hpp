#pragma once

#include <darmok/export.h>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/app.hpp>
#include <darmok/uniform.hpp>
#include <darmok/texture_uniform.hpp>
#include <darmok/texture.hpp>
#include <darmok/glm.hpp>
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

namespace darmok
{
    class Texture;
    class Program;
    class App;

    class DARMOK_EXPORT Material final
    {
    public:
		constexpr static float defaultMetallicFactor = 0.F;
        constexpr static float defaultRoughnessFactor = 0.5F;
        constexpr static float defaultNormalScale = 1.F;
        constexpr static float defaultOcclusionStrength = 0.F;
        constexpr static float defaultWhiteFurnance = 0.F;
        constexpr static uint16_t defaultShininess = 32;

        using TextureType = MaterialTextureType;
        using PrimitiveType = MaterialPrimitiveType;
        using Textures = std::unordered_map<TextureType, std::shared_ptr<Texture>>;

        Material(const std::shared_ptr<Texture>& texture = nullptr) noexcept;
        Material(const std::shared_ptr<Program>& program) noexcept;
        Material(const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& texture) noexcept;
        Material(const std::shared_ptr<Program>& program, const Color& color) noexcept;

        static std::optional<OpacityType> readOpacityType(std::string_view name) noexcept;
        static const std::string& getOpacityTypeName(OpacityType opacity) noexcept;

        static std::optional<PrimitiveType> readPrimitiveType(std::string_view name) noexcept;
        static const std::string& getPrimitiveTypeName(PrimitiveType prim) noexcept;

        static std::optional<TextureType> readTextureType(std::string_view name) noexcept;
        static const std::string& getTextureTypeName(TextureType tex) noexcept;

        bool valid() const noexcept;

        const std::string& getName() const noexcept;
        Material& setName(const std::string& name) noexcept;

        const ProgramDefines& getProgramDefines() noexcept;
        Material& setProgramDefines(const ProgramDefines& defines) noexcept;
        Material& setProgramDefine(const std::string& define, bool enabled = true) noexcept;

        std::shared_ptr<Program> getProgram() const noexcept;
        Material& setProgram(const std::shared_ptr<Program>& prog) noexcept;
        bgfx::ProgramHandle getProgramHandle() const noexcept;

        PrimitiveType getPrimitiveType() const noexcept;
        Material& setPrimitiveType(PrimitiveType type) noexcept;

        const Textures& getTextures() const noexcept;
        std::shared_ptr<Texture> getTexture(TextureType type) const noexcept;
        Material& setTexture(const std::shared_ptr<Texture>& texture) noexcept;
        Material& setTexture(TextureType type, const std::shared_ptr<Texture>& texture) noexcept;

        Material& setTexture(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& texture) noexcept;
        Material& setUniform(const std::string& name, std::optional<UniformValue> value) noexcept;

        const UniformContainer& getUniformContainer() const noexcept;
        UniformContainer& getUniformContainer() noexcept;
        const TextureUniformContainer& getTextureUniformContainer() const noexcept;
        TextureUniformContainer& getTextureUniformContainer() noexcept;
        
        const Color& getBaseColor() const noexcept;
        Material& setBaseColor(const Color& v) noexcept;

        float getMetallicFactor() const noexcept;
        Material& setMetallicFactor(float v) noexcept;

        float getRoughnessFactor() const noexcept;
        Material& setRoughnessFactor(float v) noexcept;

        float getNormalScale() const noexcept;
        Material& setNormalScale(float v) noexcept;

        float getOcclusionStrength() const noexcept;
        Material& setOcclusionStrength(float v) noexcept;

        const Color3& getEmissiveColor() const noexcept;
        Material& setEmissiveColor(const Color3& v) noexcept;

        OpacityType getOpacityType() const noexcept;
        Material& setOpacityType(OpacityType v) noexcept;

        Material& setTwoSided(bool enabled) noexcept;
        bool getTwoSided() const noexcept;

        Material& setMultipleScattering(bool enabled) noexcept;
        bool getMultipleScattering() const noexcept;

        Material& setWhiteFurnanceFactor(float v) noexcept;
        float getWhiteFurnanceFactor() const noexcept;

        // for basic material (phong lighting)

        const Color3& getSpecularColor() const noexcept;
        Material& setSpecularColor(const Color3& v) noexcept;

        uint16_t getShininess() const noexcept;
        Material& setShininess(uint16_t v) noexcept;

        static void bindMeta();

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("name", _name),
                CEREAL_NVP_("program", _program),
                CEREAL_NVP_("programDefines", _programDefines),
                CEREAL_NVP_("textures", _textures),
                CEREAL_NVP_("baseColor", _baseColor),
                CEREAL_NVP_("specularColor", _specularColor),
                CEREAL_NVP_("metallicFactor", _metallicFactor),
                CEREAL_NVP_("roughnessFactor", _roughnessFactor),
                CEREAL_NVP_("normalScale", _normalScale),
                CEREAL_NVP_("occlusionStrength", _occlusionStrength),
                CEREAL_NVP_("emissiveColor", _emissiveColor),
                CEREAL_NVP_("opacityType", _opacityType),
                CEREAL_NVP_("shininess", _shininess),
                CEREAL_NVP_("twoSided", _twoSided),
                CEREAL_NVP_("multipleScattering", _multipleScattering),
                CEREAL_NVP_("whiteFurnance", _whiteFurnance),
                CEREAL_NVP_("primitive", _primitive),
                CEREAL_NVP_("uniforms", _uniforms),
                CEREAL_NVP_("textureUniforms", _textureUniforms)
            );
        }

    private:
        std::string _name;
        std::shared_ptr<Program> _program;
        ProgramDefines _programDefines;

        Textures _textures;

        Color _baseColor;
        Color3 _specularColor;
        float _metallicFactor;
        float _roughnessFactor;
        float _normalScale;
        float _occlusionStrength;
        Color3 _emissiveColor;
        OpacityType _opacityType;
        uint16_t _shininess;
        bool _twoSided;

        bool _multipleScattering;
        float _whiteFurnance;

        PrimitiveType _primitive;

        UniformContainer _uniforms;
        TextureUniformContainer _textureUniforms;

        static const std::array<std::string, toUnderlying(OpacityType::Count)> _opacityNames;
        static const std::array<std::string, toUnderlying(PrimitiveType::Count)> _primitiveTypeNames;
        static const std::array<std::string, toUnderlying(TextureType::Count)> _texTypeNames;
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

        struct Sampler final
        {
            TextureType type;
            bgfx::UniformHandle handle;
            uint8_t stage;
        };

        std::vector<Sampler> _samplerUniforms;
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

    class DARMOK_EXPORT BX_NO_VTABLE IMaterialLoader : public ILoader<Material>
    {
    };

    using CerealMaterialLoader = CerealLoader<IMaterialLoader>;
}