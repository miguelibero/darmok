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
        struct BX_NO_VTABLE IFieldConverter
        {
            IFieldConverter(const google::protobuf::FieldDescriptor& field) noexcept
                : _field{ &field }
            {
            }

            virtual ~IFieldConverter() = default;

            [[nodiscard]] virtual bool check() const noexcept = 0;
            [[nodiscard]] virtual T get(const google::protobuf::Message& msg) noexcept = 0;
            [[nodiscard]] virtual T get(const google::protobuf::Message& msg, int i) noexcept = 0;
            [[nodiscard]] virtual void set(google::protobuf::Message& msg, const T& val) noexcept = 0;
            [[nodiscard]] virtual void add(google::protobuf::Message& msg, const T& val) noexcept = 0;

            [[nodiscard]] expected<std::vector<T>, std::string> tryGetVector(const google::protobuf::Message& msg) noexcept
            {
                if (!_field->is_repeated())
                {
                    return unexpected<std::string>{"field is not repeated"};
                }
                if (!check())
                {
                    return unexpected<std::string>{"invalid type"};
                }
                return getVector(msg);
            }

            [[nodiscard]] std::vector<T> getVector(const google::protobuf::Message& msg) noexcept
            {
                std::vector<T> vec;
                auto size = msg.GetReflection()->FieldSize(msg, _field);
                vec.reserve(size);
                for (int i = 0; i < size; ++i)
                {
                    vec.push_back(get(msg, i));
                }
                return vec;
            }

            [[nodiscard]] expected<void, std::string> trySetVector(google::protobuf::Message& msg, const std::vector<T>& val) noexcept
            {
                if (!_field->is_repeated())
                {
                    return unexpected<std::string>{"field is not repeated"};
                }
                if (!check())
                {
                    return unexpected<std::string>{"invalid type"};
                }
                setVector(msg, val);
                return {};
            }

            void setVector(google::protobuf::Message& msg, const std::vector<T>& val) noexcept
            {
                msg.GetReflection()->ClearField(&msg, _field);
                for (const auto& elm : val)
                {
                    add(msg, elm);
                }
            }

        protected:
            const google::protobuf::FieldDescriptor* _field;
        };

        template <typename A, typename B>
        concept IsFieldConvertible = std::is_same_v<A, B>;

        template<typename T = int32_t>
            requires IsFieldConvertible<T, int32_t>
        struct Int32FieldConverter final : public IFieldConverter<T>
        {
            using IFieldConverter<T>::IFieldConverter;
            using IFieldConverter<T>::_field;

            [[nodiscard]] bool check() const noexcept override
            {
                auto type = _field->cpp_type();
                return type == FieldDescriptor::CPPTYPE_INT32 || type == FieldDescriptor::CPPTYPE_ENUM;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept override
            {
                return msg.GetReflection()->GetInt32(msg, _field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept override
            {
                return msg.GetReflection()->GetRepeatedInt32(msg, _field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->SetInt32(&msg, _field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->AddInt32(&msg, _field, val);
            }
        };

        template<typename T = int64_t>
            requires IsFieldConvertible<T, int64_t>
        struct Int64FieldConverter final : public IFieldConverter<T>
        {
            using IFieldConverter<T>::IFieldConverter;
            using IFieldConverter<T>::_field;

            [[nodiscard]] bool check() const noexcept override
            {
                auto type = _field->cpp_type();
                return type == FieldDescriptor::CPPTYPE_INT64;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept override
            {
                return msg.GetReflection()->GetInt64(msg, _field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept override
            {
                return msg.GetReflection()->GetRepeatedInt64(msg, _field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->SetInt64(&msg, _field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->AddInt64(&msg, _field, val);
            }
        };

        template<typename T = uint32_t>
            requires IsFieldConvertible<T, uint32_t>
        struct Uint32FieldConverter final : public IFieldConverter<T>
        {
            using IFieldConverter<T>::IFieldConverter;
            using IFieldConverter<T>::_field;

            [[nodiscard]] bool check() const noexcept override
            {
                auto type = _field->cpp_type();
                return type == FieldDescriptor::CPPTYPE_UINT32;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept override
            {
                return msg.GetReflection()->GetUInt32(msg, _field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept override
            {
                return msg.GetReflection()->GetRepeatedUInt32(msg, _field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->SetUInt32(&msg, _field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->AddUInt32(&msg, _field, val);
            }
        };

        template<typename T = uint64_t>
            requires IsFieldConvertible<T, uint64_t>
        struct Uint64FieldConverter final : public IFieldConverter<T>
        {
            using IFieldConverter<T>::IFieldConverter;
            using IFieldConverter<T>::_field;

            [[nodiscard]] bool check() const noexcept override
            {
                auto type = _field->cpp_type();
                return type == FieldDescriptor::CPPTYPE_UINT64;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept override
            {
                return msg.GetReflection()->GetUInt64(msg, _field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept override
            {
                return msg.GetReflection()->GetRepeatedUInt64(msg, _field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->SetUInt64(&msg, _field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->AddUInt64(&msg, _field, val);
            }
        };

        template<typename T = float>
            requires IsFieldConvertible<T, float>
        struct FloatFieldConverter final : public IFieldConverter<T>
        {
            using IFieldConverter<T>::IFieldConverter;
            using IFieldConverter<T>::_field;

            [[nodiscard]] bool check() const noexcept override
            {
                auto type = _field->cpp_type();
                return type == FieldDescriptor::CPPTYPE_FLOAT;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept override
            {
                return msg.GetReflection()->GetFloat(msg, _field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept override
            {
                return msg.GetReflection()->GetRepeatedFloat(msg, _field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->SetFloat(&msg, _field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->AddFloat(&msg, _field, val);
            }
        };

        template<typename T = double>
            requires IsFieldConvertible<T, double>
        struct DoubleFieldConverter final : public IFieldConverter<T>
        {
            using IFieldConverter<T>::IFieldConverter;
            using IFieldConverter<T>::_field;

            [[nodiscard]] bool check() const noexcept override
            {
                auto type = _field->cpp_type();
                return type == FieldDescriptor::CPPTYPE_DOUBLE;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept override
            {
                return msg.GetReflection()->GetDouble(msg, _field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept override
            {
                return msg.GetReflection()->GetRepeatedDouble(msg, _field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->SetDouble(&msg, _field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->AddDouble(&msg, _field, val);
            }
        };

        template<typename T = bool>
            requires IsFieldConvertible<T, bool>
        struct BoolFieldConverter final : public IFieldConverter<T>
        {
            using IFieldConverter<T>::IFieldConverter;
            using IFieldConverter<T>::_field;

            [[nodiscard]] bool check() const noexcept override
            {
                auto type = _field->cpp_type();
                return type == FieldDescriptor::CPPTYPE_BOOL;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept override
            {
                return msg.GetReflection()->GetBool(msg, _field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept override
            {
                return msg.GetReflection()->GetRepeatedBool(msg, _field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->SetBool(&msg, _field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->AddBool(&msg, _field, val);
            }
        };

        template<typename T = std::string>
            requires IsFieldConvertible<T, std::string>
        struct StringFieldConverter final : public IFieldConverter<T>
        {
            using IFieldConverter<T>::IFieldConverter;
            using IFieldConverter<T>::_field;

            [[nodiscard]] bool check() const noexcept override
            {
                auto type = _field->cpp_type();
                return type == FieldDescriptor::CPPTYPE_STRING;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept override
            {
                return msg.GetReflection()->GetString(msg, _field);
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept override
            {
                return msg.GetReflection()->GetRepeatedString(msg, _field, i);
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->SetString(&msg, _field, val);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept override
            {
                msg.GetReflection()->AddString(&msg, _field, val);
            }
        };

        template<typename T = int>
            requires std::is_enum_v<T> || std::is_same_v<T, int>
        struct EnumFieldConverter final : public IFieldConverter<T>
        {
            using IFieldConverter<T>::IFieldConverter;
            using IFieldConverter<T>::_field;

            [[nodiscard]] bool check() const noexcept override
            {
                auto type = _field->cpp_type();
                return type == FieldDescriptor::CPPTYPE_ENUM;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept override
            {
                return static_cast<T>(msg.GetReflection()->GetEnum(msg, _field)->number());
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept override
            {
                return static_cast<T>(msg.GetReflection()->GetRepeatedEnum(msg, _field, i)->number());
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept override
            {
                auto enumVal = _field->enum_type()->FindValueByNumber(static_cast<int>(val));
                msg.GetReflection()->SetEnum(&msg, _field, enumVal);
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept override
            {
                auto enumVal = _field->enum_type()->FindValueByNumber(static_cast<int>(val));
                msg.GetReflection()->AddEnum(&msg, _field, enumVal);
            }
        };

        template<typename T = google::protobuf::Message>
            requires std::is_assignable_v<T, google::protobuf::Message>
        struct MessageFieldConverter final : public IFieldConverter<T>
        {
            using IFieldConverter<T>::IFieldConverter;
            using IFieldConverter<T>::_field;

            [[nodiscard]] bool check() const noexcept override
            {
                auto type = _field->cpp_type();
                return type == FieldDescriptor::CPPTYPE_MESSAGE;
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg) noexcept override
            {
                return *static_cast<const T*>(msg.GetReflection()->GetMessage(msg, _field));
            }

            [[nodiscard]] T get(const google::protobuf::Message& msg, int i) noexcept override
            {
                return *static_cast<const T*>(msg.GetReflection()->GetRepeatedMessage(msg, _field, i));
            }

            [[nodiscard]] void set(google::protobuf::Message& msg, const T& val) noexcept override
            {
                *static_cast<T*>(msg.GetReflection()->MutableMessage(&msg, _field)) = val;
            }

            [[nodiscard]] void add(google::protobuf::Message& msg, const T& val) noexcept override
            {
                *static_cast<T*>(msg.GetReflection()->AddMessage(&msg, _field)) = val;
            }
        };

        template<typename T>
        [[nodiscard]] expected<std::unique_ptr<IFieldConverter<T>>, std::string> createFieldConverter(const google::protobuf::FieldDescriptor& field) noexcept
        {
            switch (field.cpp_type())
            {
            case FieldDescriptor::CPPTYPE_INT32:
                return std::make_unique<Int32FieldConverter<T>>(field);
            case FieldDescriptor::CPPTYPE_INT64:
                return std::make_unique<Int64FieldConverter<T>>(field);
            case FieldDescriptor::CPPTYPE_UINT32:
                return std::make_unique<Uint32FieldConverter<T>>(field);
            case FieldDescriptor::CPPTYPE_UINT64:
                return std::make_unique<Uint64FieldConverter<T>>(field);
            case FieldDescriptor::CPPTYPE_DOUBLE:
                return std::make_unique<DoubleFieldConverter<T>>(field);
            case FieldDescriptor::CPPTYPE_FLOAT:
                return std::make_unique<FloatFieldConverter<T>>(field);
            case FieldDescriptor::CPPTYPE_BOOL:
                return std::make_unique<BoolFieldConverter<T>>(field);
            case FieldDescriptor::CPPTYPE_STRING:
                return std::make_unique<StringFieldConverter<T>>(field);
            case FieldDescriptor::CPPTYPE_ENUM:
                return std::make_unique<EnumFieldConverter<T>>(field);
            case FieldDescriptor::CPPTYPE_MESSAGE:
                return std::make_unique<MessageFieldConverter<T>>(field);
            }
            return unexpected<std::string>{"cannot convert type"};
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
