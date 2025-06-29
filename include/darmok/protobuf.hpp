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
    namespace protobuf
    {
        enum class Format
        {
            Binary,
            Json
        };

        using Message = google::protobuf::Message;
		using Descriptor = google::protobuf::Descriptor;
        using FieldDescriptor = google::protobuf::FieldDescriptor;

        [[nodiscard]] Format getFormat(const std::filesystem::path& path);
        [[nodiscard]] std::string_view getExtension(Format format);
        [[nodiscard]] std::optional<Format> getFormat(std::string_view name);
        [[nodiscard]] std::size_t getHash(const Message& msg);

        [[nodiscard]] const bgfx::Memory* copyMem(const std::string& data);
        [[nodiscard]] const bgfx::Memory* refMem(const std::string& data);

        [[nodiscard]] std::pair<std::ifstream, Format> createInputStream(const std::filesystem::path& path);
        [[nodiscard]] std::ifstream createInputStream(const std::filesystem::path& path, Format format);
        [[nodiscard]] expected<void, std::string> read(Message& msg, const std::filesystem::path& path);
        [[nodiscard]] expected<void, std::string> read(Message& msg, std::istream& input, Format format);
        [[nodiscard]] expected<void, std::string> readJson(Message& msg, std::istream& input);
        [[nodiscard]] expected<void, std::string> readJson(Message& msg, const nlohmann::json& json);
        [[nodiscard]] expected<void, std::string> readJson(Message& msg, const FieldDescriptor& field, const nlohmann::json& json);

        template<class T>
        [[nodiscard]] expected<void, std::string> readStaticMem(Message& msg, const T& mem)
        {
            if (!msg.ParseFromArray(mem, sizeof(T)))
            {
                return unexpected<std::string>{ "failed to parse from array" };
            }
            return {};
        }
        
        [[nodiscard]] std::pair<std::ofstream, Format> createOutputStream(const std::filesystem::path& path);
        [[nodiscard]] std::ofstream createOutputStream(const std::filesystem::path& path, Format format);
        [[nodiscard]] expected<void, std::string> write(const Message& msg, const std::filesystem::path& path);
        [[nodiscard]] expected<void, std::string> write(const Message& msg, std::ostream& output, Format format);
        [[nodiscard]] expected<void, std::string> writeJson(const Message& msg, std::ostream& output);
        [[nodiscard]] expected<void, std::string> writeJson(const Message& msg, nlohmann::json& json);
        [[nodiscard]] expected<void, std::string> writeJson(const Message& msg, const FieldDescriptor& field, nlohmann::json& json);

        [[nodiscard]] uint32_t getTypeId(const Message& msg);
        [[nodiscard]] uint32_t getTypeId(const Descriptor& desc);
        [[nodiscard]] std::string getFullName(const Message& msg);
        [[nodiscard]] std::string getTypeUrl(const Message& msg);
        [[nodiscard]] std::string getTypeUrl(const Descriptor& desc);
        [[nodiscard]] bool isAny(const Message& msg);

        template<class T>
        [[nodiscard]] uint32_t getTypeId()
        {
            return getTypeId(*T::descriptor());
        }

        template<class T>
        [[nodiscard]] std::string getTypeUrl()
        {
            return getTypeUrl(*T::descriptor());
        }
    }

    template<typename Interface>
    class DARMOK_EXPORT DataProtobufLoader final : public Interface
    {
    public:
        using Resource = Interface::Resource;
        using Result = Interface::Result;

        DataProtobufLoader(IDataLoader& dataLoader) noexcept
            : _dataLoader(dataLoader)
        {
        }

        DataProtobufLoader(const DataProtobufLoader& other) = delete;
        DataProtobufLoader(DataProtobufLoader&& other) = delete;

        Result operator()(std::filesystem::path path) override
        {
            auto dataResult = _dataLoader(path);
			if (!dataResult)
			{
				return unexpected<std::string>{ dataResult.error() };
			}
            auto format = protobuf::getFormat(path);
            auto res = std::make_shared<Resource>();
            DataInputStream input{dataResult.value()};
            auto readResult = protobuf::read(*res, input, format);
			if (!readResult)
			{
				return unexpected<std::string>{ readResult.error() };
			}
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
            , _outputFormat(protobuf::Format::Binary)
        {
        }

        bool startImport(const Input& input, bool dry = false) override
        {
            if (input.config.is_null())
            {
                return false;
            }
            _outputPath = input.getOutputPath(".pb");
            std::optional<protobuf::Format> outputFormat;
            if (auto jsonOutputFormat = input.getConfigField("outputFormat"))
            {
                outputFormat = protobuf::getFormat(jsonOutputFormat->get<std::string_view>());
            }
            if (outputFormat)
            {
                _outputFormat = *outputFormat;
            }
            else
            {
                _outputFormat = protobuf::getFormat(_outputPath);
            }
            return !_outputPath.empty();
        }

        virtual Outputs getOutputs(const Input& input) override
        {
            return { _outputPath };
        }

        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& outputPath) override
        {
            return protobuf::createOutputStream(outputPath, _outputFormat);
        }

        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override
        {
            auto objResult = _loader(input.path);
            if (!objResult)
            {
                throw std::runtime_error(objResult.error());
            }
            writeOutput(**objResult, out);
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
            auto result = protobuf::write(msg, out, _outputFormat);
			if (!result)
			{
				throw std::runtime_error(result.error());
			}
        }

    private:
        Loader& _loader;
        std::string _name;
        std::filesystem::path _outputPath;
        protobuf::Format _outputFormat;
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