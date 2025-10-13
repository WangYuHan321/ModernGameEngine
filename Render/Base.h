#pragma once

//                   2025 10.13
//                   WangYuHan
//                   view by LVK Vulkan Renderer

#include <assert.h>
#include <cstdint>
#include <vector>

namespace Renderer
{
  namespace Util
  {
    template<typename ObjectType>
    class Handle final
    {
      public:
        Handle() = default;
        
        bool empty() const {
          return mGen == 0;
        }

        bool valid() const{
          return mGen != 0;
        }

        uint32_t index() const{
          return mIndex;
        }

        uint32_t gen() const {
            return mGen;
        }

        void* indexAsVoid() const {
            return reinterpret_cast<void*>(static_cast<ptrdiff_t>(mIndex));
        }

        bool operator==(const Handle<ObjectType>& other) const {
            return mIndex == other.mIndex && mGen == other.mGen;
        }

        bool operator!=(const Handle<ObjectType>& other) const {
            return mIndex != other.mIndex || mGen != other.mGen;
        }

          // allow conditions 'if (handle)'
        explicit operator bool() const {
            return mGen != 0;
        }

      private:
        Handle(uint32_t index, uint32_t gen) : mIndex(index), mGen(gen) {};

        template<typename ObjectType, typename ImplObjectType>
        friend class Pool;

        uint32_t mIndex = 0;
        uint32_t mGen = 0;
    };
  }
}