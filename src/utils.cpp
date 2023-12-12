#include <darmok/utils.hpp>
#include <string>
#include <stdexcept>

namespace darmok
{
    void checkError(bx::Error& err)
    {
        if (!err.isOk())
        {
            auto msg = err.getMessage();
            std::string strMsg(msg.getPtr(), msg.getLength());
            strMsg += "( " + err.get().code + std::string(")");
            throw std::runtime_error(strMsg.c_str());
        }
    }
}