#pragma once

# include <variant>

namespace FrameGraph
{
	//
	//Array view
	//

	template<typename T>
	struct ArrayView
	{
		// types
	public:
		using value_type = T;
		using iterator = T*;
		using const_iterator = T const*;

		// variables
	private:
		union {
			T const* _array;
			T const		(*_dbgView)[400];		// debug viewer, don't use this field!
		};
		size_t		_count = 0;

		//method
	public:

		ArrayView() : _array{ null } {}

		ArrayView(T const* ptr, size_t count) : _array{ ptr }, _count{ count }
		{
			ASSERT((_count == 0) or (_array != null));
		}

		ArrayView(std::initializer_list<T> list) : _array{ list.begin() }, _count{ list.size() } {}

		template <typename AllocT>
		ArrayView(const std::vector<T, AllocT>& vec) : _array{ vec.data() }, _count{ vec.size() }
		{
			ASSERT((_count == 0) or (_array != null));
		}

		template <size_t S>
		ArrayView(const StaticArray<T, S>& arr) : _array{ arr.data() }, _count{ arr.size() } {}

		template <size_t S>
		ArrayView(const T(&arr)[S]) : _array{ arr }, _count{ S } {}

		GND explicit operator Array<T>()			const { return Array<T>{ begin(), end() }; }

		GND size_t			size()					const { return _count; }
		GND bool			empty()				const { return _count == 0; }
		GND T const* data()					const { return _array; }

		GND T const& operator [] (size_t i)	const { ASSERT(i < _count);  return _array[i]; }

		GND const_iterator	begin()				const { return _array; }
		GND const_iterator	end()					const { return _array + _count; }

		GND T const& front()				const { ASSERT(_count > 0);  return _array[0]; }
		GND T const& back()					const { ASSERT(_count > 0);  return _array[_count - 1]; }


		GND bool  operator == (ArrayView<T> rhs) const
		{
			if (size() != rhs.size())
				return false;

			for (size_t i = 0; i < size(); ++i) {
				if (not (_array[i] == rhs[i]))
					return false;
			}
			return true;
		}

		GND bool  operator >  (ArrayView<T> rhs) const
		{
			if (size() != rhs.size())
				return size() > rhs.size();

			for (size_t i = 0; i < size(); ++i)
			{
				if (_array[i] != rhs[i])
					return _array[i] > rhs[i];
			}
			return true;
		}

		GND bool  operator != (ArrayView<T> rhs) const { return not (*this == rhs); }
		GND bool  operator <  (ArrayView<T> rhs) const { return (rhs > *this); }
		GND bool  operator >= (ArrayView<T> rhs) const { return not (*this < rhs); }
		GND bool  operator <= (ArrayView<T> rhs) const { return not (*this > rhs); }


		GND ArrayView<T> section(size_t first, size_t count) const
		{
			return first < size() ?
				ArrayView<T>{ data() + first, Min(size() - first, count) } :
				ArrayView<T>{};
		}

	};

}

