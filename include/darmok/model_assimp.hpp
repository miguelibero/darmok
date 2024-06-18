#pragma once

#include <darmok/export.h>
#include <darmok/model.hpp>
#include <darmok/optional_ref.hpp>
#include <memory>
#include <string>

namespace bx
{
    struct AllocatorI;
}

namespace bgfx
{
    struct VertexLayout;
}

namespace darmok
{
    class IDataLoader;
    class IImageLoader;
    class AssimpModelLoaderImpl;

    class DARMOK_EXPORT AssimpModelLoader final : public IModelLoader
    {
    public:
        AssimpModelLoader(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader = nullptr) noexcept;
        ~AssimpModelLoader() noexcept;
        void setVertexLayout(const bgfx::VertexLayout& vertexLayout) noexcept;
		result_type operator()(std::string_view name) override;
    private:
        std::unique_ptr<AssimpModelLoaderImpl> _impl;
    };

    class DARMOK_EXPORT AssimpModelProcessor final
    {
    public:
        AssimpModelProcessor(const std::string& inputPath);
        AssimpModelProcessor& setConfigFromFile(const std::string& path);
        AssimpModelProcessor& setConfig(const nlohmann::ordered_json& config);
        AssimpModelProcessor& setHeaderVarName(const std::string& name) noexcept;
        std::string to_string() const noexcept;
        void writeFile(const std::string& outputPath);
    private:
        static const char* _vertexLayoutJsonKey;
        static const char* _embedTexturesJsonKey;

        std::string _inputPath;
        bgfx::VertexLayout _vertexLayout;
        bool _embedTextures;
        std::string _headerVarName;
        mutable std::shared_ptr<Model> _model;

        std::shared_ptr<Model> loadModel() const;
        bgfx::VertexLayout loadVertexLayout(const nlohmann::ordered_json& json);
        void writeHeader(std::ostream& os, const std::string varName) const;
    };
}

DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::AssimpModelProcessor& process) noexcept;