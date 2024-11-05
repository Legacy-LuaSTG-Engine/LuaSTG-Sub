#include "Main.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

_Use_decl_annotations_ int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
	return luastg::sub::main();
}
