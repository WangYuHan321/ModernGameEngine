#pragma once

#include "../Config.h"
#include "../Common.h"
#include "../Defines.h"
#include "../Containers/StringView.h"

namespace FrameGraph
{

	struct Logger
	{
		enum class EResult
		{
			Continue,
			Break,
			Abort,
		};

		static EResult Info(const char* msg, const char* func, const char* file, int line);
		static EResult Info(const StringView& msg, const StringView& func, const StringView& file, int line);

		static EResult Error(const char* msg, const char* func, const char* file, int line);
		static EResult Error(const StringView& msg, const StringView& func, const StringView& file, int line);

		using Callback_t = void (*) (void* userData, const StringView& msg, const StringView& file, int line, bool isError);

		static void SetCallback(Callback_t cb, void* userData);
	};




#define FG_PRIVATE_LOGX( _level, _msg_, _file_, _line_ ) \
	{\
		switch (Logger::_level_((_msg_),(FUNCTION_NAME), (_file_), (_line_)))\
		{\
			Logger::EResult::Continue: break;\
			Logger::EResult::Break: break;\
			Logger::EResult::Abort: std::abort();\
		}\
	}\

#define FG_PRIVATE_LOGI( _msg_, _file_, _line_ )	FG_PRIVATE_LOGX( Info, (_msg_), (_file_), (_line_) )
#define FG_PRIVATE_LOGE( _msg_, _file_, _line_ )	FG_PRIVATE_LOGX( Error, (_msg_), (_file_), (_line_) )

};










