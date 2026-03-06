#include "Main.h"
#include "core/CommandLineArguments.hpp"
#include "ApplicationRestart.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

_Use_decl_annotations_ int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
	core::CommandLineArguments::initialize();
	auto const code = luastg::main();
	if (luastg::ApplicationRestart::hasRestart()) {
		luastg::ApplicationRestart::start();
		return EXIT_SUCCESS;
	}
	return code;
}
