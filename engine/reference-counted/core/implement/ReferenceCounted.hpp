#pragma once
#include "core/ReferenceCounted.hpp"
#include "core/implement/ReferenceCountedDebugger.hpp"
#include <cassert>
#include <atomic>

namespace core::implement {
	template<typename Type, typename... Types>
	struct FirstTypeHelper {
		using type = Type;
	};

	template<typename... Types>
	using FirstType = typename FirstTypeHelper<Types...>::type;

	struct ReferenceCounter {
		std::atomic_int32_t strong{ 1 };
		std::atomic_int32_t weak{ 1 };
	};

	template<typename... Interfaces>
	class NoOperationReferenceCounted : public Interfaces... {
	public:
		int32_t retain() override {
			return 1;
		}
		int32_t release() override {
			return 1;
		}
		bool queryInterface(const InterfaceId& uuid, void** const output) override {
			if (output == nullptr) {
				assert(false); return false;
			}
			if (uuid == getInterfaceId<IReferenceCounted>()) {
				*output = static_cast<FirstType<Interfaces...>*>(this);
				return true;
			}
			return query<Interfaces...>(uuid, output);
		}

		NoOperationReferenceCounted() = default;
		NoOperationReferenceCounted(NoOperationReferenceCounted const&) = delete;
		NoOperationReferenceCounted(NoOperationReferenceCounted&&) = delete;
		virtual ~NoOperationReferenceCounted() = default;

		NoOperationReferenceCounted& operator=(NoOperationReferenceCounted const&) = delete;
		NoOperationReferenceCounted& operator=(NoOperationReferenceCounted&&) = delete;

	private:
		template<typename Interface, typename... Rest>
		bool query(const InterfaceId& uuid, void** const output) {
			if (uuid == getInterfaceId<Interface>()) {
				*output = static_cast<Interface*>(this);
				return true;
			}
			if constexpr (sizeof...(Rest) > 0) {
				return query<Rest...>(uuid, output);
			}
			else {
				return false;
			}
		}
	};

	template<typename... Interfaces>
	class ReferenceCounted : public Interfaces... {
	public:
		int32_t retain() override {
			return m_counter.fetch_add(1) + 1;
		}
		int32_t release() override {
			auto const last_counter = m_counter.fetch_sub(1);
			if (last_counter == 1) {
				delete this;
			}
			return last_counter - 1;
		}
		bool queryInterface(const InterfaceId& uuid, void** const output) override {
			if (output == nullptr) {
				assert(false); return false;
			}
			if (uuid == getInterfaceId<IReferenceCounted>()) {
				retain();
				*output = static_cast<FirstType<Interfaces...>*>(this);
				return true;
			}
			return query<Interfaces...>(uuid, output);
		}

		ReferenceCounted() : m_counter{ 1 } {
		#ifndef NDEBUG
			ReferenceCountedDebugger::startTracking(static_cast<FirstType<Interfaces...>*>(this));
		#endif
		}
		ReferenceCounted(ReferenceCounted const&) = delete;
		ReferenceCounted(ReferenceCounted&&) = delete;
		virtual ~ReferenceCounted() {
		#ifndef NDEBUG
			ReferenceCountedDebugger::stopTracking(static_cast<FirstType<Interfaces...>*>(this));
		#endif
		}

		ReferenceCounted& operator=(ReferenceCounted const&) = delete;
		ReferenceCounted& operator=(ReferenceCounted&&) = delete;

	private:
		template<typename Interface, typename... Rest>
		bool query(const InterfaceId& uuid, void** const output) {
			if (uuid == getInterfaceId<Interface>()) {
				retain();
				*output = static_cast<Interface*>(this);
				return true;
			}
			if constexpr (sizeof...(Rest) > 0) {
				return query<Rest...>(uuid, output);
			}
			else {
				return false;
			}
		}

		std::atomic_int32_t m_counter;
	};
}
