#pragma once
#include "core/ReferenceCounted.hpp"

namespace core {
	// 字符串视图

	using StringView = std::string_view; // pointer | size

	// 不可变的空终止字符串

	struct CORE_NO_VIRTUAL_TABLE IImmutableString : IReferenceCounted {
		[[nodiscard]] virtual bool empty() const noexcept = 0;
		[[nodiscard]] virtual char const* data() const noexcept = 0;
		[[nodiscard]] virtual size_t size() const noexcept = 0;
		[[nodiscard]] virtual char const* c_str() const noexcept = 0;
		[[nodiscard]] virtual size_t length() const noexcept = 0;
		[[nodiscard]] virtual StringView view() const noexcept = 0;

		static void create(StringView const& view, IImmutableString** output);
		static void create(char const* data, size_t size, IImmutableString** output);
	};

	// UUID v5
	// ns:URL
	// https://www.luastg-sub.com/core.IImmutableString
	template<> constexpr InterfaceId getInterfaceId<IImmutableString>() { return UUID::parse("1ef36173-0c4d-5bd8-af47-fa362a5e4805"); }

}
