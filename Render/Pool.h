#pragma once

//                   2025 10.13
//                   WangYuHan
//                   view by LVK Vulkan Renderer

#include <assert.h>
#include <cstdint>
#include <vector>
#include "Base.h"

using namespace Render::Util;

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
			if(handle.Empty())
				return;
		
			assert(mNumObjects > 0);
			const uint32_t index = handle.Index();
			assert(index < mObjects.size());
			assert(handle.Gen() == mObjects[index].mGen); // double deletion
			mObjects[index].mObj = ImplObjectType{};
			mObjects[index].mGen++;
			mObjects[index].mNextFree = mFreeListHead;
			freeListHead_ = index;
			mNumObjects--;
		}
		
		const ImplObjectType* get(Handle<ObjectType> handle) const
		{
			if (handle.Empty())
				return nullptr;
	
			const uint32_t index = handle.Index();
			assert(index < mObjects.size());
			assert(handle.Gen() == mObjects[index].mGen); // accessing deleted object
			
			return &mObjects[index].mObj;
		}
		
		ImplObjectType* Get(Handle<ObjectType> handle) {
		if (handle.Empty())
			return nullptr;
	
			const uint32_t index = handle.Index();
			assert(index < mObjects.size());
			assert(handle.gen() == mObjects[index].mGen); // accessing deleted object
			return &mObjects[index].mObj;
		}
		
		Handle<ObjectType> GetHandle(uint32_t index) const {
			assert(index < mObjects.size());
			if (index >= mObjects.size())
				return {};
	
			return Handle<ObjectType>(index, mObjects[index].mGen);
		}
		
		Handle<ObjectType> FindObject(const ImplObjectType* obj) {
		if (!obj)
			return {};
	
			for (size_t idx = 0; idx != objects_.size(); idx++) {
				if (mObjects[idx].mObj == *obj) {
					return Handle<ObjectType>((uint32_t)idx, mObjects[idx].mGen);
				}
			}
	
			return {};
		}
	
		void Clear() {
			mObjects.clear();
			freeListHead_ = kListEndSentinel;
			mNumObjects = 0;
		}
	
		uint32_t NumObjects() const {
			return mNumObjects;
		}
  };
}
