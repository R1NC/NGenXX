#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../external/cjson/cJSON.h"

#define HTTP_HEADERS_MAX_COUNT 100
#define HTTP_HEADER_MAX_LENGTH 8190

extern "C"
{
#include "../../../external/lua/lauxlib.h"

    typedef struct NGenXXHandle
    {
#ifdef USE_LUA
        lua_State *lua_state;
#endif
        void *sqlite;
    } NGenXXHandle;
}

#ifdef __cplusplus

#include <string>
#include <vector>
#include <functional>
#include "log/Log.hxx"
#include "net/HttpClient.hxx"
#include "store/SQLite.hxx"
#include "store/KV.hxx"
#include "util/JsonUtil.hxx"
#ifdef USE_LUA
#include "lua/LuaBridge.hxx"
#endif

// WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT_WASM extern "C" EMSCRIPTEN_KEEPALIVE
#define EXPORT_WASM_LUA extern "C"
#endif

#define VERSION "0.0.1"

static inline const char *str2charp(std::string s)
{
    const char *c = s.c_str();
    char *nc = (char *)malloc(strlen(c) + 1);
    strcpy(nc, c);
    return nc;
}

#ifdef USE_LUA

static inline void parse_lua_func_params(lua_State *L, std::function<void(cJSON *)> callback)
{
    const char *s = luaL_checkstring(L, 1);
    cJSON *json = cJSON_Parse(s);
    if (json)
    {
        if (json->type == cJSON_Object)
        {
            cJSON *oj = json->child;
            while (oj)
            {
                callback(oj);
                oj = oj->next;
            }
        }
        cJSON_free(json);
    }
}

#define BIND_LUA_FUNC(h, f) NGenXX::LuaBridge::bindFunc(((NGenXXHandle *)h)->lua_state, #f, f);

#endif

#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
const char *ngenxx_get_version(void)
{
    auto s = std::string(VERSION);
    return str2charp(s);
}

#ifdef USE_LUA
int ngenxx_get_versionL(lua_State *L)
{
    const char *res = ngenxx_get_version();
    lua_pushstring(L, res);
    return LUA_OK;
}
#endif

#ifdef USE_LUA
void export_funcs_for_lua(void *handle);
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
void *ngenxx_init(void)
{
    NGenXX::Net::HttpClient::create();
    NGenXXHandle *handle = (NGenXXHandle *)malloc(sizeof(NGenXXHandle));
    handle->sqlite = new NGenXX::Store::SQLite();
#ifdef USE_LUA
    handle->lua_state = NGenXX::LuaBridge::create();
    export_funcs_for_lua(handle);
#endif
    return handle;
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
void ngenxx_release(void *sdk)
{
    NGenXX::Net::HttpClient::destroy();
    if (sdk == NULL)
        return;
    NGenXXHandle *h = (NGenXXHandle *)sdk;
    delete ((NGenXX::Store::SQLite *)h->sqlite);
#ifdef USE_LUA
    if (h->lua_state)
    {
        NGenXX::LuaBridge::destroy(h->lua_state);
    }
#endif
    free(sdk);
}

#pragma mark Log

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_log_set_level(int level)
{
    NGenXX::Log::setLevel(level);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_log_set_callback(void (*callback)(int level, const char *log))
{
    NGenXX::Log::setCallback(callback);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_log_print(int level, const char *content)
{
    NGenXX::Log::print(level, content);
}

#ifdef USE_LUA
int ngenxx_log_printL(lua_State *L)
{
    int level;
    char *content = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, level);
        JSON_READ_STR(j, content); 
    });
    if (content == NULL) return LUA_ERRRUN;
    ngenxx_log_print(level, content);
    return LUA_OK;
}
#endif

#pragma mark Net.Http

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
const char *ngenxx_net_http_request(const char *url, const char *params, int method, char **headers_v, int headers_c, long timeout)
{
    if (url == NULL)
        return NULL;
    const std::string sUrl(url);
    const std::string sParams(params);
    std::vector<std::string> vHeaders;
    if (headers_v != NULL && headers_c > 0)
    {
        vHeaders = std::vector<std::string>(headers_v, headers_v + headers_c);
    }

    auto s = NGenXX::Net::HttpClient::request(sUrl, sParams, method, vHeaders, timeout);
    return str2charp(s);
}

#ifdef USE_LUA
int ngenxx_net_http_requestL(lua_State *L)
{
    char *url = NULL, *params = NULL;
    int method;
    char **headers = NULL;
    int headers_c;
    long timeout;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_STR(j, url);
        JSON_READ_STR(j, params);
        JSON_READ_NUM(j, method);
        JSON_READ_NUM(j, timeout);
        JSON_READ_STR_ARRAY(j, headers, headers_c, HTTP_HEADERS_MAX_COUNT, HTTP_HEADER_MAX_LENGTH); 
    });
    if (url == NULL) return LUA_ERRRUN;
    const char *res = ngenxx_net_http_request(url, params, method, headers, headers_c, timeout);
    lua_pushstring(L, res);
    return LUA_OK;
}
#endif

#pragma mark Store.SQLite

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void *ngenxx_store_sqlite_open(void *sdk, const char *file)
{
    if (file == NULL)
        return NULL;
    NGenXX::Store::SQLite *sqlite = (NGenXX::Store::SQLite *)(((NGenXXHandle *)sdk)->sqlite);
    return sqlite->connect(std::string(file));
}

#ifdef USE_LUA
int ngenxx_store_sqlite_openL(lua_State *L)
{
    long sdk;
    char *file = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, sdk);
        JSON_READ_STR(j, file); 
    });
    if (sdk <= 0 || file == NULL) return LUA_ERRRUN;
    void *db = ngenxx_store_sqlite_open((void *)sdk, file);
    lua_pushinteger(L, (long)db);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_sqlite_execute(void *conn, const char *sql)
{
    if (conn == NULL || sql == NULL)
        return false;
    NGenXX::Store::SQLite::Connection *xconn = (NGenXX::Store::SQLite::Connection *)conn;
    return xconn->execute(std::string(sql));
}

#ifdef USE_LUA
int ngenxx_store_sqlite_executeL(lua_State *L)
{
    long conn;
    char *sql = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, sql); 
    });
    if (conn <= 0 || sql == NULL) return LUA_ERRRUN;
    bool res = ngenxx_store_sqlite_execute((void *)conn, sql);
    lua_pushboolean(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void *ngenxx_store_sqlite_query_do(void *conn, const char *sql)
{
    if (conn == NULL || sql == NULL)
        return NULL;
    NGenXX::Store::SQLite::Connection *xconn = (NGenXX::Store::SQLite::Connection *)conn;
    return xconn->query(std::string(sql));
}

#ifdef USE_LUA
int ngenxx_store_sqlite_query_doL(lua_State *L)
{
    long conn;
    char *sql = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, sql); 
    });
    if (conn <= 0 || sql == NULL) return LUA_ERRRUN;
    void *res = ngenxx_store_sqlite_query_do((void *)conn, sql);
    lua_pushinteger(L, (long)res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_sqlite_query_read_row(void *query_result)
{
    if (query_result == NULL)
        return false;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    return xqr->readRow();
}

#ifdef USE_LUA
int ngenxx_store_sqlite_query_read_rowL(lua_State *L)
{
    long query_result;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, query_result); 
    });
    if (query_result <= 0) return LUA_ERRRUN;
    bool res = ngenxx_store_sqlite_query_read_row((void *)query_result);
    lua_pushboolean(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
const char *ngenxx_store_sqlite_query_read_column_text(void *query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return NULL;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    auto s = xqr->readColumnText(std::string(column));
    return str2charp(s);
}

#ifdef USE_LUA
int ngenxx_store_sqlite_query_read_column_textL(lua_State *L)
{
    long query_result;
    char *column = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, query_result);
        JSON_READ_STR(j, column); 
    });
    if (query_result <= 0 || column == NULL) return LUA_ERRRUN;
    const char *res = ngenxx_store_sqlite_query_read_column_text((void *)query_result, column);
    lua_pushstring(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
long long ngenxx_store_sqlite_query_read_column_integer(void *query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return 0;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    return xqr->readColumnInteger(std::string(column));
}

#ifdef USE_LUA
int ngenxx_store_sqlite_query_read_column_integerL(lua_State *L)
{
    long query_result;
    char *column = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, query_result);
        JSON_READ_STR(j, column); 
    });
    if (query_result <= 0 || column == NULL) return LUA_ERRRUN;
    long long res = ngenxx_store_sqlite_query_read_column_integer((void *)query_result, column);
    lua_pushinteger(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
double ngenxx_store_sqlite_query_read_column_float(void *query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return 0.f;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    return xqr->readColumnFloat(std::string(column));
}

#ifdef USE_LUA
int ngenxx_store_sqlite_query_read_column_floatL(lua_State *L)
{
    long query_result;
    char *column = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, query_result);
        JSON_READ_STR(j, column); 
    });
    if (query_result <= 0 || column == NULL) return LUA_ERRRUN;
    double res = ngenxx_store_sqlite_query_read_column_float((void *)query_result, column);
    lua_pushnumber(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_store_sqlite_query_drop(void *query_result)
{
    if (query_result == NULL)
        return;
    delete (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
}

#ifdef USE_LUA
int ngenxx_store_sqlite_query_dropL(lua_State *L)
{
    long query_result;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, query_result); 
    });
    if (query_result <= 0) return LUA_ERRRUN;
    ngenxx_store_sqlite_query_drop((void *)query_result);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_store_sqlite_close(void *conn)
{
    if (conn == NULL)
        return;
    delete (NGenXX::Store::SQLite::Connection *)conn;
}

#ifdef USE_LUA
int ngenxx_store_sqlite_closeL(lua_State *L)
{
    long conn;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, conn); 
    });
    if (conn <= 0) return LUA_ERRRUN;
    ngenxx_store_sqlite_close((void *)conn);
    return LUA_OK;
}
#endif

#pragma mark Store.KV

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void *ngenxx_store_kv_open(const char *uid)
{
    if (uid == NULL) return NULL;
    return new NGenXX::Store::KV(std::string(uid));
}

#ifdef USE_LUA
int ngenxx_store_kv_openL(lua_State *L)
{
    char *uid = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_STR(j, uid); 
    });
    void *res = ngenxx_store_kv_open(uid);
    lua_pushinteger(L, (long)res);
    return LUA_OK;
}

#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
const char *ngenxx_store_kv_read_string(void *kv, const char *k)
{
    if (kv == NULL || k == NULL) return NULL;
    return str2charp(((NGenXX::Store::KV *)kv)->readString(std::string(k)));
}

#ifdef USE_LUA
int ngenxx_store_kv_read_stringL(lua_State *L)
{
    long kv;
    char *k = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, kv);
        JSON_READ_STR(j, k); 
    });
    const char *res = ngenxx_store_kv_read_string((void *)kv, k);
    lua_pushstring(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_kv_write_string(void *kv, const char *k, const char *v)
{
    if (kv == NULL || k == NULL) return false;
    return ((NGenXX::Store::KV *)kv)->writeString(std::string(k), v);
}

#ifdef USE_LUA
int ngenxx_store_kv_write_stringL(lua_State *L)
{
    long kv;
    char *k = NULL;
    char *v = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, kv);
        JSON_READ_STR(j, k);
        JSON_READ_STR(j, v); 
    });
    bool res = ngenxx_store_kv_write_string((void *)kv, k, v);
    lua_pushboolean(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
long long ngenxx_store_kv_read_integer(void *kv, const char *k)
{
    if (kv == NULL || k == NULL) return 0;
    return ((NGenXX::Store::KV *)kv)->readInteger(std::string(k));
}

#ifdef USE_LUA
int ngenxx_store_kv_read_integerL(lua_State *L)
{
    long kv;
    char *k = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, kv);
        JSON_READ_STR(j, k); 
    });
    long long res = ngenxx_store_kv_read_integer((void *)kv, k);
    lua_pushinteger(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_kv_write_integer(void *kv, const char *k, long long v)
{
    if (kv == NULL || k == NULL) return false;
    return ((NGenXX::Store::KV *)kv)->writeInteger(std::string(k), v);
}

#ifdef USE_LUA
int ngenxx_store_kv_write_integerL(lua_State *L)
{
    long kv;
    char *k = NULL;
    long long v;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, kv);
        JSON_READ_STR(j, k);
        JSON_READ_NUM(j, v);
    });
    bool res = ngenxx_store_kv_write_integer((void *)kv, k, v);
    lua_pushboolean(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
double ngenxx_store_kv_read_float(void *kv, const char *k)
{
    if (kv == NULL || k == NULL) return 0;
    return ((NGenXX::Store::KV *)kv)->readFloat(std::string(k));
}

#ifdef USE_LUA
int ngenxx_store_kv_read_floatL(lua_State *L)
{
    long kv;
    char *k = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, kv);
        JSON_READ_STR(j, k); 
    });
    double res = ngenxx_store_kv_read_float((void *)kv, k);
    lua_pushnumber(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_kv_write_float(void *kv, const char *k, double v)
{
    if (kv == NULL || k == NULL) return false;
    return ((NGenXX::Store::KV *)kv)->writeFloat(std::string(k), v);
}

#ifdef USE_LUA
int ngenxx_store_kv_write_floatL(lua_State *L)
{
    long kv;
    char *k = NULL;
    double v;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, kv);
        JSON_READ_STR(j, k);
        JSON_READ_NUM(j, v); 
    });
    bool res = ngenxx_store_kv_write_float((void *)kv, k, v);
    lua_pushboolean(L, res);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_kv_contains(void *kv, const char *k)
{
    if (kv == NULL || k == NULL) return false;
    return ((NGenXX::Store::KV *)kv)->contains(std::string(k));
}

#ifdef USE_LUA
int ngenxx_store_kv_containsL(lua_State *L)
{
    long kv;
    char *k = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, kv);
        JSON_READ_STR(j, k); 
    });
    bool res = ngenxx_store_kv_contains((void *)kv, k);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_store_kv_clear(void *kv)
{
    if (kv == NULL) return;
    ((NGenXX::Store::KV *)kv)->clear();
}

#ifdef USE_LUA
int ngenxx_store_kv_clearL(lua_State *L)
{
    long kv;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, kv); 
    });
    ngenxx_store_kv_clear((void *)kv);
    return LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_store_kv_close(void *kv)
{
    if (kv == NULL) return;
    delete (NGenXX::Store::KV *)kv;
}

#ifdef USE_LUA
int ngenxx_store_kv_closeL(lua_State *L)
{
    long kv;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, kv); 
    });
    ngenxx_store_kv_close((void *)kv);
    return LUA_OK;
}
#endif

#ifdef USE_LUA

#pragma mark Lua

#ifndef __EMSCRIPTEN__
bool ngenxx_L_loadF(void *sdk, const char *file)
{
    return NGenXX::LuaBridge::loadFile(((NGenXXHandle *)sdk)->lua_state, file) == LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
bool ngenxx_L_loadS(void *sdk, const char *script)
{
    return NGenXX::LuaBridge::loadScript(((NGenXXHandle *)sdk)->lua_state, script) == LUA_OK;
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
const char *ngenxx_L_call(void *sdk, const char *func, const char *params)
{
    return NGenXX::LuaBridge::callFunc(((NGenXXHandle *)sdk)->lua_state, func, params);
}

void export_funcs_for_lua(void *handle)
{
    BIND_LUA_FUNC(handle, ngenxx_get_versionL);

    BIND_LUA_FUNC(handle, ngenxx_log_printL);

    BIND_LUA_FUNC(handle, ngenxx_net_http_requestL);

    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_openL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_executeL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_doL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_read_rowL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_read_column_textL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_read_column_integerL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_read_column_floatL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_dropL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_closeL);

    BIND_LUA_FUNC(handle, ngenxx_store_kv_openL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_read_stringL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_write_stringL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_read_integerL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_write_integerL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_read_floatL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_write_floatL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_containsL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_clearL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_closeL);
}

#endif