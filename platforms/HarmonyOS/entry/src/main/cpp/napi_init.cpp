#include "napi/native_api.h"
#include "napi_util.h"
#include "../../../../../../build.HarmonyOS/output/include/NGenXX.h"
#include <cstring>
#include <string.h>
#include <stdlib.h>

static napi_value GetVersion(napi_env env, napi_callback_info info) {
    const char *c = ngenxx_get_version();
    napi_value v = chars2NapiValue(env, c);
    free(static_cast<void *>(const_cast<char *>(c)));
    return v;
}

static napi_value Init(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *root = napiValue2chars(env, argv[0]);

    bool b = ngenxx_init(root);
    napi_value v = bool2NapiValue(env, b);

    free(static_cast<void *>(const_cast<char *>(root)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value Release(napi_env env, napi_callback_info info) {
    ngenxx_release();

    return int2NapiValue(env, napi_ok);
}

#pragma mark Log Callback

typedef struct {
    napi_async_work tsWork;
    napi_threadsafe_function tsWorkFunc;
    int logLevel;
    const char *logContent;
} TSLogWorkData;

static napi_env sNapiEnv;
static napi_ref sTsLogCallbackRef;

static void OnLogWorkCallTS(napi_env env, napi_value ts_callback, void *context, void *data) {
    if (env == NULL || ts_callback == NULL || data == NULL)
        return;

    TSLogWorkData *tSLogWorkData = (TSLogWorkData *)data;

    size_t argc = 2;
    napi_value argv[2];
    argv[0] = int2NapiValue(env, tSLogWorkData->logLevel);
    argv[1] = chars2NapiValue(env, tSLogWorkData->logContent);

    napi_value vGlobal;
    napi_status status = napi_get_global(env, &vGlobal);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_get_global() failed");

    napi_get_reference_value(sNapiEnv, sTsLogCallbackRef, &ts_callback);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_get_reference_value() failed");

    status = napi_call_function(env, vGlobal, ts_callback, argc, argv, NULL);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_call_function() failed");

    free(static_cast<void *>(const_cast<char *>(tSLogWorkData->logContent)));
    free(static_cast<void *>(tSLogWorkData));
}

static void OnLogWorkExecute(napi_env env, void *data) {
    TSLogWorkData *tSLogWorkData = (TSLogWorkData *)data;

    napi_status status = napi_acquire_threadsafe_function(tSLogWorkData->tsWorkFunc);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_acquire_threadsafe_function() failed");

    status = napi_call_threadsafe_function(tSLogWorkData->tsWorkFunc, tSLogWorkData, napi_tsfn_blocking);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_call_threadsafe_function() failed");
}

static void OnLogWorkComplete(napi_env env, napi_status status, void *data) {
    TSLogWorkData *tSLogWorkData = (TSLogWorkData *)data;

    status = napi_release_threadsafe_function(tSLogWorkData->tsWorkFunc, napi_tsfn_release);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_release_threadsafe_function() failed");

    status = napi_delete_async_work(env, tSLogWorkData->tsWork);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_delete_async_work() failed");
}

static void engineLogCallback(int level, const char *content) {
    if (sNapiEnv == NULL || content == NULL)
        return;

    TSLogWorkData *tSLogWorkData = reinterpret_cast<TSLogWorkData *>(malloc(sizeof(TSLogWorkData)));
    tSLogWorkData->tsWork = NULL;
    tSLogWorkData->tsWorkFunc = NULL;
    tSLogWorkData->logLevel = level;
    tSLogWorkData->logContent = reinterpret_cast<char *>(malloc(strlen(content) + 1));
    strcpy(const_cast<char *>(tSLogWorkData->logContent), content);
    free(static_cast<void *>(const_cast<char *>(content)));

    napi_value vWorkName = chars2NapiValue(sNapiEnv, "NAPI_LOG_CALLBACK_WORK");

    napi_value vTsCallback;
    napi_status status = napi_get_reference_value(sNapiEnv, sTsLogCallbackRef, &vTsCallback);
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_get_reference_value() failed");

    status = napi_create_threadsafe_function(sNapiEnv, vTsCallback, NULL, vWorkName, 0, 1, NULL, NULL, NULL,
                                             OnLogWorkCallTS, &(tSLogWorkData->tsWorkFunc));
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_create_threadsafe_function() failed");

    status = napi_create_async_work(sNapiEnv, NULL, vWorkName, OnLogWorkExecute, OnLogWorkComplete, tSLogWorkData,
                                    &(tSLogWorkData->tsWork));
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_create_async_work() failed");

    status = napi_queue_async_work(sNapiEnv, tSLogWorkData->tsWork);
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_queue_async_work() failed");
}

#pragma mark Log API

static napi_value LogSetLevel(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    int level = napiValue2int(env, argv[0]);

    ngenxx_log_set_level(level);

    free(static_cast<void *>(argv));
    return int2NapiValue(env, napi_ok);
}

static napi_value LogSetCallback(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    napi_value vLogCallback = argv[0];
    int status;
    if (vLogCallback == NULL) {
        sNapiEnv = NULL;

        status = napi_delete_reference(env, sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_delete_reference() failed");

        ngenxx_log_set_callback(NULL);
    } else {
        sNapiEnv = env;

        status = napi_create_reference(env, vLogCallback, 1, &sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_reference() failed");

        ngenxx_log_set_callback(engineLogCallback);
    }

    free(static_cast<void *>(argv));
    return int2NapiValue(env, napi_ok);
}

static napi_value LogPrint(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    int level = napiValue2int(env, argv[0]);
    const char *content = napiValue2chars(env, argv[1]);

    ngenxx_log_print(level, content);
    free(static_cast<void *>(const_cast<char *>(content)));

    free(static_cast<void *>(argv));
    return int2NapiValue(env, napi_ok);
}

#pragma mark Net

static napi_value NetHttpRequest(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 10);

    const char *cUrl = napiValue2chars(env, argv[0]);
    const char *cParams = napiValue2chars(env, argv[1]);
    int iMethod = napiValue2int(env, argv[2]);

    uint32_t header_c = napiValueArrayLen(env, argv[3]);
    const char **header_v = napiValue2charsArray(env, argv[3], header_c);
    
    uint32_t form_field_count = napiValueArrayLen(env, argv[4]);
    const char **form_field_name_v = napiValue2charsArray(env, argv[4], form_field_count);
    const char **form_field_mime_v = napiValue2charsArray(env, argv[5], form_field_count);
    const char **form_field_data_v = napiValue2charsArray(env, argv[6], form_field_count);
    
    const char *cFilePath = napiValue2chars(env, argv[7]);
    FILE *cFILE = cFilePath ? std::fopen(cFilePath, "r") : nullptr;
    long fileLength = napiValue2long(env, argv[8]);

    long lTimeout = napiValue2long(env, argv[9]);

    const char *res = ngenxx_net_http_request(cUrl, cParams, iMethod, 
    header_v, header_c, 
    form_field_name_v, form_field_mime_v, form_field_data_v, form_field_count,
    static_cast<void *>(cFILE), fileLength,
    lTimeout);
    napi_value nv = chars2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(res)));
    for (int i = 0; i < header_c; i++) {
        free(static_cast<void *>(const_cast<char *>(header_v[i])));
    }
    for (int i = 0; i < form_field_count; i++) {
        free(static_cast<void *>(const_cast<char *>(form_field_name_v[i])));
        free(static_cast<void *>(const_cast<char *>(form_field_mime_v[i])));
        free(static_cast<void *>(const_cast<char *>(form_field_data_v[i])));
    }
    if (cParams) free(static_cast<void *>(const_cast<char *>(cParams)));
    if (cUrl) free(static_cast<void *>(const_cast<char *>(cUrl)));
    free(static_cast<void *>(argv));
    return nv;
}

#pragma mark Store.SQLite

static napi_value StoreSQLiteOpen(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *_id = napiValue2chars(env, argv[0]);
    long res = reinterpret_cast<long>(ngenxx_store_sqlite_open(_id));
    napi_value nv = long2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(_id)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreSQLiteExecute(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *sql = napiValue2chars(env, argv[1]);

    bool res = ngenxx_store_sqlite_execute(reinterpret_cast<void *>(conn), sql);
    napi_value nv = bool2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(sql)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreSQLiteQueryDo(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *sql = napiValue2chars(env, argv[1]);

    long res = reinterpret_cast<long>(ngenxx_store_sqlite_query_do(reinterpret_cast<void *>(conn), sql));
    napi_value nv = long2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(sql)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreSQLiteQueryReadRow(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long query_result = napiValue2long(env, argv[0]);

    bool res = ngenxx_store_sqlite_query_read_row(reinterpret_cast<void *>(query_result));
    napi_value nv = bool2NapiValue(env, res);

    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreSQLiteQueryReadColumnText(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long query_result = napiValue2long(env, argv[0]);
    const char *column = napiValue2chars(env, argv[1]);

    const char *res = ngenxx_store_sqlite_query_read_column_text(reinterpret_cast<void *>(query_result), column);
    napi_value nv = chars2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(res)));
    free(static_cast<void *>(const_cast<char *>(column)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreSQLiteQueryReadColumnInteger(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long query_result = napiValue2long(env, argv[0]);
    const char *column = napiValue2chars(env, argv[1]);

    long res = ngenxx_store_sqlite_query_read_column_integer(reinterpret_cast<void *>(query_result), column);
    napi_value nv = long2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(column)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreSQLiteQueryReadColumnFloat(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long query_result = napiValue2long(env, argv[0]);
    const char *column = napiValue2chars(env, argv[1]);

    double res = ngenxx_store_sqlite_query_read_column_float(reinterpret_cast<void *>(query_result), column);
    napi_value nv = double2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(column)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreSQLiteQueryDrop(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long query_result = napiValue2long(env, argv[0]);
    ngenxx_store_sqlite_query_drop(reinterpret_cast<void *>(query_result));

    free(static_cast<void *>(argv));
    return int2NapiValue(env, napi_ok);
}

static napi_value StoreSQLiteClose(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long conn = napiValue2long(env, argv[0]);
    ngenxx_store_sqlite_close(reinterpret_cast<void *>(conn));

    free(static_cast<void *>(argv));
    return int2NapiValue(env, napi_ok);
}

#pragma mark Store.KV

static napi_value StoreKVOpen(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *_id = napiValue2chars(env, argv[0]);
    long res = (long)ngenxx_store_kv_open(_id);
    napi_value nv = long2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(_id)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreKVReadString(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    const char *res = ngenxx_store_kv_read_string(reinterpret_cast<void *>(conn), k);
    napi_value nv = chars2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(res)));
    free(static_cast<void *>(const_cast<char *>(k)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreKVWriteString(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 3);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);
    const char *v = napiValue2chars(env, argv[2]);

    bool res = ngenxx_store_kv_write_string(reinterpret_cast<void *>(conn), k, v);
    napi_value nv = bool2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(v)));
    free(static_cast<void *>(const_cast<char *>(k)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreKVReadInteger(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    long res = ngenxx_store_kv_read_integer(reinterpret_cast<void *>(conn), k);
    napi_value nv = long2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(k)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreKVWriteInteger(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 3);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);
    long v = napiValue2long(env, argv[2]);

    bool res = ngenxx_store_kv_write_integer(reinterpret_cast<void *>(conn), k, v);
    napi_value nv = bool2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(k)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreKVReadFloat(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    double res = ngenxx_store_kv_read_integer(reinterpret_cast<void *>(conn), k);
    napi_value nv = double2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(k)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreKVWriteFloat(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 3);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);
    double v = napiValue2double(env, argv[2]);

    bool res = ngenxx_store_kv_write_integer(reinterpret_cast<void *>(conn), k, v);
    napi_value nv = bool2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(k)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreKVContains(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    bool res = ngenxx_store_kv_contains(reinterpret_cast<void *>(conn), k);
    napi_value nv = bool2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(k)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value StoreKVClear(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long conn = napiValue2long(env, argv[0]);
    ngenxx_store_kv_clear(reinterpret_cast<void *>(conn));

    free(static_cast<void *>(argv));
    return int2NapiValue(env, napi_ok);
}

static napi_value StoreKVClose(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long conn = napiValue2long(env, argv[0]);
    ngenxx_store_kv_close(reinterpret_cast<void *>(conn));

    free(static_cast<void *>(argv));
    return int2NapiValue(env, napi_ok);
}

#pragma mark DeviceInfo

static napi_value DeviceType(napi_env env, napi_callback_info info) {
    int dt = ngenxx_device_type();
    napi_value v = int2NapiValue(env, dt);
    return v;
}

static napi_value DeviceName(napi_env env, napi_callback_info info) {
    const char *cDN = ngenxx_device_name();
    napi_value v = chars2NapiValue(env, cDN);
    free(static_cast<void *>(const_cast<char *>(cDN)));
    return v;
}

static napi_value DeviceManufacturer(napi_env env, napi_callback_info info) {
    const char *cDM = ngenxx_device_manufacturer();
    napi_value v = chars2NapiValue(env, cDM);
    free(static_cast<void *>(const_cast<char *>(cDM)));
    return v;
}

static napi_value DeviceOsVersion(napi_env env, napi_callback_info info) {
    const char *cOV = ngenxx_device_name();
    napi_value v = chars2NapiValue(env, cOV);
    free(static_cast<void *>(const_cast<char *>(cOV)));
    return v;
}

static napi_value DeviceCpuArch(napi_env env, napi_callback_info info) {
    int dca = ngenxx_device_cpu_arch();
    napi_value v = int2NapiValue(env, dca);
    return v;
}

#pragma mark JsonDecoder

static napi_value JsonDecoderInit(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *cJson = napiValue2chars(env, argv[0]);
    
    long res = reinterpret_cast<long>(ngenxx_json_decoder_init(cJson));
    napi_value v = long2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(cJson)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value JsonDecoderIsArray(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    bool res = ngenxx_json_decoder_is_array(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node));
    napi_value v = bool2NapiValue(env, res);
    
    free(static_cast<void *>(argv));
    return v;
}

static napi_value JsonDecoderIsObject(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    bool res = ngenxx_json_decoder_is_object(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node));
    napi_value v = bool2NapiValue(env, res);
    
    free(static_cast<void *>(argv));
    return v;
}

static napi_value JsonDecoderReadNode(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 3);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    const char *cK = napiValue2chars(env, argv[2]);
    
    long res = (long)ngenxx_json_decoder_read_node(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node), cK);
    napi_value v = long2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(cK)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value JsonDecoderReadChild(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    long res = (long)ngenxx_json_decoder_read_child(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node));
    napi_value v = long2NapiValue(env, res);
    
    free(static_cast<void *>(argv));
    return v;
}

static napi_value JsonDecoderReadNext(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    long res = (long)ngenxx_json_decoder_read_next(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node));
    napi_value v = long2NapiValue(env, res);
    
    free(static_cast<void *>(argv));
    return v;
}

static napi_value JsonDecoderReadString(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    const char *cRes = ngenxx_json_decoder_read_string(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node));
    napi_value v = chars2NapiValue(env, cRes);
    
    free(static_cast<void *>(const_cast<char *>(cRes)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value JsonDecoderReadNumber(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    double res = ngenxx_json_decoder_read_number(reinterpret_cast<void *>(decoder), reinterpret_cast<void *>(node));
    napi_value v = double2NapiValue(env, res);
    
    free(static_cast<void *>(argv));
    return v;
}

static napi_value JsonDecoderRelease(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long decoder = napiValue2long(env, argv[0]);
    
    ngenxx_json_decoder_release(reinterpret_cast<void *>(decoder));
    
    free(static_cast<void *>(argv));
    return int2NapiValue(env, napi_ok);
}

#pragma mark Coding

static napi_value CodingHexBytes2str(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);
    
    uint32_t inLen = napiValueArrayLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    
    auto cRes = ngenxx_coding_hex_bytes2str(inBytes, inLen);
    napi_value v = chars2NapiValue(env, cRes);
    
    free(static_cast<void *>(const_cast<char *>(cRes)));
    free(static_cast<void *>(const_cast<byte *>(inBytes)));
    return v;
}

static napi_value CodingHexStr2Bytes(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);
    
    const char *cStr = napiValue2chars(env, argv[0]);
    
    size outLen;
    auto cRes = ngenxx_coding_hex_str2bytes(cStr, &outLen);
    napi_value v = byteArray2NapiValue(env, cRes, outLen);
    
    free(static_cast<void *>(const_cast<byte *>(cRes)));
    free(static_cast<void *>(const_cast<char *>(cStr)));
    return v;
}

#pragma mark Crypto

static napi_value CryptoRand(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);
    
    uint32_t len = napiValue2int(env, argv[0]);
    byte out[len];
    memset(out, 0, len);
    
    ngenxx_crypto_rand(len, out);
    napi_value v = byteArray2NapiValue(env, out, len);
    
    return v;
}

static napi_value CryptoAesEncrypt(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    uint32_t inLen = napiValueArrayLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    uint32_t keyLen = napiValueArrayLen(env, argv[1]);
    const byte *keyBytes = napiValue2byteArray(env, argv[1], keyLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_aes_encrypt(inBytes, inLen, keyBytes, keyLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free(static_cast<void *>(const_cast<byte *>(outBytes)));
    free(static_cast<void *>(const_cast<byte *>(keyBytes)));
    free(static_cast<void *>(const_cast<byte *>(inBytes)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value CryptoAesDecrypt(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    uint32_t inLen = napiValueArrayLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    uint32_t keyLen = napiValueArrayLen(env, argv[1]);
    const byte *keyBytes = napiValue2byteArray(env, argv[1], keyLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_aes_decrypt(inBytes, inLen, keyBytes, keyLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free(static_cast<void *>(const_cast<byte *>(outBytes)));
    free(static_cast<void *>(const_cast<byte *>(keyBytes)));
    free(static_cast<void *>(const_cast<byte *>(inBytes)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value CryptoAesGcmEncrypt(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 5);

    size inLen = napiValueArrayLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    size keyLen = napiValueArrayLen(env, argv[1]);
    const byte *keyBytes = napiValue2byteArray(env, argv[1], keyLen);
    size initVectorLen = napiValueArrayLen(env, argv[2]);
    const byte *initVectorBytes = napiValue2byteArray(env, argv[2], initVectorLen);
    size aadLen = napiValueArrayLen(env, argv[3]);
    const byte *aadBytes = napiValue2byteArray(env, argv[3], aadLen);
    size tagBits = napiValue2int(env, argv[4]);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_aes_gcm_encrypt(inBytes, inLen, keyBytes, keyLen, initVectorBytes, initVectorLen, aadBytes, aadLen, tagBits, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free(static_cast<void *>(const_cast<byte *>(outBytes)));
    if (aadBytes) free(static_cast<void *>(const_cast<byte *>(aadBytes)));
    free(static_cast<void *>(const_cast<byte *>(initVectorBytes)));
    free(static_cast<void *>(const_cast<byte *>(keyBytes)));
    free(static_cast<void *>(const_cast<byte *>(inBytes)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value CryptoAesGcmDecrypt(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 5);

    size inLen = napiValueArrayLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    size keyLen = napiValueArrayLen(env, argv[1]);
    const byte *keyBytes = napiValue2byteArray(env, argv[1], keyLen);
    size initVectorLen = napiValueArrayLen(env, argv[2]);
    const byte *initVectorBytes = napiValue2byteArray(env, argv[2], initVectorLen);
    size aadLen = napiValueArrayLen(env, argv[3]);
    const byte *aadBytes = napiValue2byteArray(env, argv[3], aadLen);
    size tagBits = napiValue2int(env, argv[4]);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_aes_gcm_decrypt(inBytes, inLen, keyBytes, keyLen, initVectorBytes, initVectorLen, aadBytes, aadLen, tagBits, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free(static_cast<void *>(const_cast<byte *>(outBytes)));
    if (aadBytes) free(static_cast<void *>(const_cast<byte *>(aadBytes)));
    free(static_cast<void *>(const_cast<byte *>(initVectorBytes)));
    free(static_cast<void *>(const_cast<byte *>(keyBytes)));
    free(static_cast<void *>(const_cast<byte *>(inBytes)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value CryptoHashMd5(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    uint32_t inLen = napiValueArrayLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_hash_md5(inBytes, inLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free(static_cast<void *>(const_cast<byte *>(outBytes)));
    free(static_cast<void *>(const_cast<byte *>(inBytes)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value CryptoHashSha256(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    uint32_t inLen = napiValueArrayLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_hash_sha256(inBytes, inLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free(static_cast<void *>(const_cast<byte *>(outBytes)));
    free(static_cast<void *>(const_cast<byte *>(inBytes)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value CryptoBase64Encode(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    uint32_t inLen = napiValueArrayLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_base64_encode(inBytes, inLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free(static_cast<void *>(const_cast<byte *>(outBytes)));
    free(static_cast<void *>(const_cast<byte *>(inBytes)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value CryptoBase64Decode(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    uint32_t inLen = napiValueArrayLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_base64_decode(inBytes, inLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free(static_cast<void *>(const_cast<byte *>(outBytes)));
    free(static_cast<void *>(const_cast<byte *>(inBytes)));
    free(static_cast<void *>(argv));
    return v;
}

#pragma mark Lua

static napi_value LLoadF(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *file = napiValue2chars(env, argv[0]);

    bool res = ngenxx_L_loadF(file);
    napi_value nv = bool2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(file)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value LLoadS(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *script = napiValue2chars(env, argv[0]);

    bool res = ngenxx_L_loadS(script);
    napi_value nv = bool2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(script)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value LCall(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    const char *func = napiValue2chars(env, argv[0]);
    const char *params = napiValue2chars(env, argv[1]);

    const char *res = ngenxx_L_call(func, params);
    napi_value nv = chars2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(res)));
    free(static_cast<void *>(const_cast<char *>(params)));
    free(static_cast<void *>(const_cast<char *>(func)));
    free(static_cast<void *>(argv));
    return nv;
}

#pragma mark JS

static napi_value JLoadF(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *file = napiValue2chars(env, argv[0]);

    bool res = ngenxx_J_loadF(file);
    napi_value nv = bool2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(file)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value JLoadS(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    const char *script = napiValue2chars(env, argv[0]);
    const char *name = napiValue2chars(env, argv[1]);

    bool res = ngenxx_J_loadS(script, name);
    napi_value nv = bool2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(script)));
    free(static_cast<void *>(const_cast<char *>(name)));
    free(static_cast<void *>(argv));
    return nv;
}

static napi_value JLoadB(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    uint32_t inLen = napiValueArrayLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    
    size outLen;
    bool b = ngenxx_J_loadB(inBytes, inLen);
    napi_value v = bool2NapiValue(env, b);
    
    free(static_cast<void *>(const_cast<byte *>(inBytes)));
    free(static_cast<void *>(argv));
    return v;
}

static napi_value JCall(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    const char *func = napiValue2chars(env, argv[0]);
    const char *params = napiValue2chars(env, argv[1]);

    const char *res = ngenxx_J_call(func, params);
    napi_value nv = chars2NapiValue(env, res);

    free(static_cast<void *>(const_cast<char *>(res)));
    free(static_cast<void *>(const_cast<char *>(params)));
    free(static_cast<void *>(const_cast<char *>(func)));
    free(static_cast<void *>(argv));
    return nv;
}

#pragma mark Register Module

#define DECLARE_NAPI_FUNC(name, funcPtr) {name, nullptr, funcPtr, nullptr, nullptr, nullptr, napi_default, nullptr}

EXTERN_C_START
static napi_value RegisterFuncs(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNC("getVersion", GetVersion),
        DECLARE_NAPI_FUNC("init", Init),
        DECLARE_NAPI_FUNC("release", Release),

        DECLARE_NAPI_FUNC("logSetLevel", LogSetLevel),
        DECLARE_NAPI_FUNC("logSetCallback", LogSetCallback),
        DECLARE_NAPI_FUNC("logPrint", LogPrint),

        DECLARE_NAPI_FUNC("netHttpRequest", NetHttpRequest),

        DECLARE_NAPI_FUNC("lLoadF", LLoadF),
        DECLARE_NAPI_FUNC("lLoadS", LLoadS),
        DECLARE_NAPI_FUNC("lCall", LCall),

        DECLARE_NAPI_FUNC("jLoadF", JLoadF),
        DECLARE_NAPI_FUNC("jLoadS", JLoadS),
        DECLARE_NAPI_FUNC("jLoadB", JLoadB),
        DECLARE_NAPI_FUNC("jCall", JCall),

        DECLARE_NAPI_FUNC("storeSQLiteOpen", StoreSQLiteOpen),
        DECLARE_NAPI_FUNC("storeSQLiteExecute", StoreSQLiteExecute),
        DECLARE_NAPI_FUNC("storeSQLiteQueryDo", StoreSQLiteQueryDo),
        DECLARE_NAPI_FUNC("storeSQLiteQueryReadRow", StoreSQLiteQueryReadRow),
        DECLARE_NAPI_FUNC("storeSQLiteQueryReadColumnText", StoreSQLiteQueryReadColumnText),
        DECLARE_NAPI_FUNC("storeSQLiteQueryReadColumnInteger", StoreSQLiteQueryReadColumnInteger),
        DECLARE_NAPI_FUNC("storeSQLiteQueryReadColumnFloat", StoreSQLiteQueryReadColumnFloat),
        DECLARE_NAPI_FUNC("storeSQLiteQueryDrop", StoreSQLiteQueryDrop),
        DECLARE_NAPI_FUNC("storeSQLiteClose", StoreSQLiteClose),

        DECLARE_NAPI_FUNC("storeKVOpen", StoreKVOpen),
        DECLARE_NAPI_FUNC("storeKVReadString", StoreKVReadString),
        DECLARE_NAPI_FUNC("storeKVWriteString", StoreKVWriteString),
        DECLARE_NAPI_FUNC("storeKVReadInteger", StoreKVReadInteger),
        DECLARE_NAPI_FUNC("storeKVWriteInteger", StoreKVWriteInteger),
        DECLARE_NAPI_FUNC("storeKVReadFloat", StoreKVReadFloat),
        DECLARE_NAPI_FUNC("storeKVWriteFloat", StoreKVWriteFloat),
        DECLARE_NAPI_FUNC("storeKVContains", StoreKVContains),
        DECLARE_NAPI_FUNC("storeKVClear", StoreKVClear),
        DECLARE_NAPI_FUNC("storeKVClose", StoreKVClose),

        DECLARE_NAPI_FUNC("deviceType", DeviceType),
        DECLARE_NAPI_FUNC("deviceName", DeviceName),
        DECLARE_NAPI_FUNC("deviceManufacturer", DeviceManufacturer),
        DECLARE_NAPI_FUNC("deviceOsVersion", DeviceOsVersion),
        DECLARE_NAPI_FUNC("deviceCpuArch", DeviceCpuArch),

        DECLARE_NAPI_FUNC("jsonDecoderInit", JsonDecoderInit),
        DECLARE_NAPI_FUNC("jsonDecoderIsArray", JsonDecoderIsArray),
        DECLARE_NAPI_FUNC("jsonDecoderIsObject", JsonDecoderIsObject),
        DECLARE_NAPI_FUNC("jsonDecoderReadNode", JsonDecoderReadNode),
        DECLARE_NAPI_FUNC("jsonDecoderReadChild", JsonDecoderReadChild),
        DECLARE_NAPI_FUNC("jsonDecoderReadNext", JsonDecoderReadNext),
        DECLARE_NAPI_FUNC("jsonDecoderReadString", JsonDecoderReadString),
        DECLARE_NAPI_FUNC("jsonDecoderReadNumber", JsonDecoderReadNumber),
        DECLARE_NAPI_FUNC("jsonDecoderRelease", JsonDecoderRelease),

        DECLARE_NAPI_FUNC("codingHexBytes2str", CodingHexBytes2str),
        DECLARE_NAPI_FUNC("codingHexStr2Bytes", CodingHexStr2Bytes),

        DECLARE_NAPI_FUNC("cryptoRand", CryptoRand),
        DECLARE_NAPI_FUNC("cryptoAesEncrypt", CryptoAesEncrypt),
        DECLARE_NAPI_FUNC("cryptoAesDecrypt", CryptoAesDecrypt),
        DECLARE_NAPI_FUNC("cryptoAesGcmEncrypt", CryptoAesGcmEncrypt),
        DECLARE_NAPI_FUNC("cryptoAesGcmDecrypt", CryptoAesGcmDecrypt),
        DECLARE_NAPI_FUNC("cryptoHashMd5", CryptoHashMd5),
        DECLARE_NAPI_FUNC("cryptoHashSha256", CryptoHashSha256),
        DECLARE_NAPI_FUNC("cryptoBase64Encode", CryptoBase64Encode),
        DECLARE_NAPI_FUNC("cryptoBase64Decode", CryptoBase64Decode),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module ngenxxModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = RegisterFuncs,
    .nm_modname = "ngenxx",
    .nm_priv = (reinterpret_cast<void *>(0)),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterNGenXXModule(void) { napi_module_register(&ngenxxModule); }