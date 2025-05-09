#include "Data.hpp"
#include "core/SmartReference.hpp"
#include "core/implement/ReferenceCounted.hpp"
#include <cstdint>

namespace core {
	class Data final : public implement::ReferenceCounted<IData> {
	public:
		explicit Data(size_t const size) {
			m_size = size;
			m_data = std::malloc(size);
		}
		Data(size_t const size, size_t const align) {
			if constexpr (sizeof(size_t) == sizeof(uint64_t)) {
				m_size = size | UINT64_C(0x8000'0000'0000'0000);
			}
			else if constexpr (sizeof(size_t) == sizeof(uint32_t)) {
				m_size = size | UINT32_C(0x8000'0000);
			}
			else {
				static_assert(sizeof(size_t) == sizeof(uint64_t) || sizeof(size_t) == sizeof(uint32_t), "unsupported architecture");
			}
			m_data = _aligned_malloc(size, align);
		}
		Data(Data const&) = delete;
		Data(Data&&) = delete;
		~Data() override {
			m_size = 0;
			if (m_data != nullptr) {
				if (isAligned()) {
					_aligned_free(m_data);
				}
				else {
					std::free(m_data);
				}
				m_data = nullptr;
			}
		}

		Data& operator=(Data const&) = delete;
		Data& operator=(Data&&) = delete;

		void* data() override { return m_data; }
		size_t size() override {
			if constexpr (sizeof(size_t) == sizeof(uint64_t)) {
				return m_size & UINT64_C(0x7fff'ffff'ffff'ffff);
			}
			else if constexpr (sizeof(size_t) == sizeof(uint32_t)) {
				return m_size & UINT32_C(0x7fff'ffff);
			}
			else {
				static_assert(sizeof(size_t) == sizeof(uint64_t) || sizeof(size_t) == sizeof(uint32_t), "unsupported architecture");
			}
		}

	private:
		size_t m_size{};
		void* m_data{};

		bool isAligned() const noexcept {
			if constexpr (sizeof(size_t) == sizeof(uint64_t)) {
				return (m_size & UINT64_C(0x8000'0000'0000'0000)) == UINT64_C(0x8000'0000'0000'0000);
			}
			else if constexpr (sizeof(size_t) == sizeof(uint32_t)) {
				return (m_size & UINT32_C(0x8000'0000)) == UINT32_C(0x8000'0000);
			}
			else {
				static_assert(sizeof(size_t) == sizeof(uint64_t) || sizeof(size_t) == sizeof(uint32_t), "unsupported architecture");
			}
		}
	};

	bool IData::create(size_t const size, IData** const pp_data) {
		*pp_data = nullptr;
		SmartReference<IData> data;
		data.attach(new Data(size));
		if (data->data() == nullptr) {
			return false;
		}
		*pp_data = data.detach();
		return true;
	}
	bool IData::create(size_t const size, size_t const align, IData** const pp_data) {
		*pp_data = nullptr;
		SmartReference<IData> data;
		data.attach(new Data(size, align));
		if (data->data() == nullptr) {
			return false;
		}
		*pp_data = data.detach();
		return true;
	}
}
