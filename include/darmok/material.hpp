#pragma once

#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/material_fwd.hpp>

#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

#include <vector>
#include <string_view>
#include <memory>
#include <unordered_map>

namespace darmok
{
    class Texture;
    class Program;

    class Material final
    {
    public:
        DLLEXPORT Material(const std::shared_ptr<Texture>& diffuseTexture = nullptr) noexcept;
        DLLEXPORT Material(const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& diffuseTexture = nullptr) noexcept;
        DLLEXPORT ~Material();

        DLLEXPORT bool valid() const noexcept;

        DLLEXPORT std::shared_ptr<Program> getProgram() const noexcept;
        DLLEXPORT Material& setProgram(const std::shared_ptr<Program>& program) noexcept;

        DLLEXPORT std::shared_ptr<Texture> getTexture(MaterialTextureType type) const noexcept;
        DLLEXPORT Material& setTexture(MaterialTextureType type, const std::shared_ptr<Texture>& texture) noexcept;
        
        DLLEXPORT OptionalRef<const Color> getColor(MaterialColorType type) const noexcept;
        DLLEXPORT Material& setColor(MaterialColorType type, const Color& color) noexcept;
        
        DLLEXPORT MaterialPrimitiveType getPrimitiveType() const noexcept;
        DLLEXPORT Material& setPrimitiveType(MaterialPrimitiveType type) noexcept;
        
        DLLEXPORT uint8_t getShininess() const noexcept;
        DLLEXPORT Material& setShininess(uint8_t v) noexcept;

        DLLEXPORT float getSpecularStrength() const noexcept;
        DLLEXPORT Material& setSpecularStrength(float v) noexcept;

        DLLEXPORT uint64_t beforeRender(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept;

    private:
        std::shared_ptr<Program> _program;
        std::unordered_map<MaterialTextureType, bgfx::UniformHandle> _textureHandles;
        std::unordered_map<MaterialColorType, bgfx::UniformHandle> _colorHandles;
        bgfx::UniformHandle _mainHandle;

        std::unordered_map<MaterialTextureType, std::shared_ptr<Texture>> _textures;
        std::unordered_map<MaterialColorType, Color> _colors;

        glm::vec4 _mainData;
        MaterialPrimitiveType _primitive;

        void destroyHandles() noexcept;
    };



}