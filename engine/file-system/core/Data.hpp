#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
	CORE_INTERFACE IData : IReferenceCounted {
		virtual void* data() = 0;
		virtual size_t size() = 0;

		static bool create(size_t size, IData** pp_data);
		static bool create(size_t size, size_t align, IData** pp_data);
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IData
	template<> constexpr InterfaceId getInterfaceId<IData>() { return UUID::parse("acc69b53-02e3-5d58-a6c6-6c30c936ac98"); }
}
