#include <darmok/serialize.hpp>

namespace darmok
{
    std::ofstream CerealUtils::createSaveStream(CerealFormat format, const std::filesystem::path& path)
    {
        switch (format)
        {
        case CerealFormat::Binary:
            return std::ofstream(path, std::ios::binary);
        default:
            return std::ofstream(path);
        }
    }

    std::ifstream CerealUtils::createLoadStream(CerealFormat format, const std::filesystem::path& path)
    {
        switch (format)
        {
        case CerealFormat::Binary:
            return std::ifstream(path, std::ios::binary);
        default:
            return std::ifstream(path);
        }
    }

    CerealFormat CerealUtils::getFormat(const std::string& name) noexcept
    {
        if (name == "xml")
        {
            return CerealFormat::Xml;
        }
        if (name == "json")
        {
            return CerealFormat::Json;
        }
        return CerealFormat::Binary;
    }

    CerealFormat CerealUtils::getExtensionFormat(const std::filesystem::path& ext) noexcept
    {
        if (ext == ".xml")
        {
            return CerealFormat::Xml;
        }
        if (ext == ".json" || ext == ".js")
        {
            return CerealFormat::Json;
        }
        return CerealFormat::Binary;
    }

    std::string CerealUtils::getFormatExtension(CerealFormat format) noexcept
    {
        switch (format)
        {
        case CerealFormat::Binary:
        {
            return ".bin";
        }
        case CerealFormat::Json:
        {
            return ".json";
        }
        case CerealFormat::Xml:
        {
            return ".xml";
        }
        }
        return "";
    }

    std::string CerealUtils::getFormatName(CerealFormat format) noexcept
    {
        switch (format)
        {
        case CerealFormat::Binary:
        {
            return "binary";
        }
        case CerealFormat::Json:
        {
            return "json";
        }
        case CerealFormat::Xml:
        {
            return "xml";
        }
        }
        return "";
    }
}