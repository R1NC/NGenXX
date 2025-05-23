#if defined(USE_LUA)
#include "NGenXX-Lua.hxx"

#include <memory>

#include "lua/LuaBridge.hxx"
#include "util/TypeUtil.hxx"
#include "NGenXX-inner.hxx"
#include "NGenXX-Script.hxx"

namespace {
    std::unique_ptr<NGenXX::LuaBridge> bridge = nullptr;

    #define DEF_API(f, T) DEF_LUA_FUNC_##T(f##L, f##S)

    #define BIND_API(f)                                                  \
        if (bridge) [[likely]] {                                         \
            bridge->bindFunc(#f, f##L);                                  \
        }

    bool loadF(const std::string &f)
    {
        if (!bridge || f.empty()) [[unlikely]]
        {
            return false;
        }
        return bridge->loadFile(f);
    }

    bool loadS(const std::string &s)
    {
        if (!bridge || s.empty()) [[unlikely]]
        {
            return false;
        }
        return bridge->loadScript(s);
    }

    std::string call(const std::string &f, const std::string &ps)
    {
        if (!bridge || f.empty()) [[unlikely]]
        {
            return {};
        }
        return bridge->callFunc(f, ps);
    }
}

#pragma mark Native API

#if !defined(__EMSCRIPTEN__)
EXPORT
bool ngenxx_lua_loadF(const char *file)
{
    return loadF(wrapStr(file));
}
#endif

EXPORT
bool ngenxx_lua_loadS(const char *script)
{
    return loadS(wrapStr(script));
}

EXPORT
const char *ngenxx_lua_call(const char *f, const char *ps)
{
    return copyStr(call(wrapStr(f), wrapStr(ps)));
}

#pragma mark Lua API - Declaration

DEF_API(ngenxx_get_version, STRING)
DEF_API(ngenxx_root_path, STRING)

DEF_API(ngenxx_device_type, INTEGER)
DEF_API(ngenxx_device_name, STRING)
DEF_API(ngenxx_device_manufacturer, STRING)
DEF_API(ngenxx_device_os_version, STRING)
DEF_API(ngenxx_device_cpu_arch, INTEGER)

DEF_API(ngenxx_log_print, VOID)

DEF_API(ngenxx_net_http_request, STRING)
DEF_API(ngenxx_net_http_download, BOOL)

DEF_API(ngenxx_store_sqlite_open, STRING)
DEF_API(ngenxx_store_sqlite_execute, BOOL)
DEF_API(ngenxx_store_sqlite_query_do, STRING)
DEF_API(ngenxx_store_sqlite_query_read_row, BOOL)
DEF_API(ngenxx_store_sqlite_query_read_column_text, STRING)
DEF_API(ngenxx_store_sqlite_query_read_column_integer, INTEGER)
DEF_API(ngenxx_store_sqlite_query_read_column_float, FLOAT)
DEF_API(ngenxx_store_sqlite_query_drop, VOID)
DEF_API(ngenxx_store_sqlite_close, VOID)

DEF_API(ngenxx_store_kv_open, STRING)
DEF_API(ngenxx_store_kv_read_string, STRING)
DEF_API(ngenxx_store_kv_write_string, BOOL)
DEF_API(ngenxx_store_kv_read_integer, INTEGER)
DEF_API(ngenxx_store_kv_write_integer, BOOL)
DEF_API(ngenxx_store_kv_read_float, FLOAT)
DEF_API(ngenxx_store_kv_write_float, BOOL)
DEF_API(ngenxx_store_kv_all_keys, STRING)
DEF_API(ngenxx_store_kv_contains, BOOL)
DEF_API(ngenxx_store_kv_remove, BOOL)
DEF_API(ngenxx_store_kv_clear, VOID)
DEF_API(ngenxx_store_kv_close, VOID)

DEF_API(ngenxx_coding_hex_bytes2str, STRING)
DEF_API(ngenxx_coding_hex_str2bytes, STRING)
DEF_API(ngenxx_coding_case_upper, STRING)
DEF_API(ngenxx_coding_case_lower, STRING)
DEF_API(ngenxx_coding_bytes2str, STRING)
DEF_API(ngenxx_coding_str2bytes, STRING)

DEF_API(ngenxx_crypto_rand, STRING)
DEF_API(ngenxx_crypto_aes_encrypt, STRING)
DEF_API(ngenxx_crypto_aes_decrypt, STRING)
DEF_API(ngenxx_crypto_aes_gcm_encrypt, STRING)
DEF_API(ngenxx_crypto_aes_gcm_decrypt, STRING)
DEF_API(ngenxx_crypto_hash_md5, STRING)
DEF_API(ngenxx_crypto_hash_sha256, STRING)
DEF_API(ngenxx_crypto_base64_encode, STRING)
DEF_API(ngenxx_crypto_base64_decode, STRING)

DEF_API(ngenxx_z_zip_init, STRING)
DEF_API(ngenxx_z_zip_input, INTEGER)
DEF_API(ngenxx_z_zip_process_do, STRING)
DEF_API(ngenxx_z_zip_process_finished, BOOL)
DEF_API(ngenxx_z_zip_release, VOID)
DEF_API(ngenxx_z_unzip_init, STRING)
DEF_API(ngenxx_z_unzip_input, INTEGER)
DEF_API(ngenxx_z_unzip_process_do, STRING)
DEF_API(ngenxx_z_unzip_process_finished, BOOL)
DEF_API(ngenxx_z_unzip_release, VOID)
DEF_API(ngenxx_z_bytes_zip, STRING)
DEF_API(ngenxx_z_bytes_unzip, STRING)

#pragma mark Lua API - Binding

static void registerFuncs()
{
    BIND_API(ngenxx_get_version);
    BIND_API(ngenxx_root_path);

    BIND_API(ngenxx_log_print);

    BIND_API(ngenxx_device_type);
    BIND_API(ngenxx_device_name);
    BIND_API(ngenxx_device_manufacturer);
    BIND_API(ngenxx_device_os_version);
    BIND_API(ngenxx_device_cpu_arch);

    BIND_API(ngenxx_net_http_request);
    BIND_API(ngenxx_net_http_download);

    BIND_API(ngenxx_store_sqlite_open);
    BIND_API(ngenxx_store_sqlite_execute);
    BIND_API(ngenxx_store_sqlite_query_do);
    BIND_API(ngenxx_store_sqlite_query_read_row);
    BIND_API(ngenxx_store_sqlite_query_read_column_text);
    BIND_API(ngenxx_store_sqlite_query_read_column_integer);
    BIND_API(ngenxx_store_sqlite_query_read_column_float);
    BIND_API(ngenxx_store_sqlite_query_drop);
    BIND_API(ngenxx_store_sqlite_close);

    BIND_API(ngenxx_store_kv_open);
    BIND_API(ngenxx_store_kv_read_string);
    BIND_API(ngenxx_store_kv_write_string);
    BIND_API(ngenxx_store_kv_read_integer);
    BIND_API(ngenxx_store_kv_write_integer);
    BIND_API(ngenxx_store_kv_read_float);
    BIND_API(ngenxx_store_kv_write_float);
    BIND_API(ngenxx_store_kv_all_keys);
    BIND_API(ngenxx_store_kv_contains);
    BIND_API(ngenxx_store_kv_remove);
    BIND_API(ngenxx_store_kv_clear);
    BIND_API(ngenxx_store_kv_close);

    BIND_API(ngenxx_coding_hex_bytes2str);
    BIND_API(ngenxx_coding_hex_str2bytes);
    BIND_API(ngenxx_coding_case_upper);
    BIND_API(ngenxx_coding_case_lower);
    BIND_API(ngenxx_coding_bytes2str);
    BIND_API(ngenxx_coding_str2bytes);

    BIND_API(ngenxx_crypto_rand);
    BIND_API(ngenxx_crypto_aes_encrypt);
    BIND_API(ngenxx_crypto_aes_decrypt);
    BIND_API(ngenxx_crypto_aes_gcm_encrypt);
    BIND_API(ngenxx_crypto_aes_gcm_decrypt);
    BIND_API(ngenxx_crypto_hash_md5);
    BIND_API(ngenxx_crypto_hash_sha256);
    BIND_API(ngenxx_crypto_base64_encode);
    BIND_API(ngenxx_crypto_base64_decode);

    BIND_API(ngenxx_z_zip_init);
    BIND_API(ngenxx_z_zip_input);
    BIND_API(ngenxx_z_zip_process_do);
    BIND_API(ngenxx_z_zip_process_finished);
    BIND_API(ngenxx_z_zip_release);
    BIND_API(ngenxx_z_unzip_init);
    BIND_API(ngenxx_z_unzip_input);
    BIND_API(ngenxx_z_unzip_process_do);
    BIND_API(ngenxx_z_unzip_process_finished);
    BIND_API(ngenxx_z_unzip_release);
    BIND_API(ngenxx_z_bytes_zip)
    BIND_API(ngenxx_z_bytes_unzip)
}

#pragma mark Inner API

void _ngenxx_lua_init()
{
    if (bridge) [[unlikely]]
    {
        return;
    }
    bridge = std::make_unique<NGenXX::LuaBridge>();
    registerFuncs();
}

void _ngenxx_lua_release()
{
    if (!bridge) [[unlikely]]
    {
        return;
    }
    bridge.reset();
}
#endif