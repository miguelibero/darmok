#include <darmok/texture_pbr.hpp>

namespace darmok
{
    namespace PbrTextureUtils
    {
        uint32_t reverseBits(uint32_t bits)
        {
            bits = (bits << 16u) | (bits >> 16u);
            bits = ((bits & 0x55555555u) << 1u) |
                   ((bits & 0xAAAAAAAAu) >> 1u);
            bits = ((bits & 0x33333333u) << 2u) |
                   ((bits & 0xCCCCCCCCu) >> 2u);
            bits = ((bits & 0x0F0F0F0Fu) << 4u) |
                   ((bits & 0xF0F0F0F0u) >> 4u);
            bits = ((bits & 0x00FF00FFu) << 8u) |
                   ((bits & 0xFF00FF00u) >> 8u);

            return bits;
        }

        float radicalInverseVdC(uint32_t bits)
        {
            return static_cast<float>(reverseBits(bits)) *
                   2.3283064365386963e-10f;
        }

        glm::vec2 hammersley(uint32_t i, uint32_t sampleCount)
        {
            return {
                static_cast<float>(i) /
                    static_cast<float>(sampleCount),

                radicalInverseVdC(i)};
        }

        glm::vec3 importanceSampleGgx(
            const glm::vec2& xi,
            float alpha)
        {
            const float alpha2 = alpha * alpha;

            const float phi =
                2.0f * glm::pi<float>() * xi.x;

            const float cosTheta =
                std::sqrt(
                    (1.0f - xi.y) /
                    (1.0f + (alpha2 - 1.0f) * xi.y));

            const float sinTheta =
                std::sqrt(
                    std::max(
                        0.0f,
                        1.0f - cosTheta * cosTheta));

            return glm::normalize(glm::vec3{
                sinTheta * std::cos(phi),
                sinTheta * std::sin(phi),
                cosTheta});
        }

        float smithGgxCorrelated(
            float NoV,
            float NoL,
            float alpha)
        {
            const float alpha2 = alpha * alpha;

            const float lambdaV =
                NoL * std::sqrt(
                          NoV * NoV * (1.0f - alpha2) +
                          alpha2);

            const float lambdaL =
                NoV * std::sqrt(
                          NoL * NoL * (1.0f - alpha2) +
                          alpha2);

            return 0.5f /
                   std::max(
                       lambdaV + lambdaL,
                       1e-6f);
        }

        float fresnelSchlick(
            float VoH,
            float F0)
        {
            const float f =
                std::pow(1.0f - VoH, 5.0f);

            return F0 + (1.0f - F0) * f;
        }

        float integrateDirectionalAlbedo(
            float NoV,
            float alpha,
            float F0,
            uint32_t sampleCount)
        {
            const float sinThetaV =
                std::sqrt(
                    std::max(
                        0.0f,
                        1.0f - NoV * NoV));

            const glm::vec3 V{
                sinThetaV,
                0.0f,
                NoV};

            float result = 0.0f;

            for(uint32_t i = 0; i < sampleCount; ++i)
            {
                const glm::vec2 xi =
                    hammersley(i, sampleCount);

                const glm::vec3 H =
                    importanceSampleGgx(
                        xi,
                        alpha);

                const float VoH =
                    std::max(
                        glm::dot(V, H),
                        0.0f);

                if(VoH <= 0.0f)
                {
                    continue;
                }

                const glm::vec3 L =
                    glm::normalize(
                        2.0f * VoH * H - V);

                const float NoL =
                    std::max(L.z, 0.0f);

                if(NoL <= 0.0f)
                {
                    continue;
                }

                const float NoH =
                    std::max(H.z, 0.0f);

                // GGX NDF
                const float alpha2 =
                    alpha * alpha;

                const float denominator =
                    NoH * NoH * (alpha2 - 1.0f) +
                    1.0f;

                const float D =
                    alpha2 /
                    (glm::pi<float>() * denominator * denominator);

                // Smith visibility
                const float Vterm =
                    smithGgxCorrelated(
                        NoV,
                        NoL,
                        alpha);

                // Fresnel
                const float F =
                    fresnelSchlick(
                        VoH,
                        F0);

                // PDF of the sampled half vector.
                const float pdf =
                    D * NoH /
                    std::max(
                        4.0f * VoH,
                        1e-6f);

                // BRDF * NoL / PDF
                const float weight =
                    F *
                    Vterm *
                    D *
                    NoL /
                    std::max(pdf, 1e-6f);

                result += weight;
            }

            return result /
                   static_cast<float>(sampleCount);
        }

        float geometrySchlickGgx(
            float NoV,
            float roughness)
        {
            // UE4-style Schlick-GGX approximation.
            //
            // This is commonly used when generating the
            // standard split-sum BRDF LUT.
            const float k =
                (roughness * roughness) / 2.0f;

            return NoV /
                   (NoV * (1.0f - k) + k);
        }


        float geometrySmith(
            float NoV,
            float NoL,
            float roughness)
        {
            return geometrySchlickGgx(NoV, roughness) *
                   geometrySchlickGgx(NoL, roughness);
        }

        glm::vec2 integrateBrdf(
            float NoV,
            float roughness,
            uint32_t sampleCount)
        {
            // View direction in tangent space.
            const float sinThetaV =
                std::sqrt(
                    std::max(
                        0.0f,
                        1.0f - NoV * NoV));

            const glm::vec3 V{
                sinThetaV,
                0.0f,
                NoV};

            float A = 0.0f;
            float B = 0.0f;

            for(uint32_t i = 0; i < sampleCount; ++i)
            {
                const glm::vec2 Xi =
                    hammersley(i, sampleCount);

                const glm::vec3 H =
                    importanceSampleGgx(
                        Xi,
                        roughness);

                const float VoH =
                    std::max(
                        glm::dot(V, H),
                        0.0f);

                const glm::vec3 L =
                    glm::normalize(
                        2.0f * VoH * H - V);

                const float NoL =
                    std::max(L.z, 0.0f);

                if(NoL <= 0.0f)
                    continue;

                const float NoH =
                    std::max(H.z, 0.0f);

                const float G =
                    geometrySmith(
                        NoV,
                        NoL,
                        roughness);

                // Visibility term from the split-sum derivation.
                const float GVis =
                    (G * VoH) /
                    std::max(
                        NoH * NoV,
                        1e-6f);

                const float Fc =
                    std::pow(
                        1.0f - VoH,
                        5.0f);

                // F = F0 * (1 - Fc) + Fc
                //
                // Therefore:
                //
                // A accumulates the coefficient of F0
                // B accumulates the constant Fresnel term.
                A += (1.0f - Fc) * GVis;
                B += Fc * GVis;
            }

            const float invSampleCount =
                1.0f / static_cast<float>(sampleCount);

            return {
                A * invSampleCount,
                B * invSampleCount};
        }

        Definition createDefinition(std::vector<uint16_t> pixels, uint16_t size)
        {
            Definition def;
            auto& config = *def.mutable_config();

            def.set_data(std::string_view{
                reinterpret_cast<const char*>(pixels.data()),
                pixels.size() * sizeof(uint16_t)});

            config.mutable_size()->set_x(size);
            config.mutable_size()->set_y(size);
            config.set_format(Texture::Definition::RG16F);
            config.set_type(Texture::Definition::Texture2D);
            config.set_mips(false);
            config.set_depth(1);

            return def;
        }

        Definition createAlbedoLutDefinition(uint16_t size, uint32_t sampleCount)
        {
            // RG16F = 2 half-floats per pixel.
            //
            // uint16_t is exactly what we want here because each
            // channel is a 16-bit IEEE half float.
            std::vector<uint16_t> pixels(
                static_cast<size_t>(size) *
                static_cast<size_t>(size) *
                2);

            for(uint16_t y = 0; y < size; ++y)
            {
                const float alpha =
                    std::max(
                        static_cast<float>(y) /
                            static_cast<float>(size - 1),
                        0.001f);

                for(uint16_t x = 0; x < size; ++x)
                {
                    // X = NdotV
                    const float NoV =
                        std::max(
                            static_cast<float>(x) /
                                static_cast<float>(size - 1),
                            0.001f);

                    // F0 = 1
                    const float E1 =
                        integrateDirectionalAlbedo(
                            NoV,
                            alpha,
                            1.0f,
                            sampleCount);

                    // F0 = 0
                    const float E0 =
                        integrateDirectionalAlbedo(
                            NoV,
                            alpha,
                            0.0f,
                            sampleCount);

                    const size_t index =
                        (static_cast<size_t>(y) * size +
                         static_cast<size_t>(x)) *
                        2;

                    pixels[index + 0] =
                        bx::halfFromFloat(E1);

                    pixels[index + 1] =
                        bx::halfFromFloat(E0);
                }
            }

            return createDefinition(std::move(pixels), size);
        }

        Definition createBrdfLutDefinition(uint16_t size, uint32_t sampleCount)
        {
            // Two 16-bit half floats per pixel.
            std::vector<uint16_t> pixels(
                static_cast<size_t>(size) *
                static_cast<size_t>(size) *
                2);

            for(uint16_t y = 0; y < size; ++y)
            {
                // Y = perceptual roughness.
                const float roughness =
                    static_cast<float>(y) /
                    static_cast<float>(size - 1);

                for(uint16_t x = 0; x < size; ++x)
                {
                    // X = NdotV.
                    //
                    // Avoid exactly zero because several terms
                    // contain 1 / NdotV.
                    const float NoV =
                        std::max(
                            static_cast<float>(x) /
                                static_cast<float>(size - 1),
                            0.001f);

                    const glm::vec2 result =
                        integrateBrdf(
                            NoV,
                            roughness,
                            sampleCount);

                    const size_t index =
                        (static_cast<size_t>(y) * size +
                         static_cast<size_t>(x)) *
                        2;

                    pixels[index + 0] =
                        bx::halfFromFloat(result.x);

                    pixels[index + 1] =
                        bx::halfFromFloat(result.y);
                }
            }

            return createDefinition(std::move(pixels), size);
        }
    }

    void GeneratedTexturesFileImporter::TextureConfig::parse(const nlohmann::json& json) noexcept
    {
        auto itr = json.find("outputPath");
        if(itr != json.end())
        {
            outputPath = *itr;
        }
        itr = json.find("size");
        if(itr != json.end())
        {
            size = *itr;
        }
        itr = json.find("sampleCount");
        if(itr != json.end())
        {
            sampleCount = *itr;
        }
    }

    expected<GeneratedTexturesFileImporter::Effect, std::string> GeneratedTexturesFileImporter::prepare(const Input& input) noexcept
    {
        auto& json = input.config;
        Effect effect;
        auto itr = json.find("albedoLut");
        if(itr != json.end())
        {
            auto& file = _textures[TextureType::AlbedoLut];
            file = TextureConfig{"albedo_lut.bin", 512, 2048};
            file.parse(*itr);
            effect.outputs.emplace_back(file.outputPath, true);
        }
        itr = json.find("brdfLut");
        if(itr != json.end())
        {
            auto& file = _textures[TextureType::BrdfLut];
            file = TextureConfig{"brdf_lut.bin", 256, 1024};
            file.parse(*itr);
            effect.outputs.emplace_back(file.outputPath, true);
        }
        itr = json.find("outputFormat");
        _outputFormat = OutputFormat::Binary;
        if(itr != json.end())
        {
            std::string val{*itr};
            _outputFormat = protobuf::getFormat(val).value();
        }
        else if(!_textures.empty())
        {
            _outputFormat = protobuf::getPathFormat(_textures.begin()->second.outputPath);
        }
        return effect;
    }

    expected<void, std::string> GeneratedTexturesFileImporter::operator()(const Input& input, Config& config) noexcept
    {
        size_t i = 0;
        for (auto& [texType, texConfig] : _textures)
        {
            auto& out = *config.outputStreams[i++];
            Texture::Definition def;
            switch (texType)
            {
                case TextureType::AlbedoLut:
                {
                    def = PbrTextureUtils::createAlbedoLutDefinition(texConfig.size, texConfig.sampleCount);
                    break;
                }
                case TextureType::BrdfLut:
                {
                    def = PbrTextureUtils::createBrdfLutDefinition(texConfig.size, texConfig.sampleCount);
                    break;
                }
            }
            auto result = protobuf::write(def, out, _outputFormat);
            if(!result)
            {
                return result;
            }
        }
        return {};
    }

    const std::string& GeneratedTexturesFileImporter::getName() const noexcept
    {
        static const std::string name = "generated_textures";
        return name;
    }
    }
