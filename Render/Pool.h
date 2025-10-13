#pragma once

//                   2025 10.13
//                   WangYuHan
//                   view by LVK Vulkan Renderer

#include <assert.h>
#include <cstdint>
#include <vector>
#include "Base.h"

using namespace Renderer::Util;

namespace Renderer
{
  template<typename ObjectType, typename ImplObjectType>
  class Pool
  {
	private:
		static constexpr uint32_t kListEndSentinel = 0xffffffff;
		struct PoolEntry
		{
		explicit  PoolEntry(ImplObjectType& obj) : mObj( std::move(obj) ) {};
		ImplObjectType mObj = {}
		uint32_t mGen = 1;
		uint32_t mNextFree = kListEndSentinel;
		};

		uint32_t mFreeListHead = kListEndSentinel;
		uint32_t mNumObjects = 0;

    public:
		std::vector<PoolEntry> mObjects;
	
		Handle<ObjectType> Create(ImplObjectType&& obj)
		{
		uint32_t idx = 0;
		if(mFreeListHead != kListEndSentinel)
		{
			idx = mFreeListHead;
			mFreeListHead =  mObjects[idx].mNextFree;
			mObjects[idx].mObj = std::move(obj);
		}else
		{
			idx = (uint32_t)mObjects.size();
			mObjects.emplace_back(obj);
		}
		mNumObjects++;
		return Handle<ObjectType>(idx, mObjects[idx].mGen);
		}
	
		void Destroy(Handle<ObjectType> handle)
		{
			if(handle.empty())
				return;
		
			assert(mNumObjects > 0);
			assert(numObjects_ > 0); // double deletion
			const uint32_t index = handle.index();
			assert(index < objects_.size());
			assert(handle.gen() == objects_[index].gen_); // double deletion
			objects_[index].obj_ = ImplObjectType{};
			objects_[index].gen_++;
			objects_[index].nextFree_ = freeListHead_;
			freeListHead_ = index;
			numObjects_--;
		}
		
		const ImplObjectType* get(Handle<ObjectType> handle) const
		{
			if (handle.empty())
				return nullptr;
	
			const uint32_t index = handle.index();
			assert(index < objects_.size());
			assert(handle.gen() == objects_[index].gen_); // accessing deleted object
			
			return &objects_[index].obj_;
		}
		
		ImplObjectType* get(Handle<ObjectType> handle) {
		if (handle.empty())
			return nullptr;
	
			const uint32_t index = handle.index();
			assert(index < objects_.size());
			assert(handle.gen() == objects_[index].gen_); // accessing deleted object
			return &objects_[index].obj_;
		}
		
		Handle<ObjectType> getHandle(uint32_t index) const {
			assert(index < objects_.size());
			if (index >= objects_.size())
				return {};
	
			return Handle<ObjectType>(index, objects_[index].gen_);
		}
		
		Handle<ObjectType> findObject(const ImplObjectType* obj) {
		if (!obj)
			return {};
	
			for (size_t idx = 0; idx != objects_.size(); idx++) {
				if (objects_[idx].obj_ == *obj) {
					return Handle<ObjectType>((uint32_t)idx, objects_[idx].gen_);
				}
			}
	
			return {};
		}
	
		void clear() {
			objects_.clear();
			freeListHead_ = kListEndSentinel;
			numObjects_ = 0;
		}
	
		uint32_t numObjects() const {
			return numObjects_;
		}
  }	
}
