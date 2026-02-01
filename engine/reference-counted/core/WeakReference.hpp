#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
	CORE_INTERFACE IWeakReference : IReferenceCounted {
		virtual bool resolve(InterfaceId const& uuid, void** output) = 0;

		template<typename Interface>
		bool resolve(Interface** const output) {
			static_assert(std::is_base_of_v<IReferenceCounted, Interface>);
			return resolve(getInterfaceId<Interface>(), reinterpret_cast<void**>(output));
		}
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IWeakReference
	CORE_INTERFACE_ID(IWeakReference, "cef127a6-3ae3-541c-a8e2-42257d983eaa")
}
