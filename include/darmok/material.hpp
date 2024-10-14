#pragma once

#include <darmok/export.h>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/program.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/app.hpp>
#include <darmok/uniform.hpp>
#include <darmok/texture.hpp>

#include <darmok/glm.hpp>
#include <bgfx/bgfx.h>
#include <bx/bx.h>

#include <vector>
#include <string_view>
#include <memory>
#include <unordered_map>

namespace darmok
{
    class Texture;
    class Program;
    class App;

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

    class DARMOK_EXPORT Material final
    {
    public:
        using TextureType = MaterialTextureType;
        using PrimitiveType = MaterialPrimitiveType;

        Material(const std::shared_ptr<Texture>& texture = nullptr) noexcept;
        Material(const std::shared_ptr<Program>& program) noexcept;
        Material(const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& texture) noexcept;
        Material(const std::shared_ptr<Program>& program, const Color& color) noexcept;

        bool valid() const noexcept;

        const ProgramDefines& getProgramDefines() noexcept;
        Material& setProgramDefines(const ProgramDefines& defines) noexcept;
        Material& setProgramDefine(const std::string& define, bool enabled = true) noexcept;

        std::shared_ptr<Program> getProgram() const noexcept;
        Material& setProgram(const std::shared_ptr<Program>& prog) noexcept;
        bgfx::ProgramHandle getProgramHandle() const noexcept;

        PrimitiveType getPrimitiveType() const noexcept;
        Material& setPrimitiveType(PrimitiveType type) noexcept;

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

    private:
        std::shared_ptr<Program> _program;
        ProgramDefines _programDefines;

        std::unordered_map<TextureType, std::shared_ptr<Texture>> _textures;

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
    };

    class DARMOK_EXPORT BX_NO_VTABLE IMaterialLoader
    {
    public:
        using result_type = std::shared_ptr<Material>;
        virtual ~IMaterialLoader() = default;
        virtual [[nodiscard]] result_type operator()(std::string_view name) = 0;
    };
}