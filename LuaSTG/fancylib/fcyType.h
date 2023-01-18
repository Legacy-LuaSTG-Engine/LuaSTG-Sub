#pragma once
#ifndef __FTYPE__H__
#define __FTYPE__H__

#include <cstdint>

#define FCYSAFEKILL(x)   { if (x) { (x)->Release(); (x) = NULL; } }

#endif //__FTYPE__H__
