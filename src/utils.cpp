#include <darmok/utils.hpp>
#include <sstream>
#include <stdexcept>

namespace darmok
{
	void checkError(bx::Error& err)
    {
        if (!err.isOk())
        {
            auto msg = err.getMessage();
			std::stringstream ss;
			ss << std::string_view(msg.getPtr(), msg.getLength());
			ss << "( " << err.get().code << ")";
            throw std::runtime_error(ss.str());
        }
    }
}