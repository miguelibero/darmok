#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace darmok
{
	typedef int32_t (*RunAppCallback)(const std::vector<std::string>& args);
}