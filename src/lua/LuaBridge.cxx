#include "LuaBridge.hxx"

#include <string.h>
#include <stdlib.h>
#include <string>
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"

#define PRINT_L_ERROR(L, prefix)                                                                     \
    do                                                                                               \
    {                                                                                                \
        const char *luaErrMsg = lua_tostring(L, -1);                                                 \
        if (luaErrMsg != NULL)                                                                       \
        {                                                                                            \
            Log::print(NGenXXLogLevelError, (std::string(prefix) + std::string(luaErrMsg)).c_str()); \
        }                                                                                            \
    } while (0);

lua_State *NGenXX::LuaBridge::create(void)
{
    lua_State *lstate = luaL_newstate();
    luaL_openlibs(lstate);
    return lstate;
}

void NGenXX::LuaBridge::destroy(lua_State *lstate)
{
    if (lstate == NULL)
    {
        Log::print(NGenXXLogLevelError, "LuaBridge.destroy: lstate is NULL");
        return;
    }
    lua_close(lstate);
}

void NGenXX::LuaBridge::bindFunc(lua_State *lstate, const char *funcName, int (*funcPointer)(lua_State *))
{
    if (lstate == NULL)
    {
        Log::print(NGenXXLogLevelError, "LuaBridge.bindFunc: lstate is NULL");
        return;
    }
    lua_pushcfunction(lstate, funcPointer);
    lua_setglobal(lstate, funcName);
}

#ifndef __EMSCRIPTEN__
int NGenXX::LuaBridge::loadFile(lua_State *lstate, const char *file)
{
    if (lstate == NULL)
    {
        Log::print(NGenXXLogLevelError, "LuaBridge.loadFile: lstate is NULL");
        return LUA_ERRERR;
    }
    if (file == NULL)
    {
        Log::print(NGenXXLogLevelError, "LuaBridge.loadFile: file is NULL");
        return LUA_ERRERR;
    }
    int ret = luaL_dofile(lstate, file);
    if (ret != LUA_OK)
    {
        PRINT_L_ERROR(lstate, "`luaL_dofile` error:");
    }
    return ret;
}
#endif

int NGenXX::LuaBridge::loadScript(lua_State *lstate, const char *script)
{
    if (lstate == NULL)
    {
        Log::print(NGenXXLogLevelError, "LuaBridge.loadScript: lstate is NULL");
        return LUA_ERRERR;
    }
    if (script == NULL)
    {
        Log::print(NGenXXLogLevelError, "LuaBridge.loadScript: script is NULL");
        return LUA_ERRERR;
    }
    int ret = luaL_dostring(lstate, script);
    if (ret != LUA_OK)
    {
        PRINT_L_ERROR(lstate, "`luaL_dostring` error:");
    }
    return ret;
}

const char *NGenXX::LuaBridge::callFunc(lua_State *lstate, const char *func, const char *params)
{
    if (lstate == NULL)
    {
        Log::print(NGenXXLogLevelError, "LuaBridge.callFunc: lstate is NULL");
        return NULL;
    }
    if (func == NULL)
    {
        Log::print(NGenXXLogLevelError, "LuaBridge.callFunc: func is NULL");
        return NULL;
    }
    lua_getglobal(lstate, func);
    lua_pushstring(lstate, params);
    int ret = lua_pcall(lstate, 1, 1, 0);
    if (ret != LUA_OK)
    {
        PRINT_L_ERROR(lstate, "`lua_pcall` error:");
        return NULL;
    }
    const char *res = lua_tostring(lstate, -1);

    // Or memory issues will occur while Lua VM freed but the pointer is referenced outside.
    char *cRes;
    if (res != NULL)
    {
        cRes = (char *)malloc(strlen(res) + 1);
        strcpy(cRes, res);
        // free((void *)res);
    }

    lua_pop(lstate, 1);
    return cRes;
}