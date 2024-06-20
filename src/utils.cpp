#include <darmok/utils.hpp>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <array>

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

    void copyStream(std::istream& input, std::ostream& output, size_t bufferSize)
    {
        std::vector<char> buffer(bufferSize);
        while (input.read(&buffer.front(), bufferSize) || input.gcount() > 0)
        {
            output.write(&buffer.front(), input.gcount());
        }
    }

    std::string escapeArgument(const std::string& arg)
    {
        std::ostringstream oss;

        // Check if argument needs quoting based on Windows or Unix-like rules
        bool needsQuotes = false;
        for (char c : arg)
        {
            if (c == ' ' || c == '\t' || c == '"' || c == '\\')
            {
                needsQuotes = true;
                break;
            }
        }

        // Quote the argument and escape characters inside quotes
        if (needsQuotes)
        {
            oss << '"';
            for (char c : arg)
            {
                if (c == '"' || c == '\\')
                {
                    oss << '\\'; // Escape double quotes and backslashes
                }
                oss << c;
            }
            oss << '"';
        }
        else
        {
            oss << arg;
        }

        return oss.str();
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
            cmd << escapeArgument(arg) << " ";
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
}