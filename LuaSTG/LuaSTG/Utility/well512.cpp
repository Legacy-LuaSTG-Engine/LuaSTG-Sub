#include "well512.hpp"
#include <ctime>
#include <ranges>
#include <algorithm>

using std::string_view_literals::operator ""sv;

namespace random {
	well512::well512() {
		seed(static_cast<uint32_t>(clock()));
	}

	void well512::seed(uint32_t const seed) noexcept {
		m_seed = seed;
		m_index = 0;
		// 线性同余产生种子
		constexpr uint32_t mask = ~0u;
		m_state[0] = m_seed & mask;
		for (uint32_t i = 1; i < 16; ++i) {
			m_state[i] = (1812433253u * (m_state[i - 1] ^ (m_state[i - 1] >> 30)) + i) & mask;
		}
	}
	[[nodiscard]] uint32_t well512::next() noexcept {
		uint32_t a, b, c, d;
		a = m_state[m_index];
		c = m_state[(m_index + 13) & 15];
		b = a ^ c ^ (a << 16) ^ (c << 15);
		c = m_state[(m_index + 9) & 15];
		c ^= (c >> 11);
		a = m_state[m_index] = b ^ c;
		d = a ^ ((a << 5) & 0xDA442D24u);
		m_index = (m_index + 15) & 15;
		a = m_state[m_index];
		m_state[m_index] = a ^ b ^ d ^ (a << 2) ^ (b << 18) ^ (c << 28);
		return m_state[m_index];
	}
	std::string well512::serialize() const {
		return std::format("well512-"
			"{}-"
			"{}-{}-{}-{}-"
			"{}-{}-{}-{}-"
			"{}-{}-{}-{}-"
			"{}-{}-{}-{}-",
			m_index,
			m_state[0], m_state[1], m_state[2], m_state[3],
			m_state[4], m_state[5], m_state[6], m_state[7],
			m_state[8], m_state[9], m_state[10], m_state[11],
			m_state[12], m_state[13], m_state[14], m_state[15]
		);
	}
	bool well512::deserialize(std::string_view const data) {
		constexpr auto head("well512-"sv);
		if (!data.starts_with(head)) {
			return false;
		}
		std::string tail(data.substr(head.size()));
		std::ranges::replace(tail, '-', ' ');
		std::stringstream iss(tail);
		try {
			uint32_t index{};
			uint32_t state[16]{};
			iss >> index
				>> state[0] >> state[1] >> state[2] >> state[3]
				>> state[4] >> state[5] >> state[6] >> state[7]
				>> state[8] >> state[9] >> state[10] >> state[11]
				>> state[12] >> state[13] >> state[14] >> state[15];
			m_index = index;
			std::memcpy(m_state, state, sizeof(m_state));
			return true;
		}
		catch (std::exception const& e) {
			std::ignore = e;
		}
		return false;
	}
}
