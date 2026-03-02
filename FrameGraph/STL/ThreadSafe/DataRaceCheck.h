#pragma once
#include "../Common.h"

namespace FrameGraph
{
	//
	//Data Race Check
	//

	/*
	它用于检测数据竞争，而不是真正的互斥锁。当多个线程同时访问应该被保护的资源时，它会触发错误检查。

	工作原理
		核心思想
		记录当前持有"锁"的线程ID（哈希值）

		如果其他线程也想获取"锁"，就会触发检查错误

		支持同一个线程递归获取（不会报错）

		*/

	struct DataRaceCheck
	{
	//variables
	private:
		mutable Atomic<size_t> _state = 0;

	//method
	public:
		DataRaceCheck() {}

		GND bool Lock() const
		{
			// 1. 获取当前线程ID的哈希值作为唯一标识
			const size_t id = size_t(HashOf(std::this_thread::get_id()));

			// 2. 读取当前状态（谁持有锁）
			size_t curr = _state.load(memory_order_acquire);

			// 3. 检查是否当前线程已经持有锁（递归情况）
			if (curr == id)
				return true;  // 递归锁，直接返回成功

			// 4. 尝试获取锁：将_state从0改为id
			curr = 0;
			bool locked = _state.compare_exchange_strong(
				INOUT curr,      // 期望值0，实际值会存到这里
				id,              // 想要设置的值
				memory_order_relaxed
			);

			// 5. 检查是否成功
			CHECK_ERR(curr == 0);  // 如果curr != 0，说明被其他线程持有
			CHECK_ERR(locked);      // 确保CAS成功

			return true;
		}

		void Unlock() const
		{
			_state.store(0, memory_order_relaxed);
		}

	};

	//
	// Read/Write Data Race Check
	//

	struct RWDataRaceCheck
	{
	//variables
	private:
		mutable std::recursive_mutex _lockWrite;
		mutable Atomic<int>          _readCounter{ 0 };

	//method
	public:
		RWDataRaceCheck() {}

		GND bool LockExclusive()
		{
			
		}


	};
}










