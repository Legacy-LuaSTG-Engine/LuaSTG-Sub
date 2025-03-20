#include "core/UUID.hpp"
#include <limits>
#include <random>
#include <chrono>

namespace {
	constexpr char hex[16]{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	constexpr uint8_t fromHex(char const c) noexcept {
		switch (c) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'A': return 10;
		case 'B': return 11;
		case 'C': return 12;
		case 'D': return 13;
		case 'E': return 14;
		case 'F': return 15;
		case 'a': return 10;
		case 'b': return 11;
		case 'c': return 12;
		case 'd': return 13;
		case 'e': return 14;
		case 'f': return 15;
		default: return 0;
		}
	}
	constexpr uint8_t fromHex(char const high, char const low) noexcept {
		return static_cast<uint8_t>(fromHex(high) << 4 | fromHex(low));
	}
}

namespace core {
	std::string UUID::toString(bool const simple) const {
		std::string buffer;
		buffer.reserve(36);
		for (size_t i = 0; i < 16; i += 1) {
			buffer.push_back(hex[bytes[i] >> 4 & 0xf]);
			buffer.push_back(hex[bytes[i] & 0xf]);
			if (!simple && (i == 3 || i == 5 || i == 7 || i == 9)) {
				buffer.push_back('-');
			}
		}
		return buffer;
	}

	UUID UUID::version4() {
		std::random_device src;
		std::uniform_int_distribution<uint64_t> rng;
		UUID uuid;
		uuid.u128.low = rng(src);
		uuid.u128.high = rng(src);
		uuid.v4.version = 0b0100;
		uuid.v4.variant = 0b10;
		return uuid;
	}
	UUID UUID::version7() {
		std::random_device engine;
		std::uniform_int_distribution<uint64_t> rng;
		auto const ts_ms = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::utc_clock::now().time_since_epoch()).count());
		auto const src = reinterpret_cast<uint8_t const*>(&ts_ms);
		UUID uuid;
		uuid.u128.low = rng(engine);
		uuid.u128.high = rng(engine);
		auto const dst = reinterpret_cast<uint8_t*>(&uuid.u128.low);
		dst[0] = src[5];
		dst[1] = src[4];
		dst[2] = src[3];
		dst[3] = src[2];
		dst[4] = src[1];
		dst[5] = src[0];
		uuid.v7.version = 0b0111;
		uuid.v7.variant = 0b10;
		return uuid;
	}
	UUID UUID::nil() {
		return {};
	}
	UUID UUID::max() {
		UUID uuid;
		uuid.u128.low = std::numeric_limits<uint64_t>::max();
		uuid.u128.high = std::numeric_limits<uint64_t>::max();
		return uuid;
	}
	UUID UUID::parse(std::string_view const& s) {
		if (s.length() != 32 && s.length() != 36) {
			return nil();
		}
		UUID uuid{};
		for (size_t i = 0, j = 0; i < 16; i += 1) {
			uuid.bytes[i] = fromHex(s[j], s[j + 1]);
			auto const has_dash = s.length() == 36 && (i == 3 || i == 5 || i == 7 || i == 9);
			j += has_dash ? 3 : 2;
		}
		return uuid;
	}
}
