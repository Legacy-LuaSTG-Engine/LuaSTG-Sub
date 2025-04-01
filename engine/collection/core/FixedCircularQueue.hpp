#pragma once
#include <cassert>
#include <array>

namespace core {
	//     front                back
	//       |                   |
	//       v                   v
	// | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | ...
	// | - | A | B | C | D | E | - | - | ...
	//       ^               ^
	//       |               |
	// first element         last element

	template <typename T, size_t MaxSize>
	class FixedCircularQueue {
	public:
		[[nodiscard]] T& operator[](size_t const idx) { assert(!empty()); return m_data[(idx + m_front) % MaxSize]; }

		[[nodiscard]] T const& operator[](size_t const idx) const { assert(!empty()); return m_data[(idx + m_front) % MaxSize]; }

		[[nodiscard]] bool empty() const noexcept { return m_size == 0; }

		[[nodiscard]] bool full() const noexcept { return m_size >= MaxSize; }

		[[nodiscard]] size_t size() const noexcept { return m_size; }

		// ReSharper disable once CppMemberFunctionMayBeStatic
		[[nodiscard]] constexpr size_t capacity() const noexcept { return MaxSize; }

		void clear() noexcept { m_front = 0; m_back = 0; m_size = 0; }

		void placementResize(size_t const size) noexcept { assert(size <= MaxSize); m_back = size; m_size = size; }

		[[nodiscard]] T& placementPushTail() noexcept {
			assert(!full());
			T& data = m_data[m_back];			// 1. store
			m_back = (m_back + 1) % MaxSize;	// 2. forward
			m_size += 1;
			return data;
		}
		
		[[nodiscard]] T& placementPushHead() noexcept {
			assert(!full());
			m_front = (m_front + MaxSize - 1) % MaxSize;	// 1. backward
			T& data = m_data[m_front];						// 2. store
			m_size += 1;
			return data;
		}

		void pushTail(T const& value) noexcept {
			assert(!full());
			m_data[m_back] = value;				// 1. store
			m_back = (m_back + 1) % MaxSize;	// 2. forward
			m_size += 1;
		}
		
		void pushHead(T const& value) noexcept {
			assert(!full());
			m_front = (m_front + MaxSize - 1) % MaxSize;	// 1. backward
			m_data[m_front] = value;						// 2. store
			m_size += 1;
		}

		[[maybe_unused]] T& popTail() {
			assert(!empty());
			m_back = (m_back + MaxSize - 1) % MaxSize;	// 1. backward
			T& data = m_data[m_back];					// 2. load
			m_size -= 1;
			return data;
		}

		[[maybe_unused]] T& popHead() {
			assert(!empty());
			T& data = m_data[m_front];			// 1. load
			m_front = (m_front + 1) % MaxSize;	// 2. forward
			m_size -= 1;
			return data;
		}

		[[nodiscard]] T& tail() noexcept { assert(!empty()); return m_data[m_back + MaxSize - 1]; }

		[[nodiscard]] T const& tail() const noexcept { assert(!empty()); return m_data[m_back + MaxSize - 1]; }

		[[nodiscard]] T& head() noexcept { assert(!empty()); return m_data[m_front]; }

		[[nodiscard]] T const& head() const noexcept { assert(!empty()); return m_data[m_front]; }
		
	private:
		std::array<T, MaxSize> m_data;
		size_t m_front{}; // 头部索引
		size_t m_back{}; // 下一个可置入的对象的索引 
		size_t m_size{}; // 已用空间
	};
}
