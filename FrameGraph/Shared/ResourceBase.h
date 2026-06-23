#pragma once



#include "../Public/IDs.h"

#include "../STL/Common.h"



namespace FrameGraph

{

	//

	// Resource Wrapper（池内槽位）

	//

	// 包装 VBuffer / VMemoryObj 等实际对象，提供：

	//   - 生命周期状态（Initial / Failed / Created）

	//   - 引用计数（AddRef / ReleaseRef）

	//   - instanceId（Destroy 后递增，使旧 RawXxxID 失效）

	// 由 VResourceManager 的 Mutex 保护池级并发；ResourceBase 自身无锁。

	//



	template <typename ResType>

	class alignas(64) ResourceBase final

	{

	public:

		enum class EState : uint

		{

			Initial = 0,  // 未创建或已 Destroy，槽位可复用

			Failed,       // Create 失败

			Created,      // 资源有效

		};



		using Self = ResourceBase<ResType>;

		using Resource_t = ResType;

		using InstanceID_t = RawBufferID::InstanceID_t;



	private:

		// Destroy 时递增，写入 RawXxxID 的 generation 部分

		Atomic<uint> _instanceId = 0;



		Atomic<EState> _state = EState::Initial;



		// 实际 Vulkan 资源对象（VBuffer / VMemoryObj …）

		ResType _data;



		// 引用计数；归零时由 ResourceManager 调用 Destroy

		mutable Atomic<int> _refCounter = 0;



	public:

		ResourceBase() {}

		ResourceBase(Self&&) = delete;

		ResourceBase(const Self&) = delete;

		~ResourceBase() { ASSERT(IsDestroyed()); }



		// 增加引用（Create 成功后由 ResourceManager 调用）

		void AddRef() const

		{

			_refCounter.fetch_add(1, std::memory_order_relaxed);

		}



		// 减少引用，返回 true 表示计数归零、应 Destroy

		GND bool ReleaseRef(int refCount) const

		{

			return _refCounter.fetch_sub(refCount, std::memory_order_relaxed) == refCount;

		}



		GND bool IsCreated() const { return _GetState() == EState::Created; }

		GND bool IsDestroyed() const { return _GetState() <= EState::Failed; }



		// 当前 generation，用于 ID 校验

		GND InstanceID_t GetInstanceID() const { return InstanceID_t(_instanceId.load(std::memory_order_relaxed)); }

		GND int GetRefCount() const { return _refCounter.load(std::memory_order_relaxed); }



		GND ResType& Data() { return _data; }

		GND ResType const& Data() const { return _data; }



		// 转发到 _data.Create，成功则 _state = Created

		template <typename ...Args>

		bool Create(Args&& ...args)

		{

			ASSERT(IsDestroyed());

			ASSERT(GetRefCount() == 0);



			const bool result = _data.Create(std::forward<Args>(args)...);



			_state.store(result ? EState::Created : EState::Failed, std::memory_order_release);

			return result;

		}



		// 转发到 _data.Destroy，重置计数并递增 instanceId

		template <typename ...Args>

		void Destroy(Args&& ...args)

		{

			ASSERT(not IsDestroyed());



			_data.Destroy(std::forward<Args>(args)...);



			_refCounter.store(0, std::memory_order_relaxed);

			_state.store(EState::Initial, std::memory_order_relaxed);

			_instanceId.fetch_add(1, std::memory_order_release);

		}



	private:

		GND EState _GetState() const { return _state.load(std::memory_order_relaxed); }

	};



} // FrameGraph


