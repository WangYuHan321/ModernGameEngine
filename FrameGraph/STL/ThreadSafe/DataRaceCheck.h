#pragma once

// ============================================================================
// DataRaceCheck / RWDataRaceCheck — Debug 下的数据竞争检测器
// ============================================================================
//
// 【它是什么】
//   不是真正的互斥锁，而是 Debug 构建下的“访问守卫”：
//   多线程同时读写同一对象时触发 CHECK_ERR，帮助尽早发现数据竞争。
//   Release 构建（未定义 FG_ENABLE_DATA_RACE_CHECK）中为空实现，零开销。
//
// 【何时启用】
//   FG_DEBUG 时 STL/Config.h 会自动定义 FG_ENABLE_DATA_RACE_CHECK。
//
// 【两种类型】
//   DataRaceCheck     — 互斥访问（同一时刻只允许一个线程，支持同线程递归）
//   RWDataRaceCheck   — 读写锁语义（多读单写）
//
// 【基本用法】
//   1. 在类里声明成员：
//        RWDataRaceCheck _drCheck;   // 有 const 读接口 + 非 const 写接口
//        DataRaceCheck   _drCheck;   // 只有互斥访问
//
//   2. 写/改数据（Create / Destroy / 赋值）时用 EXLOCK：
//        bool Create(...) {
//            EXLOCK(_drCheck);          // 等价于 std::unique_lock{ _drCheck }
//            _desc = desc;
//            ...
//        }
//
//   3. 只读访问（const 成员函数）时用 SHAREDLOCK（RWDataRaceCheck）：
//        BufferDesc const& Description() const {
//            SHAREDLOCK(_drCheck);      // 等价于 std::shared_lock{ _drCheck }
//            return _desc;
//        }
//
//   4. DataRaceCheck（非 RW）读写都用 EXLOCK：
//        void Foo()       { EXLOCK(_drCheck); ... }
//        int  Bar() const { EXLOCK(_drCheck); ... }
//
// 【项目内参考】
//   VBuffer / VMemoryObj  — RWDataRaceCheck + EXLOCK(写) / SHAREDLOCK(读)
//   VDevice               — DataRaceCheck
//   VResourceManager      — 仍用 Mutex 管理池；与 _drCheck 是不同层级
//
// 【注意】
//   - EXLOCK / SHAREDLOCK 定义在 STL/Defines.h，作用域结束自动解锁（RAII）
//   - const 成员函数里不能 EXLOCK 非 mutable 的 DataRaceCheck；
//     RWDataRaceCheck 的读锁接口是 const，成员需加 mutable
//   - 不要与 Mutex 混用保护同一份数据，选一种即可
//
// ============================================================================

#include "../Config.h"
#include "../Common.h"
#include <shared_mutex>

#ifdef FG_ENABLE_DATA_RACE_CHECK

#include <atomic>
#include <mutex>
#include <thread>

namespace FrameGraph
{
	// 互斥守卫：任意时刻只允许一个线程进入（同线程可递归）
	struct DataRaceCheck
	{
	private:
		mutable Atomic<size_t> _state = 0;

	public:
		DataRaceCheck() {}

		GND bool Lock() const
		{
			const size_t id = size_t(std::hash<std::thread::id>{}(std::this_thread::get_id()));
			size_t curr = _state.load(memory_order_acquire);

			if (curr == id)
				return true; // 同线程递归

			curr = 0;
			const bool locked = _state.compare_exchange_strong(INOUT curr, id, memory_order_relaxed);
			CHECK_ERR(curr == 0); // 已被其他线程占用 → 数据竞争
			CHECK_ERR(locked);
			return true;
		}

		void Unlock() const
		{
			_state.store(0, memory_order_relaxed);
		}

		// 供 EXLOCK(obj) / std::unique_lock 使用
		void lock() const { (void)Lock(); }
		void unlock() const { Unlock(); }
	};

	// 读写守卫：允许多个读线程，写线程独占
	struct RWDataRaceCheck
	{
	private:
		mutable std::recursive_mutex _lockWrite;
		mutable Atomic<int> _readCounter{ 0 };

	public:
		RWDataRaceCheck() {}

		// 写锁：Create / Destroy / 修改成员
		GND bool LockExclusive()
		{
			const bool locked = _lockWrite.try_lock();
			CHECK_ERR(locked);

			int expected = _readCounter.load(memory_order_acquire);
			CHECK_ERR(expected <= 0);

			_readCounter.compare_exchange_strong(INOUT expected, expected - 1, memory_order_relaxed);
			CHECK_ERR(expected <= 0);
			return true;
		}

		GND void UnlockExclusive()
		{
			_readCounter.fetch_add(1, memory_order_relaxed);
			_lockWrite.unlock();
		}

		// 读锁：const 查询接口
		GND bool LockShared() const
		{
			int expected = 0;
			bool locked = false;

			do {
				locked = _readCounter.compare_exchange_weak(INOUT expected, expected + 1, memory_order_relaxed);

				if (expected < 0 && _lockWrite.try_lock())
				{
					_lockWrite.unlock();
					return false;
				}

				CHECK_ERR(expected >= 0);
			} while (!locked);

			return true;
		}

		void UnlockShared() const
		{
			_readCounter.fetch_sub(1, memory_order_relaxed);
		}

		// 供 EXLOCK / SHAREDLOCK 使用
		void lock() { (void)LockExclusive(); }
		void unlock() { UnlockExclusive(); }
		void lock_shared() const { (void)LockShared(); }
		void unlock_shared() const { UnlockShared(); }
	};

} // FrameGraph

#else

namespace FrameGraph
{
	// Release：空实现，EXLOCK / SHAREDLOCK 仍可写，无运行时开销
	struct DataRaceCheck
	{
		void lock() const {}
		void unlock() const {}
	};

	struct RWDataRaceCheck
	{
		void lock() {}
		void unlock() {}

		void lock_shared() const {}
		void unlock_shared() const {}
	};

} // FrameGraph

#endif
