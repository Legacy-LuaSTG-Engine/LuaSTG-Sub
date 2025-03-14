#pragma once
#include <cstdint>
#include <string>
#include <string_view>

// BEGIN: FUCK Microsoft
#ifdef max
#undef max
#endif
// END:   FUCK Microsoft

#define CORE_UUID(a, b, c, d, e0, e1, e2, e3, e4, e5) { 0x##a##u, 0x##b##u, 0x##c##u, 0x##d##u, 0x##e0##u, 0x##e1##u, 0x##e2##u, 0x##e3##u, 0x##e4##u, 0x##e5##u }

namespace core {
	// ReSharper disable once CppInconsistentNaming
	union UUID {
		struct Unsigned128 {
			uint64_t low;
			uint64_t high;
		};
		struct Parts {
			uint32_t a;
			uint16_t b;
			uint16_t c;
			uint16_t d;
			uint8_t e[6];
		};
		struct VersionX {
			uint64_t a : 48;
			uint64_t b1 : 4;
			uint64_t version : 4;
			uint64_t b2 : 8;
			uint64_t c1 : 6;
			uint64_t variant : 2;
			uint64_t c2 : 56;
		};
		struct Version4 {
			uint64_t random_a : 48;
			uint64_t random_b1 : 4;
			uint64_t version : 4;
			uint64_t random_b2 : 8;
			uint64_t random_c1 : 6;
			uint64_t variant : 2;
			uint64_t random_c2 : 56;
		};
		struct Version7 {
			uint64_t unix_ts_ms : 48;
			uint64_t random_a1 : 4;
			uint64_t version : 4;
			uint64_t random_a2 : 8;
			uint64_t random_b1 : 6;
			uint64_t variant : 2;
			uint64_t random_b2 : 56;
		};

		uint8_t bytes[16];
		Unsigned128 u128;
		Parts parts;
		VersionX vx;
		Version4 v4;
		Version7 v7;

		constexpr UUID() noexcept {
			u128.low = {};
			u128.high = {};
		}
		constexpr UUID(uint32_t const a, uint16_t const b, uint16_t const c, uint16_t const d, uint8_t const e0, uint8_t const e1, uint8_t const e2, uint8_t const e3, uint8_t const e4, uint8_t const e5) noexcept {
			parts.a = ((a & 0xff000000u) >> 24) | ((a & 0x00ff0000u) >> 8) | ((a & 0x0000ff00u) << 8) | ((a & 0x000000ffu) << 24);
			parts.b = static_cast<uint16_t>(((b & 0xff00u) >> 8) | ((b & 0x00ffu) << 8));
			parts.c = static_cast<uint16_t>(((c & 0xff00u) >> 8) | ((c & 0x00ffu) << 8));
			parts.d = static_cast<uint16_t>(((d & 0xff00u) >> 8) | ((d & 0x00ffu) << 8));
			parts.e[0] = e0;
			parts.e[1] = e1;
			parts.e[2] = e2;
			parts.e[3] = e3;
			parts.e[4] = e4;
			parts.e[5] = e5;
		}

		[[nodiscard]] bool operator==(UUID const& right) const noexcept { return u128.low == right.u128.low && u128.high == right.u128.high; }
		[[nodiscard]] bool operator!=(UUID const& right) const noexcept { return u128.low != right.u128.low || u128.high != right.u128.high; }

		template<typename Integer = uint8_t>
		[[nodiscard]] Integer version() const noexcept { return static_cast<Integer>(vx.version); }

		template<typename Integer = uint8_t>
		[[nodiscard]] Integer variant() const noexcept { return static_cast<Integer>(vx.variant); }

		[[nodiscard]] std::string toString(bool simple = false) const;

		static UUID version4();
		static UUID version7();
		static UUID nil();
		static UUID max();
		static UUID parse(std::string_view const& s);
	};

	static_assert(static_cast<uint8_t>(-1) == 0xffu);
	static_assert(sizeof(UUID::Unsigned128) == sizeof(uint8_t) * 16);
	static_assert(sizeof(UUID::VersionX) == sizeof(UUID::Unsigned128));
	static_assert(sizeof(UUID::Version4) == sizeof(UUID::Unsigned128));
	static_assert(sizeof(UUID::Version7) == sizeof(UUID::Unsigned128));
	static_assert(sizeof(UUID) == sizeof(UUID::Unsigned128));
}
