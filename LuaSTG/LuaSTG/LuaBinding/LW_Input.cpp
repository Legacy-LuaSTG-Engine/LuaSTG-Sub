#include "LuaBinding/LuaWrapper.hpp"
#include "AppFrame.h"
#include "Keyboard.h"
#include "platform/Keyboard.hpp"
#include "Mouse.h"

static int register_keyboard(lua_State* L)
{
	struct Wrapper
	{
		static int GetKeyState(lua_State* L) noexcept
		{
			lua_pushboolean(L, LAPP.GetKeyState(luaL_checkinteger(L, -1)));
			return 1;
		}
	};

	luaL_Reg lib_empty[] = {
		{NULL, NULL},
	};
	luaL_Reg lib_keyboard[] = {
		{ "GetKeyState", &Wrapper::GetKeyState },
		{NULL, NULL},
	};

	luaL_register(L, LUASTG_LUA_LIBNAME ".Input", lib_empty);             // ??? lstg.Input
	luaL_register(L, LUASTG_LUA_LIBNAME ".Input.Keyboard", lib_keyboard); // ??? lstg.Input lstg.Input.Keyboard

	struct { char const* id; char const* name; lua_Integer value; } kcode[] = {
	#define KV(KEY) { #KEY, #KEY, (lua_Integer)platform::Keyboard::Key::##KEY }
	#define KNV(NAME, KEY) { NAME, NAME, (lua_Integer)platform::Keyboard::Key::##KEY }
		KV(None),

		KV(Back),
		KV(Tab),
		{ "Clear", "Clear", /* VK_CLEAR */ 0x0C },
		KV(Enter),
		{ "Shift", "Shift", /* VK_SHIFT */ 0x10 },
		{ "Control", "Control", /* VK_CONTROL */ 0x11 },
		{ "Alt", "Alt", /* VK_MENU */ 0x12 },
		KV(Pause),
		KV(CapsLock),

		{ "ImeHangul", "ImeHangul", 0x15 },
		{ "ImeKana", "ImeKana", 0x15 },
		KV(ImeOn),
		{ "ImeJunja", "ImeJunja", 0x17 },
		{ "ImeFinal", "ImeFinal", /* VK_FINAL */ 0x18 },
		{ "ImeKanji", "ImeKanji", 0x19 },
		{ "ImeHanja", "ImeHanja", 0x19 },
		KV(ImeOff),

		KV(Escape),

		KV(ImeConvert),
		KV(ImeNoConvert),
		{ "ImeAccept", "ImeAccept", /* VK_ACCEPT */ 0x1E },
		{ "ImeModeChangeRequest", "ImeModeChangeRequest", /* VK_MODECHANGE */ 0x1F },

		KV(Space),
		KV(PageUp),
		KV(PageDown),
		KV(End),
		KV(Home),
		KV(Left),
		KV(Up),
		KV(Right),
		KV(Down),
		KV(Select),
		KV(Print),
		KV(Execute),
		KV(PrintScreen),
		KV(Insert),
		KV(Delete),
		KV(Help),
		KV(D0),
		KV(D1),
		KV(D2),
		KV(D3),
		KV(D4),
		KV(D5),
		KV(D6),
		KV(D7),
		KV(D8),
		KV(D9),

		KV(A),
		KV(B),
		KV(C),
		KV(D),
		KV(E),
		KV(F),
		KV(G),
		KV(H),
		KV(I),
		KV(J),
		KV(K),
		KV(L),
		KV(M),
		KV(N),
		KV(O),
		KV(P),
		KV(Q),
		KV(R),
		KV(S),
		KV(T),
		KV(U),
		KV(V),
		KV(W),
		KV(X),
		KV(Y),
		KV(Z),
		KV(LeftWindows),
		KV(RightWindows),
		KV(Apps),

		KV(Sleep),
		KV(NumPad0),
		KV(NumPad1),
		KV(NumPad2),
		KV(NumPad3),
		KV(NumPad4),
		KV(NumPad5),
		KV(NumPad6),
		KV(NumPad7),
		KV(NumPad8),
		KV(NumPad9),
		KV(Multiply),
		KV(Add),
		KV(Separator),
		KV(Subtract),
		KV(Decimal),
		KV(Divide),

		KV(F1),
		KV(F2),
		KV(F3),
		KV(F4),
		KV(F5),
		KV(F6),
		KV(F7),
		KV(F8),
		KV(F9),
		KV(F10),
		KV(F11),
		KV(F12),
		KV(F13),
		KV(F14),
		KV(F15),
		KV(F16),
		KV(F17),
		KV(F18),
		KV(F19),
		KV(F20),
		KV(F21),
		KV(F22),
		KV(F23),
		KV(F24),

		KV(NumLock),
		KV(Scroll),
		KV(LeftShift),
		KV(RightShift),
		KV(LeftControl),
		KV(RightControl),
		KV(LeftAlt),
		KV(RightAlt),

		KV(BrowserBack),
		KV(BrowserForward),
		KV(BrowserRefresh),
		KV(BrowserStop),
		KV(BrowserSearch),
		KV(BrowserFavorites),
		KV(BrowserHome),
		KV(VolumeMute),
		KV(VolumeDown),
		KV(VolumeUp),
		KV(MediaNextTrack),
		KV(MediaPreviousTrack),
		KV(MediaStop),
		KV(MediaPlayPause),
		KV(LaunchMail),
		KV(SelectMedia),
		KV(LaunchApplication1),
		KV(LaunchApplication2),

		KNV("Semicolon", OemSemicolon),
		KNV("Plus", OemPlus),
		KNV("Comma", OemComma),
		KNV("Minus", OemMinus),
		KNV("Period", OemPeriod),
		KNV("Question", OemQuestion),
		KNV("Tilde", OemTilde),

		KNV("OpenBrackets", OemOpenBrackets),
		KNV("Pipe", OemPipe),
		KNV("CloseBrackets", OemCloseBrackets),
		KNV("Quotes", OemQuotes),
		KV(Oem8),

		KNV("Oem102", OemBackslash),

		KV(ProcessKey),

		KV(NumPadEnter),

		KV(OemCopy),
		KV(OemAuto),
		KV(OemEnlW),

		KV(Attn),
		KV(Crsel),
		KV(Exsel),
		KV(EraseEof),

		KV(Play),
		KV(Zoom),

		KV(Pa1),
		KV(OemClear),
	#undef KNV
	#undef KV
	};
	for (auto const& v : kcode)
	{
		lua_pushstring(L, v.id);     // ??? lstg.Input lstg.Keyboard "name"
		lua_pushinteger(L, v.value); // ??? lstg.Input lstg.Keyboard "name" code
		lua_settable(L, -3);         // ??? lstg.Input lstg.Keyboard
	}

	lua_setfield(L, -1, "Keyboard"); // ??? lstg.Input
	lua_pop(L, 1);                   // ???
	
	return 0;
}
static int register_mouse(lua_State* L)
{
	struct Wrapper
	{
		static int GetKeyState(lua_State* L) noexcept
		{
			lua_pushboolean(L, LAPP.GetMouseState(luaL_checkinteger(L, 1)));
			return 1;
		}
		static int GetPosition(lua_State* L) noexcept
		{
			Core::Vector2F tPos = LAPP.GetMousePosition(lua_toboolean(L, 1));
			lua_pushnumber(L, tPos.x);
			lua_pushnumber(L, tPos.y);
			return 2;
		}
		static int GetWheelDelta(lua_State* L) noexcept
		{
			lua_pushnumber(L, (lua_Number)LAPP.GetMouseWheelDelta() / /* WHEEL_DELTA */ 120.0);
			return 1;
		}
	};

	luaL_Reg lib_empty[] = {
		{NULL, NULL},
	};
	luaL_Reg lib_mouse[] = {
		{ "GetKeyState", &Wrapper::GetKeyState },
		{ "GetPosition", &Wrapper::GetPosition },
		{ "GetWheelDelta", &Wrapper::GetWheelDelta },
		{NULL, NULL},
	};

	luaL_register(L, LUASTG_LUA_LIBNAME ".Input", lib_empty);       // ??? lstg.Input
	luaL_register(L, LUASTG_LUA_LIBNAME ".Input.Mouse", lib_mouse); // ??? lstg.Input lstg.Input.Mouse

	struct { char const* id; lua_Integer value; } mcode[] = {
		{ "None", 0 },

		{ "Primary", /* VK_LBUTTON */ 0x01 },
		{ "Left", /* VK_LBUTTON */ 0x01 },
		{ "Middle", /* VK_MBUTTON */ 0x04 },
		{ "Secondary", /* VK_RBUTTON */ 0x02 },
		{ "Right", /* VK_RBUTTON */ 0x02 },

		{ "X1", /* VK_XBUTTON1 */ 0x05 },
		{ "XButton1", /* VK_XBUTTON1 */ 0x05 },
		{ "X2", /* VK_XBUTTON2 */ 0x06 },
		{ "XButton2", /* VK_XBUTTON2 */ 0x06 },
	};
	for (auto const& v : mcode)
	{
		lua_pushstring(L, v.id);     // ??? lstg.Input lstg.Input.Mouse "name"
		lua_pushinteger(L, v.value); // ??? lstg.Input lstg.Input.Mouse "name" code
		lua_settable(L, -3);         // ??? lstg.Input lstg.Input.Mouse
	}

	lua_setfield(L, -1, "Mouse"); // ??? lstg.Input
	lua_pop(L, 1);                // ???

	return 0;
}

void LuaSTGPlus::LuaWrapper::InputWrapper::Register(lua_State* L) noexcept
{
	struct Wrapper
	{
		static int GetKeyState(lua_State* L) noexcept
		{
			lua_pushboolean(L, LAPP.GetKeyState(luaL_checkinteger(L, -1)));
			return 1;
		}
		static int GetMouseState(lua_State* L) noexcept
		{
			lua_pushboolean(L, LAPP.GetMouseState_legacy(luaL_checkinteger(L, 1)));
			return 1;
		}
		static int GetMousePosition(lua_State* L) noexcept
		{
			Core::Vector2F tPos = LAPP.GetMousePosition(lua_toboolean(L, 1));
			lua_pushnumber(L, tPos.x);
			lua_pushnumber(L, tPos.y);
			return 2;
		}
		static int GetMouseWheelDelta(lua_State* L) noexcept
		{
			lua_pushinteger(L, (lua_Integer)LAPP.GetMouseWheelDelta());
			return 1;
		}
		// 应该废弃的方法
		static int GetLastKey(lua_State* L) noexcept
		{
			lua_pushinteger(L, LAPP.GetLastKey());
			return 1;
		}
	};

	luaL_Reg lib_empty[] = {
		{NULL, NULL},
	};
	luaL_Reg lib_compat[] = {
		{ "GetKeyState", &Wrapper::GetKeyState },
		{ "GetMouseState", &Wrapper::GetMouseState },
		{ "GetMousePosition", &Wrapper::GetMousePosition },
		{ "GetMouseWheelDelta", &Wrapper::GetMouseWheelDelta },
		// 应该废弃的方法
		{ "GetLastKey", &Wrapper::GetLastKey },
		{NULL, NULL},
	};

	luaL_register(L, LUASTG_LUA_LIBNAME, lib_compat);         // ??? lstg
	luaL_register(L, LUASTG_LUA_LIBNAME ".Input", lib_empty); // ??? lstg lstg.Input
	lua_setfield(L, -1, "Input");                             // ??? lstg
	lua_pop(L, 1);                                            // ???

	register_keyboard(L);
	register_mouse(L);
}
