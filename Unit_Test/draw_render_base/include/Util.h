#pragma once

#include <malloc.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <vector>

namespace vkUtil
{

	int EndsWith(const char* s, const char* part);

	void PrintShaderSource(const char* text);

	std::string ReadShaderFile(const char* fileName);



}