#pragma once
#include "Log.h"
#include "../Common.h"

using namespace FrameGraph;

namespace
{
	GND inline FrameGraph::StringView ToShortPath(FrameGraph::StringView file)
	{
		const uint	max_parts = 2;

		size_t	i = file.length() - 1;
		uint	j = 0;

		for (; i < file.length() and j < max_parts; --i)
		{
			const char	c = file[i];

			if ((c == '\\') | (c == '/'))
				++j;
		}

		return file.substr(i + (j == max_parts ? 2 : 0));
	}
}

namespace
{
	static FrameGraph::SharedMutex			s_LogCallbackGuard; // 这里定义为共享锁 因为有大量多线程需要读取
	static FrameGraph::Logger::Callback_t   s_LogCallback = null;
	static void*							s_LogCallbackUserData = null;

	static void ExternalLogger(const FrameGraph::StringView& msg, const FrameGraph::StringView& file, int line, bool isError)
	{
		SHAREDLOCK(s_LogCallbackGuard);

		if (s_LogCallback)
			s_LogCallback(s_LogCallbackUserData, msg, file, line, isError);
	}
}


FrameGraph::Logger::EResult FrameGraph::Logger::Info(const char* msg, const char* func, const char* file, int line)
{
	return Info(StringView{ msg }, StringView{ func }, StringView{ file }, line);
}

FrameGraph::Logger::EResult FrameGraph::Logger::Error(const char* msg, const char* func, const char* file, int line)
{
	return Error(StringView{ msg }, StringView{ func }, StringView{ file }, line);
}



void FrameGraph::Logger::SetCallback(Callback_t cb, void* userData)
{
	EXLOCK(s_LogCallbackGuard);
	s_LogCallback = cb;
	s_LogCallbackUserData = userData;
}

namespace {
	inline void IDEConsoleMessage(FrameGraph::StringView message, FrameGraph::StringView file, int line, bool isError)
	{
#ifdef COMPILER_MSVC
		const String	str = String{ file } << '(' << ToString(line) << "): " << (isError ? "Error: " : "") << message << '\n';

		::OutputDebugStringA(str.c_str());
#endif
	}
}

FrameGraph::Logger::EResult FrameGraph::Logger::Info(const StringView& msg, const StringView&, const StringView& file, int line)
{
	IDEConsoleMessage(msg, file, line, false);
	//ConsoleOutput(msg, file, line, false);
	ExternalLogger(msg, file, line, false);

	return EResult::Continue;
}

FrameGraph::Logger::EResult FrameGraph::Logger::Error(const StringView& msg, const StringView& func, const StringView& file, int line)
{
	IDEConsoleMessage(msg, file, line, true);
	ExternalLogger(msg, file, line, true);

}





