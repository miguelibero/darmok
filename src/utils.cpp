#include <darmok/utils.hpp>
#include <darmok/string.hpp>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <array>
#include <random>

namespace darmok
{
	void checkError(bx::Error& err)
    {
        if (!err.isOk())
        {
            auto& msg = err.getMessage();
			std::stringstream ss;
			ss << std::string_view(msg.getPtr(), msg.getLength());
			ss << "( " << err.get().code << ")";
            throw std::runtime_error(ss.str());
        }
    }    

#ifdef _MSC_VER
    #define popen _popen
    #define pclose _pclose
#endif

    ExecResult exec(const std::vector<std::string>& args)
    {
        std::ostringstream cmd;
        for (const auto& arg : args)
        {
            cmd << StringUtils::escapeArgument(arg) << " ";
        }

        FILE* pipe = popen(cmd.str().c_str(), "r");
        if (!pipe)
        {
            throw std::runtime_error("popen() failed!");
        }

        ExecResult result;
        std::array<char, 128> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        {
            result.output += buffer.data();
        }

        result.returnCode = pclose(pipe);
        return result;
    }

    std::filesystem::path getTempPath(std::string_view suffix) noexcept
    {
        std::string name = std::tmpnam(nullptr);
        name += suffix;
        return std::filesystem::temp_directory_path() / name;
    }

    entt::id_type randomIdType() noexcept
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<entt::id_type> uni;
        return uni(rng);
    }
}