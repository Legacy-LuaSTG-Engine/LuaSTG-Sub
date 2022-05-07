#ifndef _LJ_WIN32_H
#define _LJ_WIN32_H

#include <stdio.h>

#include "lj_def.h"

LJ_FUNC void _u8gclear();

LJ_FUNC FILE* _u8fopen(char const* fname, char const* mode);
LJ_FUNC int _u8system(char const* command);
LJ_FUNC int _u8remove(char const* path);
LJ_FUNC int _u8rename(char const* oldname, char const* newname);
LJ_FUNC char* _u8tmpnam(char* str);
LJ_FUNC char* _u8getenv(char const* envname);

#endif
