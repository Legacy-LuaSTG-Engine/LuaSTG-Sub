#pragma once
#include <cassert>

namespace cpp
{
	template <typename T, size_t N>
	class fixed_object_pool
	{
	private:
		size_t _free_size;
		size_t _free[N];
		bool   _used[N];
		T      _data[N];
	public:
		bool alloc(size_t& id) noexcept
		{
			if (_free_size > 0)
			{
				_free_size--;
				id = _free[_free_size];
				_used[id] = true;
				return true;
			}
			else
			{
				id = static_cast<size_t>(-1);
				return false;
			}
		};
		void free(size_t id) noexcept
		{
			if (id < N && _used[id])
			{
				_used[id] = false;
				_free[_free_size] = id;
				_free_size++;
			}
			else
			{
				assert(false);
			}
		};
		T* object(size_t id) noexcept
		{
			if (id < N && _used[id])
			{
				return &_data[id];
			}
			else
			{
				return nullptr;
			}
		};
		size_t size() const noexcept
		{
			return N - _free_size;
		};
		size_t max_size() const noexcept
		{
			return N;
		};
		void clear() noexcept
		{
			_free_size = N;
			for (size_t idx_ = 0; idx_ < N; idx_++)
			{
				_free[idx_] = (N - 1) - idx_;
				_used[idx_] = false;
			}
		};
	public:
		fixed_object_pool() noexcept
		{
			clear();
		};
		~fixed_object_pool() noexcept
		{
		};
	};
};
