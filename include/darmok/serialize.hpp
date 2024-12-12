#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/string.hpp>

#include <stack>
#include <filesystem>
#include <fstream>

#include <cereal/cereal.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/portable_binary.hpp>

#define DARMOK_IMPLEMENT_TEMPLATE_CEREAL_SERIALIZE(funcName)	                                    \
template void funcName<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive&);                  \
template void funcName<cereal::BinaryInputArchive>(cereal::BinaryInputArchive&);                    \
template void funcName<cereal::PortableBinaryOutputArchive>(cereal::PortableBinaryOutputArchive&);  \
template void funcName<cereal::PortableBinaryInputArchive>(cereal::PortableBinaryInputArchive&);    \
template void funcName<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&);                        \
template void funcName<cereal::XMLInputArchive>(cereal::XMLInputArchive&);                          \
template void funcName<cereal::JSONOutputArchive>(cereal::JSONOutputArchive&);                      \
template void funcName<cereal::JSONInputArchive>(cereal::JSONInputArchive&);                        \
template void funcName<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&);                        \
template void funcName<cereal::XMLInputArchive>(cereal::XMLInputArchive&);                          \

#define DARMOK_IMPLEMENT_TEMPLATE_CEREAL_SAVE(funcName)	                                            \
template void funcName<cereal::BinaryOutputArchive>(cereal::BinaryOutputArchive&);                  \
template void funcName<cereal::PortableBinaryOutputArchive>(cereal::PortableBinaryOutputArchive&);  \
template void funcName<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&);                        \
template void funcName<cereal::JSONOutputArchive>(cereal::JSONOutputArchive&);                      \
template void funcName<cereal::XMLOutputArchive>(cereal::XMLOutputArchive&);                        \

#define DARMOK_IMPLEMENT_TEMPLATE_CEREAL_LOAD(funcName)	                                            \
template void funcName<cereal::BinaryInputArchive>(cereal::BinaryInputArchive&);                    \
template void funcName<cereal::PortableBinaryInputArchive>(cereal::PortableBinaryInputArchive&);    \
template void funcName<cereal::XMLInputArchive>(cereal::XMLInputArchive&);                          \
template void funcName<cereal::JSONInputArchive>(cereal::JSONInputArchive&);                        \
template void funcName<cereal::XMLInputArchive>(cereal::XMLInputArchive&);                          \


namespace darmok
{
    // using this instead of cereal::UserDataAdapter
    // because it allows us to maintain the same archive during the serialization
    template<typename T>
    struct SerializeContextStack final
    {
        static void push(T& ctx)
        {
            _stack.emplace(ctx);
        }

        static T& get()
        {
            return _stack.top().get();
        }

        static OptionalRef<T> tryGet()
        {
            if (_stack.empty())
            {
                return nullptr;
            }
            return get();
        }

        static void pop()
        {
            _stack.pop();
        }
    private:
        static thread_local std::stack<std::reference_wrapper<T>> _stack;
    };

    template<typename T>
    thread_local std::stack<std::reference_wrapper<T>> SerializeContextStack<T>::_stack;

    enum class CerealFormat
    {
        Binary,
        Json,
        Xml
    };

    struct CerealUtils final
    {
        static std::ofstream createSaveStream(CerealFormat format, const std::filesystem::path& path);
        static std::ifstream createLoadStream(CerealFormat format, const std::filesystem::path& path);
        static CerealFormat getFormat(const std::string& name) noexcept;
        static CerealFormat getExtensionFormat(const std::filesystem::path& ext) noexcept;
        static std::string getFormatExtension(CerealFormat format) noexcept;
        static std::string getFormatName(CerealFormat format) noexcept;

        template<typename T>
        static void load(T& obj, const std::filesystem::path& path)
        {
            auto format = getExtensionFormat(path.extension().string());
            auto stream = createLoadStream(format, path);
            load(obj, stream, format);
        }

        template<typename T>
        static void save(const T& obj, const std::filesystem::path& path)
        {
            auto format = getExtensionFormat(path.extension());
            auto stream = createSaveStream(format, path);
            save(obj, stream, format);
        }

        template<typename T>
        static void load(T& obj, std::istream& stream, CerealFormat format)
        {
            if (format == CerealFormat::Xml)
            {
                cereal::XMLInputArchive archive(stream);
                archive(obj);
            }
            else if (format == CerealFormat::Json)
            {
                cereal::JSONInputArchive archive(stream);
                archive(obj);
            }
            else
            {
                cereal::PortableBinaryInputArchive archive(stream);
                archive(obj);
            }
        }

        template<typename T>
        static void save(const T& obj, std::ostream& stream, CerealFormat format)
        {
            if (format == CerealFormat::Xml)
            {
                cereal::XMLOutputArchive archive(stream);
                archive(obj);
            }
            else if (format == CerealFormat::Json)
            {
                cereal::JSONOutputArchive archive(stream);
                archive(obj);
            }
            else
            {
                cereal::PortableBinaryOutputArchive archive(stream);
                archive(obj);
            }
        }

        template<typename T>
        static void load(T& obj, const DataView& dataView, CerealFormat format)
        {
            DataInputStream stream(dataView);
            load(obj, stream, format);
        }

        template<typename T>
        static std::streampos save(T& obj, Data& data, CerealFormat format)
        {
            DataOutputStream stream(data);
            save(obj, stream, format);
            return stream.tellp();
        }
    };


    template<typename Interface>
    class DARMOK_EXPORT CerealLoaderImporter : public IAssetTypeImporter
    {
    private:
        CerealLoaderImporter(Interface& loader, const std::string& name) noexcept
            : _loader(loader)
        {
        }

        bool startImport(const Input& input, bool dry = false) override
        {
            _outputPath = input.getOutputPath(".bin");
            if (input.config.contains("outputFormat"))
            {
                _outputFormat = CerealUtils::getFormat(input.config["outputFormat"]);
            }
            else if (input.dirConfig.contains("outputFormat"))
            {
                _outputFormat = CerealUtils::getFormat(input.dirConfig["outputFormat"]);
            }
            else
            {
                _outputFormat = CerealUtils::getExtensionFormat(_outputPath.extension());
            }
            return !_outputPath.empty();
        }

        virtual Outputs getOutputs(const Input& input)
        {
            return { _outputPath };
        }

        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& outputPath) override
        {
            return CerealUtils::createSaveStream(_outputFormat);
        }

        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
        {
            auto obj = _loader(input.path);
            CerealUtils::save(*obj, out, _outputFormat);
        }

        const std::string& getName() const noexcept
        {
            return _name;
        }

    private:
        Interface& _loader;
        std::string _name;
        std::filesystem::path _outputPath;
        CerealFormat _outputFormat;
    };
}