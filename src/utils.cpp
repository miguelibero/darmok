#include <darmok/utils.hpp>
#include <darmok/string.hpp>
#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <array>
#include <random>
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

#ifdef _MSC_VER
    #define popen _popen
    #define pclose _pclose
#endif

    Exec::Result Exec::run(const std::vector<Arg>& args, const std::filesystem::path& path)
    {
        std::ostringstream cmd;
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
            cmd << StringUtils::escapeArgument(strArg) << " ";
        }

        std::ostringstream out;
        std::ostringstream err;
        TinyProcessLib::Process process(cmd.str(), path.string(),
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

    TypeFilter& TypeFilter::include(entt::id_type idType) noexcept
    {
        if (idType != 0)
        {
            _includes.insert(idType);
        }
        return *this;
    }

    TypeFilter& TypeFilter::exclude(entt::id_type idType) noexcept
    {
        if (idType != 0)
        {
            _excludes.insert(idType);
        }
        return *this;
    }

    const TypeFilter::Container& TypeFilter::getIncludes() const noexcept
    {
        return _includes;
    }

    const TypeFilter::Container& TypeFilter::getExcludes() const noexcept
    {
        return _excludes;
    }

    std::string TypeFilter::toString() const noexcept
    {
        std::stringstream ss;
        ss << "TypeFilter(" << std::endl;
        if (!_includes.empty())
        {
            ss << "  includes=" << StringUtils::join(",", _includes.begin(), _includes.end()) << std::endl;
        }
        if (!_excludes.empty())
        {
            ss << "  excludes=" << StringUtils::join(",", _excludes.begin(), _excludes.end()) << std::endl;
        }
        ss << ")" << std::endl;
        return ss.str();
    }

    bool TypeFilter::operator==(const TypeFilter& other) const noexcept
    {
        return _includes == other._includes && _excludes == other._excludes;
    }

    bool TypeFilter::operator!=(const TypeFilter& other) const noexcept
    {
        return !operator==(other);
    }

    TypeFilter TypeFilter::operator+(const TypeFilter& other) const noexcept
    {
        TypeFilter r(*this);
        r += other;
        return r;
    }

    TypeFilter& TypeFilter::operator+=(const TypeFilter& other) noexcept
    {
        _includes.insert(other._includes.begin(), other._includes.end());
        _excludes.insert(other._excludes.begin(), other._excludes.end());
        return *this;
    }

    bool TypeFilter::empty() const noexcept
    {
        return _includes.empty() && _excludes.empty();
    }

    bool TypeFilter::matches(entt::id_type type) const noexcept
    {
        if (!_includes.empty())
        {
            if (!_includes.contains(type))
            {
                return false;
            }
        }
        if (!_excludes.empty())
        {
            if (_excludes.contains(type))
            {
                return false;
            }
        }
        return true;
    }

    bool TypeFilter::operator()(entt::id_type type) const noexcept
    {
        return matches(type);
    }
}