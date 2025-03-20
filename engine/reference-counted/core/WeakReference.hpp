#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
	struct CORE_NO_VIRTUAL_TABLE IWeakReference : IReferenceCounted {
		virtual Boolean32 resolve(UUID const& uuid, void** output) = 0;

		template<typename Interface> Boolean32 resolve(Interface** const output) {
			return resolve(uuid_of<Interface>(), reinterpret_cast<void**>(output));
		}
	};

	template<> constexpr UUID uuid_of<IWeakReference>() { return CORE_UUID_INITIALIZER(276a742d, ecc9, 5b57, 998c, f2, 27, 16, 00, d4, 4a); }
}
