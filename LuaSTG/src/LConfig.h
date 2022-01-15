#pragma once
#include "f2dConfig.h"

#define LUASTG_NAME          "LuaSTG"
#define LUASTG_BRANCH        "Sub"
#define LUASTG_VERSION_NAME  "0.9.0"
#define LUASTG_VERSION_MAJOR 0
#define LUASTG_VERSION_MINOR 9
#define LUASTG_VERSION_PATCH 0

#define LUASTG_INFO LUASTG_NAME " " LUASTG_BRANCH " " LUASTG_VERSION_NAME "-beta"

//#define LDEVVERSION 1

#ifdef F2D_GRAPHIC_API_D3D11
#define LUASTG_GRAPHIC_API_D3D11 1
#endif
