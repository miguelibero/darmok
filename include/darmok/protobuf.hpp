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
#include <magic_enum/magic_enum.hpp>

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

        [[nodiscard]] Format getPathFormat(const std::filesystem::path& path) noexcept;
        [[nodiscard]] std::string_view getExtension(Format format = Format::Binary) noexcept;
        [[nodiscard]] std::optional<Format> getFormat(std::string_view name) noexcept;
        [[nodiscard]] std::size_t getHash(const Message& msg) noexcept;

        [[nodiscard]] const bgfx::Memory* copyMem(const std::string& data) noexcept;
        [[nodiscard]] const bgfx::Memory* refMem(const std::string& data) noexcept;

        [[nodiscard]] std::pair<std::ifstream, Format> createInputStream(const std::filesystem::path& path) noexcept;
        [[nodiscard]] std::ifstream createInputStream(const std::filesystem::path& path, Format format) noexcept;
        [[nodiscard]] expected<void, std::string> read(Message& msg, const std::filesystem::path& path) noexcept;
        [[nodiscard]] expected<void, std::string> read(Message& msg, std::istream& input, Format format) noexcept;
        [[nodiscard]] expected<void, std::string> readJson(Message& msg, std::istream& input) noexcept;
        [[nodiscard]] expected<void, std::string> readJson(Message& msg, const nlohmann::json& json) noexcept;
        [[nodiscard]] expected<void, std::string> readJson(Message& msg, const FieldDescriptor& field, const nlohmann::json& json) noexcept;

        template<class T>
        [[nodiscard]] expected<void, std::string> readStaticMem(Message& msg, const T& mem) noexcept
        {
            if (!msg.ParseFromArray(mem, sizeof(T)))
            {
                return unexpected<std::string>{ "failed to parse from array" };
            }
            return {};
        }
        
        [[nodiscard]] std::pair<std::ofstream, Format> createOutputStream(const std::filesystem::path& path) noexcept;
        [[nodiscard]] std::ofstream createOutputStream(const std::filesystem::path& path, Format format) noexcept;
        [[nodiscard]] expected<void, std::string> write(const Message& msg, const std::filesystem::path& path) noexcept;
        [[nodiscard]] expected<void, std::string> write(const Message& msg, std::ostream& output, Format format) noexcept;
        [[nodiscard]] expected<void, std::string> writeJson(const Message& msg, std::ostream& output) noexcept;
        [[nodiscard]] expected<void, std::string> writeJson(const Message& msg, nlohmann::json& json) noexcept;
        [[nodiscard]] expected<void, std::string> writeJson(const Message& msg, const FieldDescriptor& field, nlohmann::json& json) noexcept;

        [[nodiscard]] IdType getTypeId(const Message& msg) noexcept;
        [[nodiscard]] IdType getTypeId(const Descriptor& desc) noexcept;
        [[nodiscard]] std::string getFullName(const Message& msg) noexcept;
        [[nodiscard]] std::string getTypeUrl(const Message& msg) noexcept;
        [[nodiscard]] std::string getTypeUrl(const Descriptor& desc) noexcept;
        [[nodiscard]] bool isAny(const Message& msg) noexcept;
        [[nodiscard]] std::vector<std::string> getEnumValues(const google::protobuf::EnumDescriptor& enumDesc) noexcept;

        template<class T>
        [[nodiscard]] IdType getTypeId() noexcept
        {
            return getTypeId(*T::descriptor());
        }

        template<class T>
        [[nodiscard]] std::string getTypeUrl() noexcept
        {
            return getTypeUrl(*T::descriptor());
        }

        template<typename T>
        struct FieldConverter final
        {
        };

        template <typename A, typename B>
        concept IsFieldConvertible = std::is_same_v<A, B>;

        template<typename T>
            requires IsFieldConvertible<T, int32_t>
        struct FieldConverter<T>
        {
            const google::protobuf::FieldDescriptor& field;

            FieldConverter(const google::protobuf::FieldDescriptor& field)
                : field{ field }
            {
            }

            static bool check(google::protobuf::FieldDescriptor::CppType type)
            {
                return type == FieldDescriptor::CPPTYPE_INT32 || type == FieldDescriptor::CPPTYPE_ENUM;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept
            {
                return msg.GetReflection()->GetInt32(msg, &field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept
            {
                return msg.GetReflection()->GetRepeatedInt32(msg, &field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->SetInt32(&msg, &field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->AddInt32(&msg, &field, val);
            }
        };

        template<typename T>
            requires IsFieldConvertible<T, int64_t>
        struct FieldConverter<T>
        {
            const google::protobuf::FieldDescriptor& field;

            FieldConverter(const google::protobuf::FieldDescriptor& field)
                : field{ field }
            {
            }

            static bool check(google::protobuf::FieldDescriptor::CppType type)
            {
                return type == FieldDescriptor::CPPTYPE_INT64;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept
            {
                return msg.GetReflection()->GetInt64(msg, &field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept
            {
                return msg.GetReflection()->GetRepeatedInt64(msg, &field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->SetInt64(&msg, &field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->AddInt64(&msg, &field, val);
            }
        };

        template<typename T>
            requires IsFieldConvertible<T, uint32_t>
        struct FieldConverter<T>
        {
            const google::protobuf::FieldDescriptor& field;

            FieldConverter(const google::protobuf::FieldDescriptor& field)
                : field{ field }
            {
            }

            static bool check(google::protobuf::FieldDescriptor::CppType type)
            {
                return type == FieldDescriptor::CPPTYPE_UINT32;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept
            {
                return msg.GetReflection()->GetUInt32(msg, &field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept
            {
                return msg.GetReflection()->GetRepeatedUInt32(msg, &field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->SetUInt32(&msg, &field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->AddUInt32(&msg, &field, val);
            }
        };

        template<typename T>
            requires IsFieldConvertible<T, uint64_t>
        struct FieldConverter<T>
        {
            const google::protobuf::FieldDescriptor& field;

            FieldConverter(const google::protobuf::FieldDescriptor& field)
                : field{ field }
            {
            }

            static bool check(google::protobuf::FieldDescriptor::CppType type)
            {
                return type == FieldDescriptor::CPPTYPE_UINT64;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept
            {
                return msg.GetReflection()->GetUInt64(msg, &field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept
            {
                return msg.GetReflection()->GetRepeatedUInt64(msg, &field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->SetUInt64(&msg, &field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->AddUInt64(&msg, &field, val);
            }
        };

        template<typename T>
            requires IsFieldConvertible<T, float>
        struct FieldConverter<T>
        {
            const google::protobuf::FieldDescriptor& field;

            FieldConverter(const google::protobuf::FieldDescriptor& field)
                : field{ field }
            {
            }

            static bool check(google::protobuf::FieldDescriptor::CppType type)
            {
                return type == FieldDescriptor::CPPTYPE_FLOAT;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept
            {
                return msg.GetReflection()->GetFloat(msg, &field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept
            {
                return msg.GetReflection()->GetRepeatedFloat(msg, &field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->SetFloat(&msg, &field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->AddFloat(&msg, &field, val);
            }
        };

        template<typename T>
            requires IsFieldConvertible<T, double>
        struct FieldConverter<T>
        {
            const google::protobuf::FieldDescriptor& field;

            FieldConverter(const google::protobuf::FieldDescriptor& field)
                : field{ field }
            {
            }

            static bool check(google::protobuf::FieldDescriptor::CppType type)
            {
                return type == FieldDescriptor::CPPTYPE_DOUBLE;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept
            {
                return msg.GetReflection()->GetDouble(msg, &field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept
            {
                return msg.GetReflection()->GetRepeatedDouble(msg, &field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->SetDouble(&msg, &field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->AddDouble(&msg, &field, val);
            }
        };

        template<typename T>
            requires IsFieldConvertible<T, bool>
        struct FieldConverter<T>
        {
            const google::protobuf::FieldDescriptor& field;

            FieldConverter(const google::protobuf::FieldDescriptor& field)
                : field{ field }
            {
            }

            static bool check(google::protobuf::FieldDescriptor::CppType type)
            {
                return type == FieldDescriptor::CPPTYPE_BOOL;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept
            {
                return msg.GetReflection()->GetBool(msg, &field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept
            {
                return msg.GetReflection()->GetRepeatedBool(msg, &field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->SetBool(&msg, &field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->AddBool(&msg, &field, val);
            }
        };

        template<typename T>
            requires IsFieldConvertible<T, std::string>
        struct FieldConverter<T>
        {
            const google::protobuf::FieldDescriptor& field;

            FieldConverter(const google::protobuf::FieldDescriptor& field)
                : field{ field }
            {
            }

            static bool check(google::protobuf::FieldDescriptor::CppType type)
            {
                return type == FieldDescriptor::CPPTYPE_STRING;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept
            {
                return msg.GetReflection()->GetString(msg, &field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept
            {
                return msg.GetReflection()->GetRepeatedString(msg, &field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->SetString(&msg, &field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept
            {
                msg.GetReflection()->AddString(&msg, &field, val);
            }
        };

        template<typename T>
            requires std::is_enum_v<T>
        struct FieldConverter<T>
        {
            const google::protobuf::FieldDescriptor& field;

            FieldConverter(const google::protobuf::FieldDescriptor& field)
                : field{ field }
            {
            }

            static bool check(google::protobuf::FieldDescriptor::CppType type)
            {
                return type == FieldDescriptor::CPPTYPE_ENUM;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept
            {
                return msg.GetReflection()->GetEnum(msg, &field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept
            {
                return msg.GetReflection()->GetRepeatedEnum(msg, &field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept
            {
                auto enumVal = field.enum_type()->FindValueByNumber(val);
                msg.GetReflection()->SetEnum(&msg, &field, enumVal);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept
            {
                auto enumVal = field.enum_type()->FindValueByNumber(val);
                msg.GetReflection()->AddEnum(&msg, &field, enumVal);
            }
        };

        template<typename T>
        [[nodiscard]] expected<FieldConverter<T>, std::string> createFieldConverter(const google::protobuf::FieldDescriptor& field) noexcept
        {
            if(!FieldConverter<T>::check(field.cpp_type()))
            {
                return unexpected<std::string>{"cannot convert type"};
            }
            return FieldConverter<T>{ field };
        }

        template<typename T>
        [[nodiscard]] expected<std::vector<T>, std::string> toVector(const google::protobuf::Message& msg, const google::protobuf::FieldDescriptor& field) noexcept
        {
            using namespace google::protobuf;
            if (!field.is_repeated())
            {
                return unexpected<std::string>{"field is not repeated"};
            }
            auto converterResult = createFieldConverter<T>(field);
            if (!converterResult)
            {
                return unexpected{ std::move(converterResult).error() };
            }
            auto& converter = converterResult.value();

            std::vector<T> vec;
            auto size = msg.GetReflection()->FieldSize(msg, &field);
            vec.reserve(size);

            for (int i = 0; i < size; ++i)
            {
                vec.push_back(converter.get(msg, i));
            }
            return vec;
        }

        template<typename T>
        [[nodiscard]] expected<void, std::string> fromVector(google::protobuf::Message& msg, const google::protobuf::FieldDescriptor& field, const std::vector<T>& val) noexcept
        {
            if (!field.is_repeated())
            {
                return unexpected<std::string>{"field is not repeated"};
            }
            auto converterResult = createFieldConverter<T>(field);
            if (!converterResult)
            {
                return unexpected{ std::move(converterResult).error() };
            }
            auto& converter = converterResult.value();
            msg.GetReflection()->ClearField(&msg, &field);
            for (const auto& elm : val)
            {
                converter.add(msg, elm);
            }
            return {};
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
                _outputFormat = protobuf::getPathFormat(outputPath);
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
bool operator==(const google::protobuf::RepeatedPtrField<T> proto, const std::unordered_set<T>& container) noexcept
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
