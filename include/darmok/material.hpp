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

    class DARMOK_EXPORT Material final
    {
    public:
        using ColorType = MaterialColorType;
        using TextureType = MaterialTextureType;
        using PrimitiveType = MaterialPrimitiveType;

        Material(const std::shared_ptr<Texture>& diffuseTexture = nullptr) noexcept;
        Material(const std::shared_ptr<Program>& program) noexcept;
        Material(const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& diffuseTexture) noexcept;
        Material(const std::shared_ptr<Program>& program, const Color& diffuseColor) noexcept;
        ~Material();
        Material(const Material& other) noexcept;

        bool valid() const noexcept;

        const ProgramDefines& getProgramDefines() noexcept;
        Material& setProgramDefines(const ProgramDefines& defines) noexcept;
        Material& setProgramDefine(const std::string& define, bool enabled = true) noexcept;

        std::shared_ptr<Program> getProgram() const noexcept;
        Material& setProgram(const std::shared_ptr<Program>& prog) noexcept;

        std::shared_ptr<Texture> getTexture(TextureType type) const noexcept;
        Material& setTexture(TextureType type, const std::shared_ptr<Texture>& texture) noexcept;
        
        std::optional<Color> getColor(ColorType type) const noexcept;
        Material& setColor(ColorType type, const std::optional<Color>& color) noexcept;
        
        PrimitiveType getPrimitiveType() const noexcept;
        Material& setPrimitiveType(PrimitiveType type) noexcept;
        
        uint8_t getShininess() const noexcept;
        Material& setShininess(uint8_t v) noexcept;

        float getSpecularStrength() const noexcept;
        Material& setSpecularStrength(float v) noexcept;

        void renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;

    private:
        std::shared_ptr<Program> _program;
        ProgramDefines _programDefines;
        std::unordered_map<TextureType, bgfx::UniformHandle> _textureHandles;
        std::unordered_map<ColorType, bgfx::UniformHandle> _colorHandles;
        bgfx::UniformHandle _mainHandle;

        std::unordered_map<TextureType, std::shared_ptr<Texture>> _textures;
        std::unordered_map<ColorType, Color> _colors;

        glm::vec4 _mainData;
        PrimitiveType _primitive;

        void createHandles() noexcept;
        void destroyHandles() noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IMaterialLoader
    {
    public:
        using result_type = std::shared_ptr<Material>;
        virtual ~IMaterialLoader() = default;
        virtual [[nodiscard]] result_type operator()(std::string_view name) = 0;
    };
}