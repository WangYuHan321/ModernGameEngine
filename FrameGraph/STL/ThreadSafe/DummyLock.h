#pragma once

#include "../Common.h"

namespace FrameGraph
{
	// 空锁：满足 lock_guard / unique_lock 接口，无实际开销
	struct DummyLock
	{
		void lock() {}
		void unlock() {}
	};

	struct DummySharedLock
	{
		void lock() {}
		void unlock() {}
		void lock_shared() {}
		void unlock_shared() {}
	};

} // FrameGraph
