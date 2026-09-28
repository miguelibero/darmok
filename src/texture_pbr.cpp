#include <darmok/texture_pbr.hpp>
#include <darmok/multiarray.hpp>
#include <fmt/format.h>
#include <magic_enum/magic_enum_format.hpp>

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

        void buildBasis(
            glm::vec3 N,
            glm::vec3& T,
            glm::vec3& B)
        {
            const glm::vec3 up =
                std::abs(N.y) < 0.999f
                    ? glm::vec3(0, 1, 0)
                    : glm::vec3(1, 0, 0);

            T = glm::normalize(glm::cross(up, N));
            B = glm::cross(N, T);
        }

        glm::vec3 importanceSampleGgx(
            glm::vec2 Xi,
            float alpha,
            glm::vec3 N = glm::vec3(0.0f, 0.0f, 1.0f)
        )
        {
            const float alpha2 =
                alpha * alpha;

            const float phi =
                2.0f *
                glm::pi<float>() *
                Xi.x;

            const float cosTheta =
                std::sqrt(
                    (1.0f - Xi.y) /
                    (1.0f +
                     (alpha2 - 1.0f) * Xi.y));

            const float sinTheta =
                std::sqrt(
                    std::max(
                        0.0f,
                        1.0f -
                            cosTheta * cosTheta));

            glm::vec3 H{
                std::cos(phi) * sinTheta,
                std::sin(phi) * sinTheta,
                cosTheta};

            glm::vec3 T;
            glm::vec3 B;

            buildBasis(N, T, B);

            return glm::normalize(
                T * H.x +
                B * H.y +
                N * H.z);
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

                const float alpha2 =
                    alpha * alpha;

                const float denominator =
                    NoH * NoH * (alpha2 - 1.0f) +
                    1.0f;

                const float D =
                    alpha2 /
                    (glm::pi<float>() * denominator * denominator);

                const float Vterm =
                    smithGgxCorrelated(
                        NoV,
                        NoL,
                        alpha);

                const float F =
                    fresnelSchlick(
                        VoH,
                        F0);

                const float pdf =
                    D * NoH /
                    std::max(
                        4.0f * VoH,
                        1e-6f);

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
            float alpha)
        {
            const float k =
                alpha / 2.0f;

            return NoV /
                   (NoV * (1.0f - k) + k);
        }

        glm::vec3 cosineSampleHemisphere(
            glm::vec2 xi)
        {
            const float phi =
                2.0f * glm::pi<float>() * xi.x;

            const float cosTheta =
                std::sqrt(1.0f - xi.y);

            const float sinTheta =
                std::sqrt(xi.y);

            return {
                std::cos(phi) * sinTheta,
                std::sin(phi) * sinTheta,
                cosTheta};
        }

        glm::vec2 directionToEquirectangularUv(glm::vec3 dir)
        {
            dir = glm::normalize(dir);

            const float u =
                std::atan2(dir.z, dir.x) /
                    (2.0f * glm::pi<float>()) +
                0.5f;

            const float v =
                std::asin(glm::clamp(dir.y, -1.0f, 1.0f)) /
                    glm::pi<float>() +
                0.5f;

            return {u, v};
        }

        using PixelArray2d = Array2d<glm::vec4, uint32_t>;
        using PixelArray3d = Array3d<glm::vec4, uint32_t>;

        glm::vec3 sampleEquirectangular(
            const PixelArray2d& pixels,
            glm::vec3 direction)
        {
            glm::vec2 uv =
                directionToEquirectangularUv(direction);

            uv.x -= std::floor(uv.x);

            uv.y = glm::clamp(uv.y, 0.0f, 1.0f);

            const float x =
                uv.x * static_cast<float>(pixels.size().x - 1);

            const float y =
                uv.y * static_cast<float>(pixels.size().y - 1);

            const uint32_t x0 =
                static_cast<uint32_t>(x);

            const uint32_t y0 =
                static_cast<uint32_t>(y);

            const uint32_t x1 =
                (x0 + 1) % pixels.size().x;

            const uint32_t y1 =
                std::min(y0 + 1, pixels.size().y - 1);

            const float tx = x - static_cast<float>(x0);
            const float ty = y - static_cast<float>(y0);

            const auto c00 = pixels(x0, y0);
            const auto c10 = pixels(x1, y0);
            const auto c01 = pixels(x0, y1);
            const auto c11 = pixels(x1, y1);

            return glm::mix(
                glm::mix(c00, c10, tx),
                glm::mix(c01, c11, tx),
                ty);
        }

        glm::vec3 integrateIrradiance(
            const PixelArray2d& hdr,
            glm::vec3 N,
            uint32_t sampleCount)
        {
            glm::vec3 T;
            glm::vec3 B;

            buildBasis(N, T, B);

            glm::vec3 result{0.0f};

            for(uint32_t i = 0; i < sampleCount; ++i)
            {
                const glm::vec2 xi{
                    static_cast<float>(i) /
                        static_cast<float>(sampleCount),

                    radicalInverseVdC(i)};

                const glm::vec3 local =
                    cosineSampleHemisphere(xi);

                const glm::vec3 L =
                    glm::normalize(
                        T * local.x +
                        B * local.y +
                        N * local.z);

                const float NoL =
                    std::max(glm::dot(N, L), 0.0f);

                result +=
                    sampleEquirectangular(
                        hdr,
                        L) *
                    NoL;
            }

            return result *
                   (glm::pi<float>() /
                    static_cast<float>(sampleCount));
        }

        float geometrySmith(
            float NoV,
            float NoL,
            float alpha)
        {
            return geometrySchlickGgx(NoV, alpha) *
                   geometrySchlickGgx(NoL, alpha);
        }

        glm::vec2 integrateBrdf(
            float NoV,
            float alpha,
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

            float A = 0.0f;
            float B = 0.0f;

            for(uint32_t i = 0; i < sampleCount; ++i)
            {
                const glm::vec2 Xi =
                    hammersley(i, sampleCount);

                const glm::vec3 H =
                    importanceSampleGgx(
                        Xi,
                        alpha);

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
                        alpha);

                const float GVis =
                    (G * VoH) /
                    std::max(
                        NoH * NoV,
                        1e-6f);

                const float Fc =
                    std::pow(
                        1.0f - VoH,
                        5.0f);

                A += (1.0f - Fc) * GVis;
                B += Fc * GVis;
            }

            const float invSampleCount =
                1.0f / static_cast<float>(sampleCount);

            return {
                A * invSampleCount,
                B * invSampleCount};
        }        

        glm::vec3 integratePrefilteredEnvironment(
            const PixelArray2d& hdr,
            glm::vec3 R,
            float alpha,
            uint32_t sampleCount)
        {
            glm::vec3 result{0.0f};
            float totalWeight = 0.0f;

            for(uint32_t i = 0; i < sampleCount; ++i)
            {
                const glm::vec2 Xi{
                    static_cast<float>(i) /
                        static_cast<float>(sampleCount),

                    radicalInverseVdC(i)};

                const glm::vec3 H =
                    importanceSampleGgx(
                        Xi,
                        alpha, R);

                const glm::vec3 L =
                    glm::normalize(
                        2.0f *
                            glm::dot(R, H) *
                            H -
                        R);

                const float NoL =
                    std::max(
                        glm::dot(R, L),
                        0.0f);

                if(NoL <= 0.0f)
                    continue;

                result +=
                    sampleEquirectangular(
                        hdr,
                        L) *
                    NoL;

                totalWeight += NoL;
            }

            return totalWeight > 0.0f
                       ? result / totalWeight
                       : glm::vec3(0.0f);
        }

        // RG16F = 2 half-floats per pixel. Use uint32_t for the length type so
        // totalSize doesn't overflow (256*256=65536 > uint16_t max).
        using HalfFloatPixelArray2d = Array2d<glm::vec<2, uint16_t>, uint32_t>;

        Definition createHalfFloatDefinition(const HalfFloatPixelArray2d& pixels, uint16_t size)
        {
            Definition def;
            auto& config = *def.mutable_config();

            def.set_data(pixels.dataView().stringView());
            config.mutable_size()->set_x(size);
            config.mutable_size()->set_y(size);
            config.set_format(Texture::Definition::RG16F);
            config.set_type(Texture::Definition::Texture2D);
            config.set_mips(false);
            config.set_depth(0);
            config.set_layers(1);

            return def;
        }


        Definition createAlbedoLutDefinition(uint16_t size, uint32_t sampleCount)
        {
            HalfFloatPixelArray2d pixels{size};

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

                    pixels(x, y) = {
                        bx::halfFromFloat(E1),
                        bx::halfFromFloat(E0)
                    };
                }
            }

            return createHalfFloatDefinition(std::move(pixels), size);
        }

        Definition createBrdfLutDefinition(uint16_t size, uint32_t sampleCount)
        {
            HalfFloatPixelArray2d pixels{size};

            for(uint16_t y = 0; y < size; ++y)
            {
                // Y = perceptual roughness.
                const float roughness =
                    static_cast<float>(y) /
                    static_cast<float>(size - 1);

                const float alpha = roughness * roughness;

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
                            alpha,
                            sampleCount);

                    pixels(x, y) = {
                        bx::halfFromFloat(result.x),
                        bx::halfFromFloat(result.y)
                    };
                }
            }

            return createHalfFloatDefinition(std::move(pixels), size);
        }

        expected<PixelArray2d, std::string> loadMipData(const bimg::ImageMip& mip)
        {
            if (mip.m_format != bimg::TextureFormat::RGBA32F)
            {
                return unexpected{"format is not RGBA32F"};
            }
            return PixelArray2d::load(DataView{mip.m_data, mip.m_size}, {mip.m_width, mip.m_height});
        }

        glm::vec3 cubeDirection(
            uint32_t face,
            uint32_t x,
            uint32_t y,
            uint32_t size)
        {
            float a = 2.0f * (float(x) + 0.5f) / float(size) - 1.0f;
            float b = 2.0f * (float(y) + 0.5f) / float(size) - 1.0f;

            switch(face)
            {
            case 0:
                return glm::normalize(glm::vec3(1.0f, -b, -a)); // +X
            case 1:
                return glm::normalize(glm::vec3(-1.0f, -b, a)); // -X
            case 2:
                return glm::normalize(glm::vec3(a, 1.0f, b)); // +Y
            case 3:
                return glm::normalize(glm::vec3(a, -1.0f, -b)); // -Y
            case 4:
                return glm::normalize(glm::vec3(a, -b, 1.0f)); // +Z
            case 5:
                return glm::normalize(glm::vec3(-a, -b, -1.0f)); // -Z
            default:
                return {};
            }
        }

        Definition createEnvironmentIrradianceDefinition(const PixelArray2d& source, uint32_t size, uint32_t sampleCount)
        {
            // 6 faces × width × height × RGBA
            PixelArray3d result{6, size, size};

            for(uint32_t face = 0; face < 6; ++face)
            {
                for(uint32_t y = 0; y < size; ++y)
                {
                    for(uint32_t x = 0; x < size; ++x)
                    {
                        glm::vec3 N =
                            cubeDirection(face, x, y, size);

                        glm::vec3 irradiance =
                            integrateIrradiance(
                                source,
                                N,
                                sampleCount);

                        result[face][x][y] = glm::vec4{irradiance, 1.0f};
                    }
                }
            }

            Definition def;
            auto& config = *def.mutable_config();

            def.set_data(result.dataView().stringView());
            config.mutable_size()->set_x(size);
            config.mutable_size()->set_y(size);
            config.set_format(Texture::Definition::RGBA32F);
            config.set_type(Texture::Definition::CubeMap);
            config.set_mips(false);
            config.set_depth(0);
            config.set_layers(1);

            return def;
        }

        Definition createEnvironmentPrefilteredDefinition(const PixelArray2d& source, uint16_t size, uint32_t sampleCount)
        {
            // mipCount x 6 faces × width × height × RGBA
            auto mipCount = Image::getMipCountForSize(size);
            std::vector<PixelArray2d::value_type> result;

            for(uint32_t mipNum = 0; mipNum < mipCount; ++mipNum)
            {
                uint32_t mipSize = size >> mipNum;
                mipSize = std::max(uint32_t{1}, mipSize);
                PixelArray3d mip{6, mipSize, mipSize};

                float roughness =
                    mipCount > 1
                        ? float(mipNum) / float(mipCount - 1)
                        : 0.0f;

                auto alpha = roughness * roughness;

                for(uint32_t face = 0; face < 6; ++face)
                {
                    for(uint32_t y = 0; y < mipSize; ++y)
                    {
                        for(uint32_t x = 0; x < mipSize; ++x)
                        {
                            glm::vec3 N =
                                cubeDirection(
                                    face,
                                    x,
                                    y,
                                    mipSize);

                            glm::vec3 prefilteredColor(0.0f);
                            float totalWeight = 0.0f;

                            for(uint32_t i = 0;
                                i < sampleCount;
                                ++i)
                            {
                                glm::vec2 Xi;

                                Xi.x =
                                    float(i) /
                                    float(sampleCount);

                                Xi.y =
                                    radicalInverseVdC(i);

                                glm::vec3 H =
                                    importanceSampleGgx(
                                        Xi,
                                        alpha,
                                        N);

                                glm::vec3 L =
                                    glm::normalize(
                                        glm::reflect(-N, H));

                                float NoL =
                                    glm::max(
                                        glm::dot(N, L),
                                        0.0f);

                                if(NoL > 0.0f)
                                {
                                    glm::vec3 radiance =
                                        sampleEquirectangular(
                                            source,
                                            L);

                                    prefilteredColor +=
                                        radiance * NoL;

                                    totalWeight += NoL;
                                }
                            }

                            if(totalWeight > 0.0f)
                            {
                                prefilteredColor /= totalWeight;
                            }

                            mip[face][x][y] = glm::vec4(prefilteredColor, 1.0f);
                        }
                    }
                }
                result.insert(result.end(),
                    std::make_move_iterator(mip.begin()),
                    std::make_move_iterator(mip.end())
                );
            }

            Definition def;
            auto& config = *def.mutable_config();

            DataView dataView{result.data(), result.size() * sizeof(PixelArray2d::value_type)};

            def.set_data(dataView.stringView());
            config.mutable_size()->set_x(size);
            config.mutable_size()->set_y(size);
            config.set_format(Texture::Definition::RGBA32F);
            config.set_type(Texture::Definition::CubeMap);
            config.set_mips(true);
            config.set_depth(0);
            config.set_layers(1);

            return def;
        }
    }

    void GeneratedTextureConfig::parse(const nlohmann::json& json) noexcept
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
        itr = json.find("samples");
        if(itr != json.end())
        {
            samples = *itr;
        }
    }

    expected<GeneratedTexturesFileImporter::Effect, std::string> GeneratedTexturesFileImporter::prepare(const Input& input) noexcept
    {
        auto& json = input.config;
        Effect effect;
        if(!std::filesystem::is_directory(input.path))
        {
            return effect;
        }
        if(json.is_null())
        {
            return effect;
        }
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
            if (config.outputStreams.size() <= i || !config.outputStreams[i])
            {
                ++i;
                continue;
            }
            auto& out = *config.outputStreams[i++];
            Texture::Definition def;
            switch (texType)
            {
                case TextureType::AlbedoLut:
                {
                    def = PbrTextureUtils::createAlbedoLutDefinition(texConfig.size, texConfig.samples);
                    break;
                }
                case TextureType::BrdfLut:
                {
                    def = PbrTextureUtils::createBrdfLutDefinition(texConfig.size, texConfig.samples);
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

    EnvironmentTextureFileImporter::EnvironmentTextureFileImporter(OptionalRef<bx::AllocatorI> alloc) noexcept 
        : _alloc{alloc}
    {
    }

    expected<EnvironmentTextureFileImporter::Effect, std::string> EnvironmentTextureFileImporter::prepare(const Input& input) noexcept
    {
        Effect effect;
        auto& json = input.config;
        if(json.is_null())
        {
            return effect;
        }

        auto addOutput = [&](std::filesystem::path path)
        {
            auto str = path.string();
            StringUtils::replace(str, "{name}", input.path.stem().string());
            effect.outputs.emplace_back(str, true);
        };

        auto itr = json.find("irradiance");
        if(itr != json.end())
        {
            auto& file = _textures[TextureType::Irradiance];
            file = TextureConfig{"{name}_env_irradiance.bin", 32, 256};
            file.parse(*itr);
            addOutput(file.outputPath);
        }
        itr = json.find("prefiltered");
        if(itr != json.end())
        {
            auto& file = _textures[TextureType::Prefiltered];
            file = TextureConfig{"{name}_env_prefiltered.bin", 128, 1024};
            file.parse(*itr);
            addOutput(file.outputPath);
        }
        _outputFormat = OutputFormat::Binary;
        itr = json.find("outputFormat");
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

    expected<void, std::string> EnvironmentTextureFileImporter::operator()(const Input& input, Config& config) noexcept
    {
        using namespace PbrTextureUtils;

        auto& alloc = _alloc ? *_alloc : _defaultAlloc;
        auto dataResult = Data::fromFile(input.path, alloc);
        if(!dataResult)
        {
            return unexpected{fmt::format("Failed to read file: {}", dataResult.error())};
        }
        auto imgResult = Image::load(dataResult.value(), alloc);
        if (!imgResult)
        {
            return unexpected{fmt::format("Failed to load image: {}", imgResult.error())};
        }
        auto convertResult = imgResult->convertFormat(bimg::TextureFormat::RGBA32F);
        if(!convertResult)
        {
            return unexpected{fmt::format("Failed to convert image: {}", convertResult.error())};
        }
        auto mipResult = convertResult->getMip(0, 0);
        if(!mipResult)
        {
            return unexpected{fmt::format("Failed to get image mip: {}", mipResult.error())};
        }
        auto arrayResult = loadMipData(mipResult.value());
        if(!arrayResult)
        {
            return unexpected{fmt::format("Failed to convert image mip to array: {}", arrayResult.error())};
        }

        auto pixels = std::move(arrayResult).value();

        size_t i = 0;
        for (auto& [texType, texConfig] : _textures)
        {
            if (config.outputStreams.size() <= i || !config.outputStreams[i])
            {
                ++i;
                continue;
            }
            auto& out = *config.outputStreams[i++];
            Texture::Definition def;
            switch(texType)
            {
                case TextureType::Irradiance:
                {
                    def = createEnvironmentIrradianceDefinition(pixels, texConfig.size, texConfig.samples);
                    break;
                }
                case TextureType::Prefiltered:
                {
                    def = createEnvironmentPrefilteredDefinition(pixels, texConfig.size, texConfig.samples);
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

    const std::string& EnvironmentTextureFileImporter::getName() const noexcept
    {
        static const std::string name = "env_texture";
        return name;
    }
}
