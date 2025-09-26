#include <darmok/protobuf.hpp>
#include <darmok/stream.hpp>
#include <darmok/string.hpp>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/any.h>
#include <magic_enum/magic_enum.hpp>

namespace darmok
{
    namespace protobuf
    {
        Format getFormat(const std::filesystem::path& path)
        {
            if (path.extension() == ".json")
            {
                return Format::Json;
            }
            if (path.extension() == ".xml")
            {
                return Format::Json;
            }
            return Format::Binary;
        }

        std::string_view getExtension(Format format)
        {
            if (format == Format::Json)
            {
                return ".json";
            }
            return ".pb";
        }

        std::optional<Format> getFormat(std::string_view name)
        {
            return magic_enum::enum_cast<Format>(name);
        }

        std::size_t getHash(const Message& msg)
        {
            // TODO: better algorithm
            return std::hash<std::string>{}(msg.SerializeAsString());
        }

        const bgfx::Memory* copyMem(const std::string& data)
        {
            return bgfx::copy(data.data(), data.size());
        }

        const bgfx::Memory* refMem(const std::string& data)
        {
            return bgfx::makeRef(data.data(), data.size());
        }

        std::pair<std::ifstream, Format> createInputStream(const std::filesystem::path& path)
        {
            auto format = getFormat(path);
            return { createInputStream(path, format), format };
        }

        std::ifstream createInputStream(const std::filesystem::path& path, Format format)
        {
            int streamFlags = 0;
            if (format == Format::Binary)
            {
                streamFlags = std::ios::binary;
            }
            return std::ifstream(path, streamFlags);
        }

        std::pair<std::ofstream, Format> createOutputStream(const std::filesystem::path& path)
        {
            auto format = getFormat(path);
            return { createOutputStream(path, format), format };
        }

        std::ios_base::openmode getOutputStreamMode(Format format)
        {
            if (format == Format::Binary)
            {
                return std::ios::binary;
            }
            return std::ios::out;
        }

        std::ofstream createOutputStream(const std::filesystem::path& path, Format format)
        {
            return std::ofstream{ path, getOutputStreamMode(format) };
        }

        expected<void, std::string> read(Message& msg, const std::filesystem::path& path)
        {
            auto [input, format] = createInputStream(path);
            return read(msg, input, format);
        }

        expected<void, std::string> read(Message& msg, std::istream& input, Format format)
        {
            if (!input)
            {
                return unexpected{ "could not create input stream" };
            }
            if (format == Format::Json)
            {
                return readJson(msg, input);
            }
            else if (!msg.ParseFromIstream(&input))
            {
                return unexpected{ "failed to parse from binary file" };
            }
            return {};
        }

        expected<void, std::string> readJson(Message& msg, std::istream& input)
        {
            auto readResult = StreamUtils::readString(input);
            if (!readResult)
            {
				return unexpected{ readResult.error() };
            }
            auto status = google::protobuf::util::JsonStringToMessage(readResult.value(), &msg);
            if (!status.ok())
            {
                return unexpected{ "got protobuf error status " + status.ToString() };
            }
            return {};
        }

        expected<void, std::string> readJson(Message& msg, const nlohmann::json& json)
        {
            const auto* desc = msg.GetDescriptor();

            for (int i = 0; i < desc->field_count(); ++i)
            {
                const auto* field = desc->field(i);
                if (!field)
                {
                    continue;
                }
                auto result = readJson(msg, *field, json);
                if (!result)
                {
                    return result;
                }
            }
            return {};
        }

        expected<void, std::string> readJson(Message& msg, const FieldDescriptor& field, const nlohmann::json& json)
        {
            const auto& name = field.name();
            auto itr = json.find(name);
            if (itr == json.end())
            {
                return {};
            }
            auto& jsonVal = *itr;
            const auto* desc = msg.GetDescriptor();
            const auto* refl = msg.GetReflection();

            if (jsonVal.is_null())
            {
                refl->ClearField(&msg, &field);
                return {};
            }

            if (field.is_repeated())
            {
                for (auto& jsonElm : jsonVal)
                {
                    switch (field.cpp_type()) {
                    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                        refl->AddInt32(&msg, &field, jsonElm);
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                        refl->AddInt64(&msg, &field, jsonElm);
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
                        refl->AddUInt32(&msg, &field, jsonElm);
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
                        refl->AddUInt64(&msg, &field, jsonElm);
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
                        refl->AddDouble(&msg, &field, jsonElm);
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                        refl->AddFloat(&msg, &field, jsonElm);
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                        refl->AddBool(&msg, &field, jsonElm);
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                        refl->AddEnumValue(&msg, &field, jsonElm);
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                        refl->AddString(&msg, &field, jsonElm);
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
                        auto result = readJson(*refl->AddMessage(&msg, &field), jsonElm);
                        if (!result)
                        {
                            return unexpected{result.error()};
                        }
                        break;
                    }
                }
            }
            else if (field.is_map())
            {
                for (auto& [jsonKey, jsonVal] : json.items())
                {
                    auto* entry = refl->AddMessage(&msg, &field);
                    const auto* entryRefl = entry->GetReflection();
                    const auto* entryDesc = entry->GetDescriptor();
                    auto keyField = entryDesc->map_key();
                    auto valField = entryDesc->map_value();
                    entryRefl->SetString(entry, keyField, jsonKey);
                    auto result = readJson(*entry, *valField, jsonVal);
                    if (!result)
                    {
                        return unexpected{result.error()};
                    }
                }
            }
            else
            {
                switch (field.cpp_type()) {
                case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                    refl->SetInt32(&msg, &field, jsonVal);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                    refl->SetInt64(&msg, &field, jsonVal);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
                    refl->SetUInt32(&msg, &field, jsonVal);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
                    refl->SetUInt64(&msg, &field, jsonVal);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
                    refl->SetDouble(&msg, &field, jsonVal);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                    refl->SetFloat(&msg, &field, jsonVal);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                    refl->SetBool(&msg, &field, jsonVal);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                    refl->SetEnumValue(&msg, &field, jsonVal);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                    refl->SetString(&msg, &field, jsonVal);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
                    auto result = readJson(*refl->MutableMessage(&msg, &field), jsonVal);
                    if (!result)
                    {
                        return unexpected{result.error()};
                    }
                    break;
                }
            }
            return {};
        }

        expected<void, std::string> write(const Message& msg, const std::filesystem::path& path)
        {
            auto [output, format] = createOutputStream(path);
            return write(msg, output, format);
        }

        expected<void, std::string> write(const Message& msg, std::ostream& output, Format format)
        {
            if (!output)
            {
                return unexpected{ "could not create output stream" };
            }
            if (format == Format::Json)
            {
                return writeJson(msg, output);
            }
            else if (!msg.SerializeToOstream(&output))
            {
                return unexpected{ "failed to serialize to binary file" };
            }
            return {};
        }

        expected<void, std::string> writeJson(const Message& msg, std::ostream& output)
        {
            google::protobuf::util::JsonPrintOptions options;
            options.add_whitespace = true;
            options.preserve_proto_field_names = true;

            std::string json;
            auto status = google::protobuf::util::MessageToJsonString(msg, &json, options);
            if (!status.ok())
            {
                return unexpected{ "got protobuf error status " + status.ToString() };
            }

            output << json;
            return {};
        }

        expected<void, std::string> writeJson(const Message& msg, nlohmann::json& json)
        {
            const auto* desc = msg.GetDescriptor();

            for (int i = 0; i < desc->field_count(); ++i)
            {
                const auto* field = desc->field(i);
                if (!field)
                {
                    continue;
                }
                auto result = writeJson(msg, *field, json);
                if (!result)
                {
                    return result;
                }
            }
            return {};
        }

        expected<void, std::string> writeJson(const Message& msg, const FieldDescriptor& field, nlohmann::json& json)
        {
            const auto& name = field.name();
            const auto* desc = msg.GetDescriptor();
            const auto* refl = msg.GetReflection();

            if (!refl->HasField(msg, &field))
            {
                json[name] = nullptr;
                return {};
            }

            if (field.is_repeated())
            {
                auto size = refl->FieldSize(msg, &field);
                auto jsonArray = nlohmann::json::array();
                for (int i = 0; i < size; ++i)
                {
                    switch (field.cpp_type()) {
                    case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                        jsonArray.push_back(refl->GetRepeatedInt32(msg, &field, i));
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                        jsonArray.push_back(refl->GetRepeatedInt64(msg, &field, i));
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
                        jsonArray.push_back(refl->GetRepeatedUInt32(msg, &field, i));
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
                        jsonArray.push_back(refl->GetRepeatedUInt64(msg, &field, i));
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
                        jsonArray.push_back(refl->GetRepeatedDouble(msg, &field, i));
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                        jsonArray.push_back(refl->GetRepeatedFloat(msg, &field, i));
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                        jsonArray.push_back(refl->GetRepeatedBool(msg, &field, i));
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                        jsonArray.push_back(refl->GetRepeatedEnumValue(msg, &field, i));
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                        jsonArray.push_back(refl->GetRepeatedString(msg, &field, i));
                        break;
                    case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
                        auto& jsonObj = jsonArray.emplace_back();
                        auto result = writeJson(refl->GetRepeatedMessage(msg, &field, i), jsonObj);
                        if (!result)
                        {
                            return unexpected{result.error()};
                        }
                        break;
                    }
                }
                json[name] = jsonArray;
            }
            else if (field.is_map())
            {
                auto size = refl->FieldSize(msg, &field);
                auto jsonObj = nlohmann::json::object();
                for (int i = 0; i < size; ++i)
                {
                    auto& entry = refl->GetRepeatedMessage(msg, &field, i);
                    const auto* entryRefl = entry.GetReflection();
                    const auto* entryDesc = entry.GetDescriptor();
                    auto keyField = entryDesc->map_key();
                    auto valField = entryDesc->map_value();
                    auto jsonKey = entryRefl->GetString(entry, keyField);
                    auto result = writeJson(entry, *valField, json[jsonKey]);
                    if (!result)
                    {
                        return unexpected{result.error()};
                    }
                }
                json[name] = jsonObj;
            }
            else
            {
                switch (field.cpp_type()) {
                case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
                    json[name] = refl->GetInt32(msg, &field);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
                    json[name] = refl->GetInt64(msg, &field);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
                    json[name] = refl->GetUInt32(msg, &field);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
                    json[name] = refl->GetUInt64(msg, &field);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
                    json[name] = refl->GetDouble(msg, &field);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
                    json[name] = refl->GetFloat(msg, &field);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
                    json[name] = refl->GetBool(msg, &field);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                    json[name] = refl->GetEnumValue(msg, &field);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
                    json[name] = refl->GetString(msg, &field);
                    break;
                case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
                    auto result = writeJson(refl->GetMessage(msg, &field), json[name]);
                    if (!result)
                    {
                        return unexpected{result.error()};
                    }
                    break;
                }
            }
            return {};
        }

        static constexpr std::string_view typeUrlPrefix = "type.googleapis.com/";

        IdType getTypeId(const Message& msg)
        {
            return std::hash<std::string>{}(getFullName(msg));
        }

        IdType getTypeId(const Descriptor& desc)
        {
            return std::hash<std::string>{}(desc.full_name());
        }

        std::string getFullName(const Message& msg)
        {
            if (isAny(msg))
            {
                auto fullName = static_cast<const google::protobuf::Any&>(msg).type_url();
                if (StringUtils::startsWith(fullName, typeUrlPrefix))
                {
                    fullName = fullName.substr(typeUrlPrefix.size());
                }
                return fullName;
            }
            return msg.GetDescriptor()->full_name();
        }

        std::string getTypeUrl(const Message& msg)
        {
            if (isAny(msg))
            {
                return static_cast<const google::protobuf::Any&>(msg).type_url();
            }
            return getTypeUrl(*msg.GetDescriptor());
        }

        std::string getTypeUrl(const Descriptor& desc)
        {
            return std::string{ typeUrlPrefix } + desc.full_name();
        }

        bool isAny(const Message& msg)
        {
            return msg.GetDescriptor()->full_name() == "google.protobuf.Any";
        }

        std::vector<std::string> getEnumValues(const google::protobuf::EnumDescriptor& enumDesc)
        {
            std::vector<std::string> values;
            auto prefix = enumDesc.full_name() + "_";
            values.reserve(enumDesc.value_count());
            for (int i = 0; i < enumDesc.value_count(); ++i)
            {
                auto value = enumDesc.value(i)->name();
                if (StringUtils::startsWith(value, prefix))
                {
                    value = value.substr(prefix.size());
                }
                values.push_back(std::move(value));
            }
            return values;
        }
    }
}

void to_json(nlohmann::json& json, const google::protobuf::Message& msg)
{
    auto result = darmok::protobuf::writeJson(msg, json);
    if (!result)
    {
		throw std::runtime_error("failed to write json: " + result.error());
    }
}

void from_json(const nlohmann::json& json, google::protobuf::Message& msg)
{
    auto result = darmok::protobuf::readJson(msg, json);
    if (!result)
    {
        throw std::runtime_error("failed to read json: " + result.error());
    }
}