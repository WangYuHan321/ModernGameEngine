#pragma once

#include "../CompileTime/TypeTraits.h"
#include "../Containers/FixedArray.h"
#include "../ThreadSafe/DummyLock.h"
#include "../ThreadSafe/AtomicPtr.h"
#include "../Math/Math.h"
#include "../Memory/UntypedAllocator.h"
#include "../Memory/MemUtils.h"
#include "../Math/BitMath.h"
#include "../Algorithms/Cast.h"

namespace FrameGraph
{
	//
	// Chunked Indexed Pool
	//
	// 分块索引池：按 ChunkSize 分块存储 Value，空闲 index 复用；
	// Assign / Unassign 由 AssignOpGuard 保护（通常为 Mutex）。
	//

	template <typename ValueType,
			  typename IndexType,
			  size_t ChunkSize,
			  size_t MaxChunks = 16,
			  typename AllocatorType = UntypedAlignedAllocator,
			  typename AssignOpGuard = DummyLock,
			  template <typename T> class AtomicChunkPtr = NonAtomicPtr>
	struct ChunkedIndexedPool final
	{
		STATIC_ASSERT(ChunkSize > 0 and MaxChunks > 0);
		STATIC_ASSERT(IsPowerOfTwo(ChunkSize));

	public:
		using Self = ChunkedIndexedPool<ValueType, IndexType, ChunkSize, MaxChunks, AllocatorType, AssignOpGuard, AtomicChunkPtr>;
		using Index_t = IndexType;
		using Value_t = ValueType;
		using Allocator_t = AllocatorType;

	private:
		static constexpr size_t MaxSize = ChunkSize * MaxChunks;

		using RawIndex_t = Local::Conditional<(MaxSize > std::numeric_limits<uint32_t>::max()), uint64_t,
			Local::Conditional<(MaxSize > std::numeric_limits<uint16_t>::max()), uint32_t, uint16_t>>;

		using IndexCountArray_t = FixedArray<RawIndex_t, MaxChunks>;
		using IndexChunk = StaticArray<RawIndex_t, ChunkSize>;
		using ValueChunk = StaticArray<Value_t, ChunkSize>;

		using IndexChunks_t = StaticArray<IndexChunk*, MaxChunks>;
		using ValueChunks_t = StaticArray<AtomicChunkPtr<ValueChunk>, MaxChunks>;

	private:
		mutable AssignOpGuard _assignOpGuard;
		IndexCountArray_t _indexCount;
		IndexChunks_t _indices{};
		ValueChunks_t _values{};
		Allocator_t _alloc;

	public:
		ChunkedIndexedPool(const Self&) = delete;
		ChunkedIndexedPool(Self&&) = default;

		Self& operator = (const Self&) = delete;
		Self& operator = (Self&&) = default;

		explicit ChunkedIndexedPool(const Allocator_t& alloc = Allocator_t()) :
			_alloc{ alloc }
		{
			_indices.fill(null);

			for (auto& chunk : _values)
				chunk.Store(null);

			_indexCount.push_back(ChunkSize);
			_CreateChunk(0);
		}

		~ChunkedIndexedPool()
		{
			Release();
		}

		void Release()
		{
			EXLOCK(_assignOpGuard);

			_indexCount.clear();

			for (auto& chunk : _indices)
			{
				if (chunk)
				{
					chunk->~IndexChunk();
					_alloc.Deallocate(chunk, SizeOf<IndexChunk>, AlignOf<IndexChunk>);
					chunk = null;
				}
			}

			for (auto& chunk : _values)
			{
				auto* value = chunk.Load();
				if (value)
				{
					value->~ValueChunk();
					_alloc.Deallocate(value, SizeOf<ValueChunk>, AlignOf<ValueChunk>);
					chunk.Store(null);
				}
			}
		}

		// 取一个空闲 index；当前块用尽时按需分配新 chunk（最多 MaxChunks 块）
		GND bool Assign(OUT Index_t& index)
		{
			EXLOCK(_assignOpGuard);

			for (size_t i = 0, count = _indexCount.size(); i < count; ++i)
			{
				auto& idx_count = _indexCount[i];

				if (idx_count > 0)
				{
					ASSERT(_indices[i]);
					index = Index_t((*_indices[i])[--idx_count]);
					return true;
				}
			}

			if (_indexCount.size() == _indexCount.capacity())
			{
				ASSERT(!"out of memory!");
				return false;
			}

			const size_t chunk_idx = _indexCount.size();
			_indexCount.push_back(ChunkSize);
			_CreateChunk(chunk_idx);

			index = Index_t((*_indices[chunk_idx])[--_indexCount[chunk_idx]]);
			return true;
		}

		// 归还 index 到空闲栈，供后续 Assign 复用（不析构 Value_t）
		void Unassign(Index_t index)
		{
			EXLOCK(_assignOpGuard);
			_Unassign(index);
		}

		GND Value_t& operator [] (Index_t index)
		{
			const Index_t chunk_idx = index / Index_t(ChunkSize);
			const Index_t idx = index % Index_t(ChunkSize);
			ASSERT(size_t(chunk_idx) < _indexCount.size());

			auto* val_chunk = _values[chunk_idx].Load();
			ASSERT(val_chunk);

			return (*val_chunk)[size_t(idx)];
		}

		GND Value_t const& operator [] (Index_t index) const
		{
			const Index_t chunk_idx = index / Index_t(ChunkSize);
			const Index_t idx = index % Index_t(ChunkSize);
			ASSERT(size_t(chunk_idx) < _indexCount.size());

			auto* val_chunk = _values[chunk_idx].Load();
			ASSERT(val_chunk);

			return (*val_chunk)[size_t(idx)];
		}

		GND size_t size() const
		{
			EXLOCK(_assignOpGuard);
			return _indexCount.size() * ChunkSize;
		}

		GND static constexpr size_t capacity() { return MaxChunks * ChunkSize; }

		GND bool empty() const
		{
			EXLOCK(_assignOpGuard);

			for (auto cnt : _indexCount)
			{
				if (cnt != ChunkSize)
					return false;
			}
			return true;
		}

		GND BytesU DynamicSize() const
		{
			EXLOCK(_assignOpGuard);
			BytesU sz{ sizeof(*this) };

			for (auto& idx : _indices)
				sz += (idx ? sizeof(*idx) : 0_b);

			for (auto& chunk : _values)
				sz += (chunk.Load() ? sizeof(ValueChunk) : 0_b);

			return sz;
		}

	private:
		// 新建 chunk：indices 栈填入 [chunkBase .. chunkBase+ChunkSize-1]，values 用 AtomicPtr 发布
		void _CreateChunk(size_t chunkIndex)
		{
			auto* idx_chunk = Cast<IndexChunk>(_alloc.Allocate(SizeOf<IndexChunk>, AlignOf<IndexChunk>));
			_indices[chunkIndex] = idx_chunk;

			for (size_t i = 0; i < ChunkSize; ++i)
			{
				const size_t idx = (ChunkSize - 1 - i) + (chunkIndex * ChunkSize);
				ASSERT(idx <= std::numeric_limits<RawIndex_t>::max());
				(*idx_chunk)[i] = RawIndex_t(idx);
			}

			ASSERT(_values[chunkIndex].Load() == null);

			auto* val_chunk = Cast<ValueChunk>(_alloc.Allocate(SizeOf<ValueChunk>, AlignOf<ValueChunk>));
			PlacementNew<ValueChunk>(val_chunk);
			_values[chunkIndex].Store(val_chunk);
		}

		void _Unassign(Index_t index)
		{
			const Index_t chunk_idx = index / Index_t(ChunkSize);

			ASSERT(size_t(chunk_idx) < _indexCount.size());
			ASSERT(index >= Index_t(chunk_idx * ChunkSize) and index < Index_t((chunk_idx + 1) * ChunkSize));

			auto& idx_count = _indexCount[size_t(chunk_idx)];
			auto& indices = *_indices[size_t(chunk_idx)];
			ASSERT(idx_count < ChunkSize);

			indices[idx_count++] = RawIndex_t(index);
		}
	};

} // FrameGraph
