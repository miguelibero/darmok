#pragma once

#include <darmok/export.h>
#include <darmok/program_core.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/color.hpp>
#include <darmok/uniform.hpp>
#include <darmok/texture.hpp>

#include <cereal/cereal.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>

namespace darmok
{
    class DARMOK_EXPORT MaterialDefinition final
    {
    public:
        using TextureType = MaterialTextureType;
        using PrimitiveType = MaterialPrimitiveType;

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(program),
                CEREAL_NVP(programDefines),
                CEREAL_NVP(textures),
                CEREAL_NVP(baseColor),
                CEREAL_NVP(specularColor),
                CEREAL_NVP(metallicFactor),
                CEREAL_NVP(roughnessFactor),
                CEREAL_NVP(normalScale),
                CEREAL_NVP(occlusionStrength),
                CEREAL_NVP(emissiveColor),
                CEREAL_NVP(opacityType),
                CEREAL_NVP(shininess),
                CEREAL_NVP(twoSided),
                CEREAL_NVP(multipleScattering),
                CEREAL_NVP(whiteFurnance),
                CEREAL_NVP(primitive),
                CEREAL_NVP(uniforms),
                CEREAL_NVP(textureUniforms)
            );
        }
        ProgramDefinition program;
        ProgramDefines programDefines; 

        std::unordered_map<TextureType, std::shared_ptr<TextureDefinition>> textures;

        Color baseColor;
        Color3 specularColor;
        float metallicFactor;
        float roughnessFactor;
        float normalScale;
        float occlusionStrength;
        Color3 emissiveColor;
        OpacityType opacityType;
        uint16_t shininess;
        bool twoSided;

        bool multipleScattering;
        float whiteFurnance;

        PrimitiveType primitive;

        std::unordered_map<std::string, UniformValue> uniforms;
        std::unordered_map<TextureUniformKey, std::shared_ptr<TextureDefinition>, TextureUniformKey::Hash> textureUniforms;
    };
}