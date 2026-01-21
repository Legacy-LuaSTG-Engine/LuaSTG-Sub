#pragma once
#include <cstdint>
#include "core/UUID.hpp"

#define CORE_INTERFACE struct __declspec(novtable)
#define CORE_INTERFACE_ID(NAME, ID) template<> constexpr InterfaceId getInterfaceId<NAME>() { return UUID::parse(ID); }

namespace core {
	using InterfaceId = UUID;

	template<typename Interface>
	InterfaceId getInterfaceId() { return Interface::interface_uuid; }

	CORE_INTERFACE IReferenceCounted {
		virtual bool queryInterface(InterfaceId const& uuid, void** output) = 0;
		virtual int32_t retain() = 0;
		virtual int32_t release() = 0;

		template<typename Interface> bool queryInterface(Interface** const output) {
			static_assert(std::is_base_of_v<IReferenceCounted, Interface>);
			return queryInterface(getInterfaceId<Interface>(), reinterpret_cast<void**>(output));
		}
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IReferenceCounted
	template<> constexpr InterfaceId getInterfaceId<IReferenceCounted>() { return UUID::parse("b6a42c9f-376b-57e7-95a0-68b74556d1e4"); }
}
