#pragma once

namespace core {
	template<typename T, size_t N>
	class FixedObjectPool {
	public:
		FixedObjectPool() noexcept { clear(); }
		FixedObjectPool(FixedObjectPool const&) = delete;
		FixedObjectPool(FixedObjectPool&&) = delete;
		~FixedObjectPool() noexcept = default;

		FixedObjectPool& operator=(FixedObjectPool const&) = delete;
		FixedObjectPool& operator=(FixedObjectPool&&) = delete;

		bool alloc(size_t& id) noexcept {
			if (m_free_count > 0) {
				m_free_count--;
				id = m_free_indices[m_free_count];
				m_used[id] = true;
				m_free_indices[m_free_count] = static_cast<size_t>(-1);
				return true;
			}
			id = static_cast<size_t>(-1);
			return false;
		}

		void free(size_t const id) noexcept {
			if (id < N && m_used[id]) {
				m_used[id] = false;
				m_free_indices[m_free_count] = id;
				m_free_count++;
			}
		}

		T* object(size_t const id) noexcept {
			if (id < N && m_used[id]) {
				return &m_data[id];
			}
			return nullptr;
		}

		[[nodiscard]] size_t size() const noexcept { return N - m_free_count; }

		// ReSharper disable once CppMemberFunctionMayBeStatic
		[[nodiscard]] constexpr size_t capacity() const noexcept { return N; }

		void clear() noexcept {
			m_free_count = N;
			for (size_t idx = 0; idx < N; idx++) {
				m_free_indices[idx] = (N - 1) - idx;
			}
			for (auto& v : m_used) {
				v = false;
			}
		}

	private:
		size_t m_free_count{};
		size_t m_free_indices[N]{};
		bool m_used[N]{};
		T m_data[N]{};
	};
}
