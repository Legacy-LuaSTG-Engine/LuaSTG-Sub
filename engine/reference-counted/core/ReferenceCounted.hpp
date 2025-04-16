#pragma once
#include <cstdint>
#include "core/UUID.hpp"

#define CORE_NO_VIRTUAL_TABLE __declspec(novtable)

namespace core {
	template<typename Interface>
	UUID uuid_of() { return Interface::interface_uuid; }

	struct CORE_NO_VIRTUAL_TABLE IReferenceCounted {
		virtual bool queryInterface(UUID const& uuid, void** output) = 0;
		virtual int32_t reference() = 0;
		virtual int32_t release() = 0;

		template<typename Interface> bool queryInterface(Interface** const output) {
			return queryInterface(uuid_of<Interface>(), reinterpret_cast<void**>(output));
		}
	};

	template<> constexpr UUID uuid_of<IReferenceCounted>() { return CORE_UUID_INITIALIZER(3b4b05b7, 0986, 5d2f, 8fce, c8, 0c, 01, 03, 95, 28); }
}
