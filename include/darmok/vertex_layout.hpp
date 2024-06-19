#pragma once

#include <darmok/export.h>
#include <darmok/asset_core.hpp>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include <unordered_map>
#include <filesystem>

namespace darmok
{
    struct DARMOK_EXPORT VertexLayoutUtils final
    {
        static [[nodiscard]] bgfx::Attrib::Enum getBgfxAttrib(const std::string_view name) noexcept;
		static [[nodiscard]] bgfx::AttribType::Enum getBgfxAttribType(const std::string_view name) noexcept;
        static [[nodiscard]] std::string getBgfxAttribName(bgfx::Attrib::Enum val) noexcept;
        static [[nodiscard]] std::string getBgfxAttribTypeName(bgfx::AttribType::Enum val) noexcept;
        static void readFile(const std::filesystem::path& path, bgfx::VertexLayout& layout);
        static void writeFile(const std::filesystem::path& path, const bgfx::VertexLayout& layout);
		static void readJson(const nlohmann::ordered_json& json, bgfx::VertexLayout& layout);
        static void writeJson(nlohmann::ordered_json& json, const bgfx::VertexLayout& layout);
        static void readVaryingDef(std::istream& is, bgfx::VertexLayout& layout);
    private:
        static const std::unordered_map<std::string, bgfx::Attrib::Enum>& getVaryingDefAttrs() noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IVertexLayoutLoader
    {
    public:
        using result_type = bgfx::VertexLayout;
        virtual ~IVertexLayoutLoader() = default;
        virtual [[nodiscard]] result_type operator()(std::string_view name) = 0;
    };

    class IDataLoader;

    class DARMOK_EXPORT BinaryVertexLayoutLoader final : public IVertexLayoutLoader
    {
    public:
        BinaryVertexLayoutLoader(IDataLoader& dataLoader) noexcept;
        [[nodiscard]] bgfx::VertexLayout operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
    };

    enum class VertexLayoutProcessorOutputFormat
    {
        Binary,
        Json,
    };

    class DARMOK_EXPORT VertexLayoutProcessor final : public IAssetTypeProcessor
    {
    public:
        using OutputFormat = VertexLayoutProcessorOutputFormat;
        VertexLayoutProcessor() noexcept;
        VertexLayoutProcessor& setOutputFormat(OutputFormat format) noexcept;
        bgfx::VertexLayout read(const std::filesystem::path& path) const;

        bool getOutputs(const std::filesystem::path& input, std::vector<std::filesystem::path>& outputs) const override;
        std::ofstream createOutputStream(size_t outputIndex, const std::filesystem::path& path) const override;
        void writeOutput(const std::filesystem::path& input, size_t outputIndex, std::ostream& out) const override;
        std::string getName() const noexcept override;

    private:
        OutputFormat _outputFormat;

        static std::filesystem::path getFilename(const std::filesystem::path& path, OutputFormat format) noexcept;
    };
}

namespace bgfx
{
    // TODO: check if bgfx::read/write VertexLayout can be accessed
    // https://github.com/bkaradzic/bgfx/blob/master/src/vertexlayout.h#L39
    // instead of creating custom serialization

    template<typename Archive>
    void serialize(Archive& archive, VertexLayout& layout)
    {
        archive(
            layout.m_hash,
            layout.m_stride,
            layout.m_offset,
            layout.m_attributes
        );
    }
}

DARMOK_EXPORT std::string to_string(const bgfx::VertexLayout& layout) noexcept;
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const bgfx::VertexLayout& layout) noexcept;
