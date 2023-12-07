#include <darmok/utils.hpp>
#include <string>

namespace darmok
{
    void checkError(bx::Error& err)
    {
        if (!err.isOk())
        {
            auto msg = err.getMessage();
            std::string strMsg(msg.getPtr(), msg.getLength());
            throw std::exception(strMsg.c_str(), err.get().code);
        }
    }
}