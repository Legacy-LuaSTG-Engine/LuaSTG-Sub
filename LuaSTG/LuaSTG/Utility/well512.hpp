#pragma once
#include <cstdint>
#include <string_view>
#include <string>

namespace random {
	// WELL512 随机数算法
	// 摘自《游戏编程精粹 7》
	class well512 {
	public:
		well512();
		explicit well512(uint32_t const seed) noexcept {
			this->seed(seed);
		}

		[[nodiscard]] uint32_t seed() const noexcept { return m_seed; }
		void seed(uint32_t seed) noexcept;
		[[nodiscard]] uint32_t next() noexcept;
		[[nodiscard]] uint32_t next(uint32_t const bound) noexcept {
			return next() % (bound + 1);
		}
		[[nodiscard]] int32_t integer(int32_t const a, int32_t const b) noexcept {
			auto const range = static_cast<int64_t>(b) - static_cast<int64_t>(a);
			return a + next(static_cast<uint32_t>(range));
		}
		[[nodiscard]] float number() noexcept {
			return static_cast<float>(next(1000000)) / 1000000.0f;
		}
		[[nodiscard]] float number(float const bound) noexcept {
			return number() * bound;
		}
		[[nodiscard]] float number(float const a, float const b) noexcept {
			return a + number() * (b - a);
		}
		[[nodiscard]] int32_t sign() noexcept {
			return static_cast<int32_t>(next(1) * 2) - 1;
		}
		[[nodiscard]] std::string serialize() const;
		[[nodiscard]] bool deserialize(std::string_view data);
	private:
		uint32_t m_state[16]{};
		uint32_t m_index{};
		uint32_t m_seed{};
	};
}
