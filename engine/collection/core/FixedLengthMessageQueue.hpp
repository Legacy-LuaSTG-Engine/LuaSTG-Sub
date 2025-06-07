#pragma once
#include <array>
#include <atomic>
#include <semaphore>

namespace core {
	template<typename Type, size_t Size>
	class FixedLengthMessageQueue {
	public:
		void put(Type const& value) {
			m_space_available.acquire();
			m_data[m_write_index.fetch_add(1) % m_data.size()] = value;
			m_data_available.release();
		}

		template <typename Rep, typename Period>
		[[nodiscard]] bool put(Type const& value, std::chrono::duration<Rep, Period> const timeout) {
			if (m_space_available.try_acquire_for(timeout)) {
				m_data[m_write_index.fetch_add(1) % m_data.size()] = value;
				m_data_available.release();
				return true;
			}
			return false;
		}

		void get(Type& value) {
			m_data_available.acquire();
			value = m_data[m_read_index.fetch_add(1) % m_data.size()];
			m_space_available.release();
		}

		template <typename Rep, typename Period>
		[[nodiscard]] bool get(Type& value, std::chrono::duration<Rep, Period> const timeout) {
			if (m_data_available.try_acquire_for(timeout)) {
				value = m_data[m_read_index.fetch_add(1) % m_data.size()];
				m_space_available.release();
				return true;
			}
			return false;
		}

	private:
		std::array<Type, Size> m_data{};
		std::atomic_size_t m_write_index{ 0 };
		std::atomic_size_t m_read_index{ 0 };
		std::counting_semaphore<Size> m_space_available{ Size };
		std::counting_semaphore<Size> m_data_available{ 0 };
	};
}
