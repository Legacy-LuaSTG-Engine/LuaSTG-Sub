#include "lua/plus.hpp"

#ifndef NDEBUG
namespace {
	using std::string_literals::operator ""s;
	using std::string_view_literals::operator ""sv;
	void main(lua_State* L) {
		lua::stack_t S(L);
		S.push_value(true);
		S.push_value(false);
		S.push_value(1i32);
		S.push_value(1ui32);
		S.push_value(3.14f);
		S.push_value(6.28);
		S.push_value("Hello world!");
		S.push_value("Hello world!"sv);
		S.push_value("Hello world!"s);
	}
}
#endif
