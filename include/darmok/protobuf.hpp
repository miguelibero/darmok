#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/scene_fwd.hpp>
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

        [[nodiscard]] Format getPathFormat(const std::filesystem::path& path);
        [[nodiscard]] std::string_view getExtension(Format format = Format::Binary);
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

        [[nodiscard]] IdType getTypeId(const Message& msg);
        [[nodiscard]] IdType getTypeId(const Descriptor& desc);
        [[nodiscard]] std::string getFullName(const Message& msg);
        [[nodiscard]] std::string getTypeUrl(const Message& msg);
        [[nodiscard]] std::string getTypeUrl(const Descriptor& desc);
        [[nodiscard]] bool isAny(const Message& msg);
        [[nodiscard]] std::vector<std::string> getEnumValues(const google::protobuf::EnumDescriptor& enumDesc);

        template<class T>
        [[nodiscard]] IdType getTypeId()
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
            : _dataLoader{ dataLoader }
        {
        }

        Result operator()(std::filesystem::path path) noexcept override
        {
            auto dataResult = (*_dataLoader)(path);
			if (!dataResult)
			{
				return unexpected<std::string>{ dataResult.error() };
			}
            auto format = protobuf::getPathFormat(path);
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
        OptionalRef<IDataLoader> _dataLoader;
    };

    template<typename Loader>
    class DARMOK_EXPORT ProtobufFileImporter : public IFileTypeImporter
    {
    public:
        ProtobufFileImporter(Loader& loader, const std::string& name) noexcept
            : _loader{ loader }
            , _outputFormat{ protobuf::Format::Binary }
        {
        }

        expected<Effect, std::string> prepare(const Input& input) noexcept override
        {
            Effect effect;
            if (input.config.is_null())
            {
                return effect;
            }

            auto outputPath = input.getOutputPath(".pb");
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
                _outputFormat = protobuf::getPathFormat(_outputPath);
            }
            auto binary = _outputFormat == protobuf::Format::Binary;
            effect.outputs.emplace_back(outputPath, binary);
            return effect;
        }

        expected<void, std::string> operator()(const Input& input, Config& config) noexcept override
        {
            auto loadResult = _loader(input.path);
            if (!loadResult)
            {
                return unexpected{ loadResult.error() };
            }
            auto& msg = loadResult.value();
            if (!msg)
            {
                return unexpected{ "empty protobuf message" };
            }
            for (auto& out : config.outputStreams)
            {
                if (!out)
                {
                    continue;
                }
                auto result = protobuf::write(*msg, *out, _outputFormat);
                if (!result)
                {
                    return unexpected{ result.error() };
                }
            }
            return {};
        }

        const std::string& getName() const noexcept override
        {
            return _name;
        }

    private:
        Loader& _loader;
        std::string _name;
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
