#if defined(USE_QJS)
#include "NGenXX-Js.hxx"

#include <cstring>
#include <cstdlib>

#include <memory>
#include <functional>

#include "util/TypeUtil.hxx"
#include "NGenXX-inner.hxx"
#include "js/JsBridge.hxx"
#include "NGenXX-Script.hxx"

std::unique_ptr<NGenXX::JsBridge> _ngenxx_js = nullptr;
static std::function<const char *(const char *msg)> _NGenXX_J_msg_callback = nullptr;

#define BIND_JS_FUNC(f)                                                        \
  if (_ngenxx_js) [[likely]] {                                                 \
    _ngenxx_js->bindFunc(#f, f);                                               \
  }

std::string _ngenxx_js_ask_platform(const char *msg)
{
    std::string s;
    if (msg == nullptr || _NGenXX_J_msg_callback == nullptr)
    {
        return s;
    }
    auto len = std::strlen(msg);
    auto cMsg = mallocX<char>(len);
    std::strncpy(cMsg, msg, len);
    auto res = _NGenXX_J_msg_callback(cMsg);
    return res ?: s;
}

DEF_JS_FUNC_STRING(ngenxx_ask_platformJ, _ngenxx_js_ask_platform)

DEF_JS_FUNC_STRING(ngenxx_get_versionJ, ngenxx_get_versionS)
DEF_JS_FUNC_STRING(ngenxx_root_pathJ, ngenxx_root_pathS)

DEF_JS_FUNC_INT32(ngenxx_device_typeJ, ngenxx_device_typeS)
DEF_JS_FUNC_STRING(ngenxx_device_nameJ, ngenxx_device_nameS)
DEF_JS_FUNC_STRING(ngenxx_device_manufacturerJ, ngenxx_device_manufacturerS)
DEF_JS_FUNC_STRING(ngenxx_device_os_versionJ, ngenxx_device_os_versionS)
DEF_JS_FUNC_INT32(ngenxx_device_cpu_archJ, ngenxx_device_cpu_archS)

DEF_JS_FUNC_VOID(ngenxx_log_printJ, ngenxx_log_printS)

DEF_JS_FUNC_STRING_ASYNC(_ngenxx_js, ngenxx_net_http_requestJ, ngenxx_net_http_requestS)
DEF_JS_FUNC_BOOL_ASYNC(_ngenxx_js, ngenxx_net_http_downloadJ, ngenxx_net_http_downloadS)

DEF_JS_FUNC_STRING(ngenxx_store_sqlite_openJ, ngenxx_store_sqlite_openS)
DEF_JS_FUNC_BOOL_ASYNC(_ngenxx_js, ngenxx_store_sqlite_executeJ, ngenxx_store_sqlite_executeS)
DEF_JS_FUNC_STRING_ASYNC(_ngenxx_js, ngenxx_store_sqlite_query_doJ, ngenxx_store_sqlite_query_doS)
DEF_JS_FUNC_BOOL(ngenxx_store_sqlite_query_read_rowJ, ngenxx_store_sqlite_query_read_rowS)
DEF_JS_FUNC_STRING(ngenxx_store_sqlite_query_read_column_textJ, ngenxx_store_sqlite_query_read_column_textS)
DEF_JS_FUNC_INT64(ngenxx_store_sqlite_query_read_column_integerJ, ngenxx_store_sqlite_query_read_column_integerS)
DEF_JS_FUNC_FLOAT(ngenxx_store_sqlite_query_read_column_floatJ, ngenxx_store_sqlite_query_read_column_floatS)
DEF_JS_FUNC_VOID(ngenxx_store_sqlite_query_dropJ, ngenxx_store_sqlite_query_dropS)
DEF_JS_FUNC_VOID(ngenxx_store_sqlite_closeJ, ngenxx_store_sqlite_closeS)

DEF_JS_FUNC_STRING(ngenxx_store_kv_openJ, ngenxx_store_kv_openS)
DEF_JS_FUNC_STRING(ngenxx_store_kv_read_stringJ, ngenxx_store_kv_read_stringS)
DEF_JS_FUNC_BOOL(ngenxx_store_kv_write_stringJ, ngenxx_store_kv_write_stringS)
DEF_JS_FUNC_INT64(ngenxx_store_kv_read_integerJ, ngenxx_store_kv_read_integerS)
DEF_JS_FUNC_BOOL(ngenxx_store_kv_write_integerJ, ngenxx_store_kv_write_integerS)
DEF_JS_FUNC_FLOAT(ngenxx_store_kv_read_floatJ, ngenxx_store_kv_read_floatS)
DEF_JS_FUNC_BOOL(ngenxx_store_kv_write_floatJ, ngenxx_store_kv_write_floatS)
DEF_JS_FUNC_STRING(ngenxx_store_kv_all_keysJ, ngenxx_store_kv_all_keysS)
DEF_JS_FUNC_BOOL(ngenxx_store_kv_containsJ, ngenxx_store_kv_containsS)
DEF_JS_FUNC_BOOL(ngenxx_store_kv_removeJ, ngenxx_store_kv_removeS)
DEF_JS_FUNC_VOID(ngenxx_store_kv_clearJ, ngenxx_store_kv_clearS)
DEF_JS_FUNC_VOID(ngenxx_store_kv_closeJ, ngenxx_store_kv_closeS)

DEF_JS_FUNC_STRING(ngenxx_coding_hex_bytes2strJ, ngenxx_coding_hex_bytes2strS)
DEF_JS_FUNC_STRING(ngenxx_coding_hex_str2bytesJ, ngenxx_coding_hex_str2bytesS)
DEF_JS_FUNC_STRING(ngenxx_coding_bytes2strJ, ngenxx_coding_bytes2strS)
DEF_JS_FUNC_STRING(ngenxx_coding_str2bytesJ, ngenxx_coding_str2bytesS)
DEF_JS_FUNC_STRING(ngenxx_coding_case_upperJ, ngenxx_coding_case_upperS)
DEF_JS_FUNC_STRING(ngenxx_coding_case_lowerJ, ngenxx_coding_case_lowerS)

DEF_JS_FUNC_STRING(ngenxx_crypto_randJ, ngenxx_crypto_randS)
DEF_JS_FUNC_STRING(ngenxx_crypto_aes_encryptJ, ngenxx_crypto_aes_encryptS)
DEF_JS_FUNC_STRING(ngenxx_crypto_aes_decryptJ, ngenxx_crypto_aes_decryptS)
DEF_JS_FUNC_STRING(ngenxx_crypto_aes_gcm_encryptJ, ngenxx_crypto_aes_gcm_encryptS)
DEF_JS_FUNC_STRING(ngenxx_crypto_aes_gcm_decryptJ, ngenxx_crypto_aes_gcm_decryptS)
DEF_JS_FUNC_STRING(ngenxx_crypto_hash_md5J, ngenxx_crypto_hash_md5S)
DEF_JS_FUNC_STRING(ngenxx_crypto_hash_sha256J, ngenxx_crypto_hash_sha256S)
DEF_JS_FUNC_STRING(ngenxx_crypto_base64_encodeJ, ngenxx_crypto_base64_encodeS)
DEF_JS_FUNC_STRING(ngenxx_crypto_base64_decodeJ, ngenxx_crypto_base64_decodeS)

DEF_JS_FUNC_STRING(ngenxx_z_zip_initJ, ngenxx_z_zip_initS)
DEF_JS_FUNC_INT64(ngenxx_z_zip_inputJ, ngenxx_z_zip_inputS)
DEF_JS_FUNC_STRING(ngenxx_z_zip_process_doJ, ngenxx_z_zip_process_doS)
DEF_JS_FUNC_BOOL(ngenxx_z_zip_process_finishedJ, ngenxx_z_zip_process_finishedS)
DEF_JS_FUNC_VOID(ngenxx_z_zip_releaseJ, ngenxx_z_zip_releaseS)
DEF_JS_FUNC_STRING(ngenxx_z_unzip_initJ, ngenxx_z_unzip_initS)
DEF_JS_FUNC_INT64(ngenxx_z_unzip_inputJ, ngenxx_z_unzip_inputS)
DEF_JS_FUNC_STRING(ngenxx_z_unzip_process_doJ, ngenxx_z_unzip_process_doS)
DEF_JS_FUNC_BOOL(ngenxx_z_unzip_process_finishedJ, ngenxx_z_unzip_process_finishedS)
DEF_JS_FUNC_VOID(ngenxx_z_unzip_releaseJ, ngenxx_z_unzip_releaseS)
DEF_JS_FUNC_STRING_ASYNC(_ngenxx_js, ngenxx_z_bytes_zipJ, ngenxx_z_bytes_zipS)
DEF_JS_FUNC_STRING_ASYNC(_ngenxx_js, ngenxx_z_bytes_unzipJ, ngenxx_z_bytes_unzipS)

#pragma mark JS

bool ngenxxJsLoadF(const std::string &file, bool isModule)
{
    if (!_ngenxx_js || file.empty()) [[unlikely]]
    {
        return false;
    }
    return _ngenxx_js->loadFile(file, isModule);
}

bool ngenxxJsLoadS(const std::string &script, const std::string &name, bool isModule)
{
    if (!_ngenxx_js || script.empty() || name.empty()) [[unlikely]]
    {
        return false;
    }
    return _ngenxx_js->loadScript(script, name, isModule);
}

bool ngenxxJsLoadB(const Bytes &bytes, bool isModule)
{
    if (!_ngenxx_js || bytes.empty()) [[unlikely]]
    {
        return false;
    }
    return _ngenxx_js->loadBinary(bytes, isModule);
}

std::string ngenxxJsCall(const std::string &func, const std::string &params, bool await)
{
    std::string s;
    if (!_ngenxx_js || func.empty()) [[unlikely]]
    {
        return s;
    }
    return _ngenxx_js->callFunc(func, params, await);
}

void ngenxxJsSetMsgCallback(const std::function<const char *(const char *msg)> &callback)
{
    _NGenXX_J_msg_callback = callback;
}

EXPORT_AUTO
bool ngenxx_js_loadF(const char *file, bool is_module)
{
    return ngenxxJsLoadF(wrapStr(file), is_module);
}

EXPORT_AUTO
bool ngenxx_js_loadS(const char *script, const char *name, bool is_module)
{
    return ngenxxJsLoadS(wrapStr(script), wrapStr(name), is_module);
}

EXPORT_AUTO
bool ngenxx_js_loadB(const byte *bytes, size_t len, bool is_module)
{
    return ngenxxJsLoadB(wrapBytes(bytes, len), is_module);
}

EXPORT_AUTO
const char *ngenxx_js_call(const char *func, const char *params, bool await)
{
    return copyStr(ngenxxJsCall(wrapStr(func), wrapStr(params), await));
}

EXPORT_AUTO
void ngenxx_js_set_msg_callback(const char *(*const callback)(const char *msg))
{
    ngenxxJsSetMsgCallback(callback);
}

#pragma mark JS Module Register

void _ngenxx_js_registerFuncs()
{
    BIND_JS_FUNC(ngenxx_ask_platformJ);

    BIND_JS_FUNC(ngenxx_get_versionJ);
    BIND_JS_FUNC(ngenxx_root_pathJ);

    BIND_JS_FUNC(ngenxx_log_printJ);

    BIND_JS_FUNC(ngenxx_device_typeJ);
    BIND_JS_FUNC(ngenxx_device_nameJ);
    BIND_JS_FUNC(ngenxx_device_manufacturerJ);
    BIND_JS_FUNC(ngenxx_device_os_versionJ);
    BIND_JS_FUNC(ngenxx_device_cpu_archJ);

    BIND_JS_FUNC(ngenxx_net_http_requestJ);
    BIND_JS_FUNC(ngenxx_net_http_downloadJ);

    BIND_JS_FUNC(ngenxx_store_sqlite_openJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_executeJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_doJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_read_rowJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_read_column_textJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_read_column_integerJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_read_column_floatJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_dropJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_closeJ);

    BIND_JS_FUNC(ngenxx_store_kv_openJ);
    BIND_JS_FUNC(ngenxx_store_kv_read_stringJ);
    BIND_JS_FUNC(ngenxx_store_kv_write_stringJ);
    BIND_JS_FUNC(ngenxx_store_kv_read_integerJ);
    BIND_JS_FUNC(ngenxx_store_kv_write_integerJ);
    BIND_JS_FUNC(ngenxx_store_kv_read_floatJ);
    BIND_JS_FUNC(ngenxx_store_kv_write_floatJ);
    BIND_JS_FUNC(ngenxx_store_kv_all_keysJ);
    BIND_JS_FUNC(ngenxx_store_kv_containsJ);
    BIND_JS_FUNC(ngenxx_store_kv_removeJ);
    BIND_JS_FUNC(ngenxx_store_kv_clearJ);
    BIND_JS_FUNC(ngenxx_store_kv_closeJ);

    BIND_JS_FUNC(ngenxx_coding_hex_bytes2strJ);
    BIND_JS_FUNC(ngenxx_coding_hex_str2bytesJ);
    BIND_JS_FUNC(ngenxx_coding_bytes2strJ);
    BIND_JS_FUNC(ngenxx_coding_str2bytesJ);
    BIND_JS_FUNC(ngenxx_coding_case_upperJ);
    BIND_JS_FUNC(ngenxx_coding_case_lowerJ);

    BIND_JS_FUNC(ngenxx_crypto_randJ);
    BIND_JS_FUNC(ngenxx_crypto_aes_encryptJ);
    BIND_JS_FUNC(ngenxx_crypto_aes_decryptJ);
    BIND_JS_FUNC(ngenxx_crypto_aes_gcm_encryptJ);
    BIND_JS_FUNC(ngenxx_crypto_aes_gcm_decryptJ);
    BIND_JS_FUNC(ngenxx_crypto_hash_md5J);
    BIND_JS_FUNC(ngenxx_crypto_hash_sha256J);
    BIND_JS_FUNC(ngenxx_crypto_base64_encodeJ);
    BIND_JS_FUNC(ngenxx_crypto_base64_decodeJ);

    BIND_JS_FUNC(ngenxx_z_zip_initJ);
    BIND_JS_FUNC(ngenxx_z_zip_inputJ);
    BIND_JS_FUNC(ngenxx_z_zip_process_doJ);
    BIND_JS_FUNC(ngenxx_z_zip_process_finishedJ);
    BIND_JS_FUNC(ngenxx_z_zip_releaseJ);
    BIND_JS_FUNC(ngenxx_z_unzip_initJ);
    BIND_JS_FUNC(ngenxx_z_unzip_inputJ);
    BIND_JS_FUNC(ngenxx_z_unzip_process_doJ);
    BIND_JS_FUNC(ngenxx_z_unzip_process_finishedJ);
    BIND_JS_FUNC(ngenxx_z_unzip_releaseJ);
    BIND_JS_FUNC(ngenxx_z_bytes_zipJ)
    BIND_JS_FUNC(ngenxx_z_bytes_unzipJ)
}

void _ngenxx_js_init(void)
{
    if (_ngenxx_js) [[unlikely]]
    {
        return;
    }
    _ngenxx_js = std::make_unique<NGenXX::JsBridge>();
    _ngenxx_js_registerFuncs();
}

void _ngenxx_js_release(void)
{
    if (!_ngenxx_js) [[unlikely]]
    {
        return;
    }
    _ngenxx_js.reset();
    _NGenXX_J_msg_callback = nullptr;
}
#endif