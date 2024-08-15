#pragma once

#include <darmok/export.h>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/program.hpp>

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

        Material& setTwoSided(bool enabled) noexcept;
        bool getTwoSided() const noexcept;

        Material& setMultipleScattering(bool enabled) noexcept;
        bool getMultipleScattering() const noexcept;

        Material& setWhiteFurnanceFactor(float v) noexcept;
        float getWhiteFurnanceFactor() const noexcept;

        void renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;

        static void staticInit(App& app) noexcept;
        static void staticShutdown() noexcept;

    private:
        std::shared_ptr<Program> _program;
        ProgramDefines _programDefines;

        std::unordered_map<TextureType, std::shared_ptr<Texture>> _textures;

        Color _baseColor;
        float _metallicFactor;
        float _roughnessFactor;
        float _normalScale;
        float _occlusionStrength;
        Color3 _emissiveColor;
        bool _opaque;
        bool _twoSided;

        bool _multipleScattering;
        float _whiteFurnance;

        PrimitiveType _primitive;

        struct StaticConfig;
        static std::unique_ptr<StaticConfig> _staticConfig;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IMaterialLoader
    {
    public:
        using result_type = std::shared_ptr<Material>;
        virtual ~IMaterialLoader() = default;
        virtual [[nodiscard]] result_type operator()(std::string_view name) = 0;
    };
}