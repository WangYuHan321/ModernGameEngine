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

		//独占锁
		GND bool LockExclusive()
		{
			//我要开始修改数据了，此时绝对不能有其他线程在读或写。
			bool	locked = _lockWrite.try_lock();
			CHECK_ERR(locked);	// 确保当前没有其他线程持有写锁。如果能锁住，说明你是唯一的写者。

			int		expected = _readCounter.load(memory_order_acquire);
			CHECK_ERR(expected <= 0);	// 断言 expected <= 0（即没有读锁）

			//通过 expected-1 将其设为一个负数（比如 -1），用这个特殊值来标记“现在写锁被持有了”。   
			// compare_exchange_strong  CPU 提供 LOCK CMPXCHG 指令，在执行期间锁住内存总线或缓存行  锁住的时间是纳秒级，只针对这一个内存地址
			_readCounter.compare_exchange_strong(INOUT expected, expected - 1, memory_order_relaxed);
			CHECK_ERR(expected <= 0);	// 断言 expected <= 0 检查的是 CAS 操作失败时，expected 被更新成的那个值。

			/*
			compare_exchange_strong 的语义是：

			如果 _readCounter 当前值等于 expected（也就是进入时的值），则将其设为 expected - 1，返回 true

			如果不等于，则将 expected 更新为 _readCounter 的当前值，返回 false
			*/

			return true;
		}

		GND UnlockExclusive()
		{
			_readCounter.fetch_add(1, memory_order_relaxed);	// 将 _readCounter 加回 1，表示写锁被释放了。
			_lockWrite.unlock();	// 释放写锁
		}


		GND bool LockShared() const
		{
			int expected = 0;           // 期望值：希望 _readCounter 当前是 0
			bool locked = false;        // 是否成功获得锁

			do {
				// 尝试原子操作：如果 _readCounter == expected(0)，则将其设为 expected+1(1)
				// 如果失败，expected 会被更新为 _readCounter 的当前值
				locked = _readCounter.compare_exchange_weak(INOUT expected, expected + 1,
					memory_order_relaxed);

				// 关键检查：如果发现有写锁(expected < 0) 且 当前线程能获取写锁  CAS 失败
				if (expected < 0 and _lockWrite.try_lock())
				{
					_lockWrite.unlock();  // 立即释放写锁
					return false;         // 返回失败，不要调用 UnlockShared
				}

				// 断言：确保 expected 不是负数（即没有其他线程持有写锁）
				CHECK_ERR(expected >= 0);// -1 >= 0? false → 断言触发！

			} while (not locked);  // 如果 CAS 失败，重试

			return true;  // 成功获取读锁
		}

		void UnlockShared() const
		{
			_readCounter.fetch_sub(1, memory_order_relaxed);
		}
	};
}










