#pragma once

#include <darmok/export.h>
#include <darmok/expected.hpp>

#include <filesystem>
#include <string>
#include <fstream>

#include <google/protobuf/message.h>
#include <google/protobuf/repeated_ptr_field.h>
#include <nlohmann/json.hpp>

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

        ProtobufFormat getFormat(const std::filesystem::path& path);

        std::pair<std::ifstream, ProtobufFormat> createInputStream(const std::filesystem::path& path);
        expected<void, std::string> read(Message& msg, const std::filesystem::path& path);
        expected<void, std::string> read(Message& msg, std::istream& input, ProtobufFormat format);
        expected<void, std::string> readJson(Message& msg, std::istream& input);
        
        std::pair<std::ofstream, ProtobufFormat> createOutputStream(const std::filesystem::path& path);
        expected<void, std::string> write(const Message& msg, const std::filesystem::path& path);
        expected<void, std::string> write(const Message& msg, std::ostream& output, ProtobufFormat format);
        expected<void, std::string> writeJson(const Message& msg, std::ostream& output);
    }
}

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