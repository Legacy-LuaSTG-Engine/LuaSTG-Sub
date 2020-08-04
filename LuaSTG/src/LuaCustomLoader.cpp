#include <string>
#include "LuaCustomLoader.hpp"
#include "Global.h"
#include "AppFrame.h"

static int readable(const char* filename) {
    try {
        return LFMGR.FileExistEx(filename) ? 1 : 0;
    }
    catch(...) {}
    return 0;
}

static const char* pushnexttemplate(lua_State* L, const char* path) {
    const char* l;
    while (*path == *LUA_PATHSEP) path++;  /* skip separators */
    if (*path == '\0') return NULL;  /* no more templates */
    l = strchr(path, *LUA_PATHSEP);  /* find next separator */
    if (l == NULL) l = path + strlen(path);
    lua_pushlstring(L, path, (size_t)(l - path));  /* template */
    return l;
}

static const char* searchpath(lua_State* L, const char* name, const char* path, const char* sep, const char* dirsep) {
    luaL_Buffer msg;  /* to build error message */
    luaL_buffinit(L, &msg);
    if (*sep != '\0')  /* non-empty separator? */
        name = luaL_gsub(L, name, sep, dirsep);  /* replace it by 'dirsep' */
    while ((path = pushnexttemplate(L, path)) != NULL) {
        const char* filename = luaL_gsub(L, lua_tostring(L, -1),
            LUA_PATH_MARK, name);
        lua_remove(L, -2);  /* remove path template */
        if (readable(filename))  /* does file exist and is readable? */
            return filename;  /* return that file name */
        lua_pushfstring(L, "\n\tno file " LUA_QS, filename);
        lua_remove(L, -2);  /* remove file name */
        luaL_addvalue(&msg);  /* concatenate error msg. entry */
    }
    luaL_pushresult(&msg);  /* create error message */
    return NULL;  /* not found */
}

static const char* findfile(lua_State* L, const char* name, const char* pname) {
    std::string path;
    lua_getglobal(L, "package");                                               // ??? t
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "path");                                           // ??? t s
        if (lua_isstring(L, -1)) {
            path = lua_tostring(L, -1);
        }
        else {
            luaL_error(L, LUA_QL("package.%s") " must be a string", pname);
        }
        lua_pop(L, 1);                                                         // ??? t
    }
    else {
        luaL_error(L, LUA_QL("package") " must be a table");
    }
    lua_pop(L, 1);                                                             // ???
    return searchpath(L, name, path.c_str(), ".", "/");
}

static void loaderror(lua_State* L, const char* filename) {
    luaL_error(L, "error loading module " LUA_QS " from file " LUA_QS ":\n\t%s",
        lua_tostring(L, 1), filename, lua_tostring(L, -1));
}

static int package_loader_luastg(lua_State* L) {
    const char* filename;
    const char* name = luaL_checkstring(L, 1);
    filename = findfile(L, name, "path");
    if (filename == NULL) return 1;  /* library not found in this path */
    //if (luaL_loadfile(L, filename) != 0)
        //loaderror(L, filename);
    fcyRefPointer<fcyStream> stream; {
        fcyStream* p = LAPP.GetFileManager().LoadFile(filename);
        stream.DirectSet(p);
    }
    if (*stream == nullptr)
        loaderror(L, filename);
    else {
        fLen length = stream->GetLength();
        std::string str;
        str.resize((size_t)length);
        stream->SetPosition(FCYSEEKORIGIN_BEG, 0);
        fLen rd = 0;
        stream->ReadBytes((fData)str.data(), length, &rd);
        if (rd != stream->GetLength())
            loaderror(L, filename);
        if (luaL_loadstring(L, str.c_str()) != 0)
            loaderror(L, filename);
    }
    return 1;  /* library loaded successfully */
}

namespace LuaSTGPlus {
	void lua_register_custom_loader(lua_State* L) {
        lua_getglobal(L, "package");                         // ??? t
        if (lua_istable(L, -1)) {
            lua_getfield(L, -1, "loaders");                  // ??? t t
            if (lua_istable(L, -1)) {
                lua_pushinteger(L, lua_objlen(L, -1) + 1);   // ??? t t i
                lua_pushcfunction(L, package_loader_luastg); // ??? t t i f
                lua_settable(L, -3);                         // ??? t t
            }
            lua_pop(L, 1);                                   // ??? t
        }
        lua_pop(L, 1);                                       // ???
	}
};
