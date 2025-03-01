#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/asset_core.hpp>
#include <filesystem>
#include <string>
#include <fstream>

#include <google/protobuf/message.h>
#include <google/protobuf/repeated_ptr_field.h>
#include <nlohmann/json.hpp>
#include <bgfx/bgfx.h>

namespace darmok
{
    enum class ProtobufFormat
    {
        Binary,
        Json
    };

    namespace ProtobufUtils
    {
        using Message = google::protobuf::Message;
        using FieldDescriptor = google::protobuf::FieldDescriptor;

        ProtobufFormat getFormat(const std::filesystem::path& path);
        std::optional<ProtobufFormat> getFormat(const std::string& name);

        const bgfx::Memory* copyMem(const std::string& data);
        const bgfx::Memory* refMem(const std::string& data);

        std::pair<std::ifstream, ProtobufFormat> createInputStream(const std::filesystem::path& path);
        std::ifstream createInputStream(const std::filesystem::path& path, ProtobufFormat format);
        expected<void, std::string> read(Message& msg, const std::filesystem::path& path);
        expected<void, std::string> read(Message& msg, std::istream& input, ProtobufFormat format);
        expected<void, std::string> readJson(Message& msg, std::istream& input);
        expected<void, std::string> readJson(Message& msg, const nlohmann::json& json);
        expected<void, std::string> readJson(Message& msg, const FieldDescriptor& field, const nlohmann::json& json);

        template<class T>
        expected<void, std::string> readStaticMem(Message& msg, const T& mem)
        {
            if (!mem.ParseFromArray(mem, sizeof(mem)))
            {
                return unexpected<std::string>{ "failed to parse from array" };
            }
            return {};
        }
        
        std::pair<std::ofstream, ProtobufFormat> createOutputStream(const std::filesystem::path& path);
        std::ofstream createOutputStream(const std::filesystem::path& path, ProtobufFormat format);
        expected<void, std::string> write(const Message& msg, const std::filesystem::path& path);
        expected<void, std::string> write(const Message& msg, std::ostream& output, ProtobufFormat format);
        expected<void, std::string> writeJson(const Message& msg, std::ostream& output);
        expected<void, std::string> writeJson(const Message& msg, nlohmann::json& json);
        expected<void, std::string> writeJson(const Message& msg, const FieldDescriptor& field, nlohmann::json& json);

    }

    template<typename Interface>
    class DARMOK_EXPORT ProtobufLoader final : public Interface
    {
    public:
        using Resource = Interface::Resource;

        ProtobufLoader(IDataLoader& dataLoader) noexcept
            : _dataLoader(dataLoader)
        {
        }

        ProtobufLoader(const ProtobufLoader& other) = delete;
        ProtobufLoader(ProtobufLoader&& other) = delete;

        std::shared_ptr<Resource> operator()(std::filesystem::path path) override
        {
            auto data = _dataLoader(path);
            auto format = ProtobufUtils::getFormat(path);
            auto res = std::make_shared<Resource>();
            DataInputStream input(data);
            ProtobufUtils::read(*res, input, format);
            return res;
        }
    private:
        IDataLoader& _dataLoader;
    };

    template<typename Loader>
    class DARMOK_EXPORT ProtobufFileImporter : public IFileTypeImporter
    {
    public:
        ProtobufFileImporter(Loader& loader, const std::string& name) noexcept
            : _loader(loader)
            , _outputFormat(ProtobufFormat::Binary)
        {
        }

        bool startImport(const Input& input, bool dry = false) override
        {
            if (input.config.is_null())
            {
                return false;
            }
            _outputPath = input.getOutputPath(".pb");
            std::optional<ProtobufFormat> outputFormat;
            if (auto jsonOutputFormat = input.getConfigField("outputFormat"))
            {
                outputFormat = ProtobufUtils::getFormat(jsonOutputFormat->get<std::string>());
            }
            if (outputFormat)
            {
                _outputFormat = *outputFormat;
            }
            else
            {
                _outputFormat = ProtobufUtils::getFormat(_outputPath);
            }
            return !_outputPath.empty();
        }

        virtual Outputs getOutputs(const Input& input) override
        {
            return { _outputPath };
        }

        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& outputPath) override
        {
            return ProtobufUtils::createOutputStream(outputPath, _outputFormat);
        }

        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override
        {
            auto obj = _loader(input.path);
            writeOutput(*obj, out);
        }

        void endImport(const Input& input) override
        {
            _outputPath.clear();
        }

        const std::string& getName() const noexcept
        {
            return _name;
        }

    protected:

        void writeOutput(const google::protobuf::Message& msg, std::ostream& out)
        {
            ProtobufUtils::write(msg, out, _outputFormat);
        }

    private:
        Loader& _loader;
        std::string _name;
        std::filesystem::path _outputPath;
        ProtobufFormat _outputFormat;
    };
}

void to_json(nlohmann::json& json, const google::protobuf::Message& msg);
void from_json(const nlohmann::json& json, google::protobuf::Message& msg);

template<class T>
bool operator==(const google::protobuf::RepeatedPtrField<T> proto, const std::unordered_set<T>& container)
{
    if (proto.size() != container.size())
    {
        return false;
    }
    for (auto& elm : container)
    {
        if (std::find(proto.begin(), proto.end(), elm) == proto.end())
        {
            return false;
        }
    }
    return true;
}