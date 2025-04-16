#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
	struct CORE_NO_VIRTUAL_TABLE IWeakReference : IReferenceCounted {
		virtual bool resolve(InterfaceId const& uuid, void** output) = 0;

		template<typename Interface> bool resolve(Interface** const output) {
			return resolve(getInterfaceId<Interface>(), reinterpret_cast<void**>(output));
		}
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IWeakReference
	template<> constexpr InterfaceId getInterfaceId<IWeakReference>() { return UUID::parse("cef127a6-3ae3-541c-a8e2-42257d983eaa"); }
}
