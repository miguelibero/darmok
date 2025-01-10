#include <darmok/utils.hpp>
#include <darmok/string.hpp>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <array>
#include <process.hpp>

namespace darmok
{
	void checkError(bx::Error& err)
    {
        if (!err.isOk())
        {
            auto& msg = err.getMessage();
			std::stringstream ss;
			ss << std::string_view(msg.getCPtr(), msg.getLength());
			ss << "( " << err.get().code << ")";
            throw std::runtime_error(ss.str());
        }
    }

    Random::Random(uint32_t seed) noexcept
        : _seed(seed)
    {
    }

    uint32_t Random::hash(uint32_t input) noexcept
    {
        // PGC hash
        uint32_t state = input * 747796405u + 2891336453u;
        uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
    }

    std::string Exec::argsToString(const std::vector<Arg>& args)
    {
        std::ostringstream ss;
        for (const auto& arg : args)
        {
            std::string strArg;
            if (auto path = std::get_if<std::filesystem::path>(&arg))
            {
                strArg = path->string();
                std::replace(strArg.begin(), strArg.end(), '\\', '/');
            }
            else if (auto constChar = std::get_if<const char*>(&arg))
            {
                strArg = *constChar;
            }
            else
            {
                strArg = std::get<std::string>(arg);
            }
            ss << StringUtils::escapeArgument(strArg) << " ";
        }
        return ss.str();
    }

    Exec::Result Exec::run(const std::vector<Arg>& args, const std::filesystem::path& path)
    {
        auto cmd = argsToString(args);
        std::ostringstream out;
        std::ostringstream err;
        TinyProcessLib::Process process(cmd, path.string(),
            [&out](const char* bytes, size_t n)
            {
                out.write(bytes, n);
            },
            [&err](const char* bytes, size_t n)
            {
                err.write(bytes, n);
            }
        );
        auto code = process.get_exit_status();
        return Result{
            .out = out.str(),
            .err = err.str(),
            .returnCode = code
        };
    }

    std::filesystem::path getTempPath(std::string_view prefix) noexcept
    {
        auto filename = std::string(prefix) + "XXXXXX";
        auto path = (std::filesystem::temp_directory_path() / filename).string();
        std::vector<char> data;
        data.insert(data.end(), path.begin(), path.end());
        mkstemp(&data.front());
        return &data.front();
    }
}