#pragma once
#include <cstdint>
#include "core/UUID.hpp"

#define CORE_NO_VIRTUAL_TABLE __declspec(novtable)

namespace core {
	using InterfaceId = UUID;

	template<typename Interface>
	InterfaceId getInterfaceId() { return Interface::interface_uuid; }

	struct CORE_NO_VIRTUAL_TABLE IReferenceCounted {
		virtual bool queryInterface(InterfaceId const& uuid, void** output) = 0;
		virtual int32_t retain() = 0;
		virtual int32_t release() = 0;

		template<typename Interface> bool queryInterface(Interface** const output) {
			return queryInterface(getInterfaceId<Interface>(), reinterpret_cast<void**>(output));
		}
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IReferenceCounted
	template<> constexpr InterfaceId getInterfaceId<IReferenceCounted>() { return UUID::parse("b6a42c9f-376b-57e7-95a0-68b74556d1e4"); }
}
