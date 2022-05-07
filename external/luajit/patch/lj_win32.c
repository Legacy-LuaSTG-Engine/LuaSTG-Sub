#define lj_win32_c
#define LUA_CORE

#include "lj_win32.h"

#include <assert.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static wchar_t* win32_to_wide(char const* str)
{
    if (str == NULL) return NULL;
    size_t const len = strlen(str);
    int const required = MultiByteToWideChar(CP_UTF8, 0, str, (int)len, NULL, 0);
    if (required <= 0) return NULL;
    size_t const sz = sizeof(wchar_t) * (required + 1);
    wchar_t* buf = (wchar_t*)malloc(sz);
    if (buf == NULL) return NULL;
    memset(buf, 0, sz);
    int const result = MultiByteToWideChar(CP_UTF8, 0, str, (int)len, buf, required);
    if (result <= 0 || result != required) return NULL;
    return buf;
}
static void win32_to_wide_free(wchar_t** p)
{
    if (*p) free(*p);
    *p = NULL;
}
static char* win32_to_utf8(wchar_t const* str)
{
    if (str == NULL) return NULL;
    size_t const len = wcslen(str);
    int const required = WideCharToMultiByte(CP_UTF8, 0, str, (int)len, NULL, 0, NULL, NULL);
    if (required <= 0) return NULL;
    size_t const sz = sizeof(char) * (required + 1);
    char* buf = (char*)malloc(sz);
    if (buf == NULL) return NULL;
    memset(buf, 0, sz);
    int const result = WideCharToMultiByte(CP_UTF8, 0, str, (int)len, buf, required, NULL, NULL);
    if (result <= 0 || result != required) return NULL;
    return buf;
}
static char* win32_to_utf8_buf(wchar_t const* str, char* buf, size_t bufsz)
{
    assert(buf != NULL && bufsz > 0);
    if (str == NULL) return NULL;
    size_t const len = wcslen(str);
    int const required = WideCharToMultiByte(CP_UTF8, 0, str, (int)len, NULL, 0, NULL, NULL);
    if (required <= 0) return NULL;
    if ((required + 1) > (int)bufsz) return NULL; // too large
    int const result = WideCharToMultiByte(CP_UTF8, 0, str, (int)len, buf, required, NULL, NULL);
    if (result <= 0 || result != required) return NULL;
    return buf;
}
static void win32_to_utf8_free(char** p)
{
    if (*p) free(*p);
    *p = NULL;
}

static char* _u8buf = NULL;
void _u8gclear()
{
    win32_to_utf8_free(&_u8buf);
}

FILE* _u8fopen(char const* fname, char const* mode)
{
    FILE* fp = NULL;
    wchar_t* w_fname = win32_to_wide(fname);
    wchar_t* w_mode = win32_to_wide(mode);
    errno_t const result = _wfopen_s(&fp, w_fname, w_mode);
    win32_to_wide_free(&w_fname);
    win32_to_wide_free(&w_mode);
    return (result == 0) ? fp : NULL;
}
FILE* _u8popen(char const* command, char const* mode)
{
    wchar_t* w_command = win32_to_wide(command);
    wchar_t* w_mode = win32_to_wide(mode);
    FILE* const result = _wpopen(w_command, w_mode);
    win32_to_wide_free(&w_command);
    win32_to_wide_free(&w_mode);
    return result;
}
int _u8system(char const* command)
{
    wchar_t* w_command = win32_to_wide(command);
    int const result = _wsystem(w_command);
    win32_to_wide_free(&w_command);
    return result;
}
int _u8remove(char const* path)
{
    wchar_t* w_path = win32_to_wide(path);
    int const result = _wremove(w_path);
    win32_to_wide_free(&w_path);
    return result;
}
int _u8rename(char const* oldname, char const* newname)
{
    wchar_t* w_oldname = win32_to_wide(oldname);
    wchar_t* w_newname = win32_to_wide(newname);
    int const result = _wrename(w_oldname, w_newname);
    win32_to_wide_free(&w_oldname);
    win32_to_wide_free(&w_newname);
    return result;
}
char* _u8tmpnam(char* str)
{
    static wchar_t wbuf[MAX_PATH + 1];
    memset(wbuf, 0, sizeof(wbuf));
    if (str)
    {
        wchar_t* result = _wtmpnam(wbuf);
        if (result == NULL) return NULL;
        char* u8_buf = win32_to_utf8(wbuf);
        size_t const len = strlen(u8_buf);
        memcpy(str, u8_buf, sizeof(char) * len);
        str[len] = '\0';
        win32_to_utf8_free(&u8_buf);
        return (result != NULL) ? str : NULL;
    }
    else
    {
        _u8gclear();
        wchar_t* result = _wtmpnam(NULL);
        if (result == NULL) return NULL;
        _u8buf = win32_to_utf8(result);
        return _u8buf;
    }
}
char* _u8getenv(char const* envname)
{
    _u8gclear();
    wchar_t* w_envname = win32_to_wide(envname);
    wchar_t* result = _wgetenv(w_envname);
    win32_to_wide_free(&w_envname);
    if (result == NULL) return NULL;
    _u8buf = win32_to_utf8(result);
    return _u8buf;
}
