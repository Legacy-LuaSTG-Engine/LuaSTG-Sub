#pragma once
#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <string_view>
#ifndef NDEBUG
#include <string>
#endif

namespace LuaSTGPlus {
	// MPQ哈希表
	class MPQHashCryptTable {
	private:
		uint32_t m_CryptTable[0x500] = {};
	public:
		uint32_t operator[](uint32_t idx) const {
			return m_CryptTable[idx];
		}

		MPQHashCryptTable() {
			uint32_t seed = 0x00100001;
			for (uint32_t index1 = 0; index1 < 0x100; index1++) {
				for (uint32_t index2 = index1, i = 0; i < 5; i++, index2 += 0x100) {
					unsigned long temp1, temp2;
					seed = (seed * 125 + 3) % 0x2AAAAB;
					temp1 = (seed & 0xFFFF) << 0x10;
					seed = (seed * 125 + 3) % 0x2AAAAB;
					temp2 = (seed & 0xFFFF);
					m_CryptTable[index2] = (temp1 | temp2);
				}
			}
		}

		static const MPQHashCryptTable& GetInstance() {
			static MPQHashCryptTable s_Table;
			return s_Table;
		}
	};

	// MPQ哈希算法
	template<uint32_t HashType>
	uint32_t MPQHash(std::string_view key) {
		uint32_t seed1 = 0x7FED7FED;
		uint32_t seed2 = 0xEEEEEEEE;
		uint32_t ch;
		for (auto const& c : key) {
			ch = static_cast<unsigned char>(c);
			seed1 = MPQHashCryptTable::GetInstance()[(HashType << 8) + ch] ^ (seed1 + seed2);
			seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
		}
		return seed1;
	}

	// 字典键
	struct DictionaryKey {
		uint32_t HashKey;
		uint32_t Hash1;
		uint32_t Hash2;
	#ifndef NDEBUG
		std::string Key;
	#endif

		bool operator==(const DictionaryKey& right) const {
		#ifndef NDEBUG
			if (HashKey == right.HashKey && Hash1 == right.Hash1 && Hash2 == right.Hash2) {
				assert(Key == right.Key);
				return true;
			}
			else {
				assert(Key != right.Key);
				return false;
			}
		#else
			return (HashKey == right.HashKey && Hash1 == right.Hash1 && Hash2 == right.Hash2);
		#endif
		}

		DictionaryKey() : HashKey(0), Hash1(0), Hash2(0) {}

		DictionaryKey(std::string_view KeyStr) {
			HashKey = MPQHash<0>(KeyStr);
			Hash1 = MPQHash<1>(KeyStr);
			Hash2 = MPQHash<2>(KeyStr);
		#ifndef NDEBUG
			Key = KeyStr;
		#endif
		}

		DictionaryKey(const DictionaryKey& org) : HashKey(org.HashKey), Hash1(org.Hash1), Hash2(org.Hash2) {
		#ifndef NDEBUG
			Key = org.Key;
		#endif
		}

		DictionaryKey(DictionaryKey&& org) noexcept : HashKey(org.HashKey), Hash1(org.Hash1), Hash2(org.Hash2) {
		#ifndef NDEBUG
			Key = std::move(org.Key);
		#endif
		}
	};

	// 字典键hash函数，用来获取hash值
	struct DictionaryKeyHasher {
		size_t operator()(const DictionaryKey& k) const {
			return k.HashKey;
		}
	};

	// 基于MPQ哈希的字典
	// DictionaryKey为键，DictionaryKeyHasher为键的hash值获取方法
	template<typename T>
	using Dictionary = std::unordered_map<DictionaryKey, T, DictionaryKeyHasher>;
}
