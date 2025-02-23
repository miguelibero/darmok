#include <darmok/protobuf.hpp>
#include <darmok/stream.hpp>
#include <google/protobuf/util/json_util.h>

namespace darmok
{
    namespace ProtobufUtils
    {
        ProtobufFormat getFormat(const std::filesystem::path& path)
        {
            if (path.extension() == ".json")
            {
                return ProtobufFormat::Json;
            }
            return ProtobufFormat::Binary;
        }

        std::pair<std::ifstream, ProtobufFormat> createInputStream(const std::filesystem::path& path)
        {
            int streamFlags = 0;
            auto format = getFormat(path);
            if (format == ProtobufFormat::Binary)
            {
                streamFlags = std::ios::binary;
            }
            return { std::ifstream(path, streamFlags), format };
        }

        std::pair<std::ofstream, ProtobufFormat> createOutputStream(const std::filesystem::path& path)
        {
            int streamFlags = 0;
            auto format = getFormat(path);
            if (format == ProtobufFormat::Binary)
            {
                streamFlags = std::ios::binary;
            }
            return { std::ofstream(path, streamFlags), format };
        }

        expected<void, std::string> read(Message& msg, const std::filesystem::path& path)
        {
            auto [input, format] = createInputStream(path);
            return read(msg, input, format);
        }

        expected<void, std::string> read(Message& msg, std::istream& input, ProtobufFormat format)
        {
            if (!input)
            {
                return unexpected<std::string>{ "could not create input stream" };
            }
            if (format == ProtobufFormat::Json)
            {
                return readJson(msg, input);
            }
            else if (!msg.ParseFromIstream(&input))
            {
                return unexpected<std::string>{ "failed to parse from binary file" };
            }
            return {};
        }

        expected<void, std::string> readJson(Message& msg, std::istream& input)
        {
            auto json = StreamUtils::readString(input);
            auto status = google::protobuf::util::JsonStringToMessage(json, &msg);
            if (!status.ok())
            {
                return unexpected<std::string>{ "got protobuf error status " + status.ToString() };
            }
            return {};
        }

        expected<void, std::string> write(const Message& msg, const std::filesystem::path& path)
        {
            auto [output, format] = createOutputStream(path);
            return write(msg, output, format);
        }

        expected<void, std::string> write(const Message& msg, std::ostream& output, ProtobufFormat format)
        {
            if (!output)
            {
                return unexpected<std::string>{ "could not create output stream" };
            }
            if (format == ProtobufFormat::Json)
            {
                return writeJson(msg, output);
            }
            else if (!msg.SerializeToOstream(&output))
            {
                return unexpected<std::string>{ "failed to serialize to binary file" };
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
                return unexpected<std::string>{ "got protobuf error status " + status.ToString() };
            }

            output << json;
            return {};
        }
    }
}