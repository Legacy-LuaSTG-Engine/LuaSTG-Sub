#pragma once

//DLL base macro
#ifdef EYES2D_EXPORTS
#define EYESDLLAPI _declspec(dllexport) //导出，开发时使用
#else
#define EYESDLLAPI _declspec(dllimport) //导入，调用时使用
#endif // E2D_PROJECT

//Windows base header
#include <Windows.h>
#include <wrl.h>

//usefull tool
#include "E2DException.hpp"
