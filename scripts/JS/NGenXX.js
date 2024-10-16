function _Array2Str(array) {
    var s = '';
    array = array || [];
    array.forEach((item, index, arr) => {
        s += `${item}`;
    });
    return s;
}

function _Map2UrlStr(map) {
    map = map || new Map();
    var s = '';
    map.forEach((v, k) => {
        s += s.length === 0 ? '?' : '&';
        s += `${k}=${v}`;
    });
    return s;
}

function _Map2StrArray(map) {
    var arr = [];
    map = map || new Map();
    map.forEach((v, k) => {
        arr.push(`${k}=${v}`);
    });
    return arr;
}

function _json2Array(json) {
    return JSON.parse(json || '[]');
}

function _printArray(arr) {
    arr.map((x) => {
        NGenXXLogPrint(NGenXXLogLevel.Debug, `${x}`);
    });
}

// Log

const NGenXXLogLevel = Object.freeze({
    Info: 0,
    Debug: 1,
    Warn: 2,
    Error: 3,
    None: 4
});

function NGenXXLogPrint(level, content) {
    content = content || '';
    let inJson = JSON.stringify({
        "level": level,
        "content": content
    });
    ngenxx_log_printJ(inJson);
}

// DeviceInfo

const NGenXXDeviceType = Object.freeze({
    Unknown: 0,
    Android: 1,
    ApplePhone: 2,
    ApplePad: 3,
    AppleMac: 4,
    AppleWatch: 5,
    AppleTV: 6,
    HarmonyOS: 7,
    Windows: 8,
    Linux: 9,
    Web: 10
});

const NGenXXDeviceCpuArch = Object.freeze({
    Unknown: 0,
    X86: 1,
    X86_64: 2,
    IA64: 3,
    ARM: 4,
    ARM_64: 5
});

function NGenXXDeviceType() {
    return ngenxx_sdk_engine_device_typeJ();
}

function NGenXXDeviceName() {
    return ngenxx_sdk_engine_device_nameJ();
}

function NGenXXDeviceManufacturer() {
    return ngenxx_sdk_engine_device_manufacturerJ();
}

function NGenXXDeviceOsVersion() {
    return ngenxx_sdk_engine_device_os_versionJ();
}

function NGenXXDeviceCpuArch() {
    return ngenxx_sdk_engine_device_cpu_archJ();
}

// Net.Http

const NGenXXHttpMethod = Object.freeze({
    Get: 0,
    Post: 1,
    Put: 2
});

function NGenXXNetHttpRequest(url, paramMap, method, headerMap, formFieldNameArray, formFieldMimeArray, formFieldDataArray, timeout) {
    paramStr = _Map2UrlStr(paramMap);
    headerArray = _Map2StrArray(headerMap);
    formFieldNameArray = formFieldNameArray || [];
    formFieldMimeArray = formFieldMimeArray || [];
    formFieldDataArray = formFieldDataArray || [];
    
    let inJson = JSON.stringify({
        "url": url,
        "method": method,
        "params": paramStr,
        "header_v": headerArray,
        "header_c": headerArray.length,
        "form_field_name_v": formFieldNameArray,
        "form_field_mime_v": formFieldMimeArray,
        "form_field_data_v": formFieldDataArray,
        "form_field_count": formFieldNameArray.length,
        "timeout": timeout
    });
    
    return ngenxx_net_http_requestJ(inJson);
}

// Store.SQLite

function NGenXXStoreSQLiteOpen(_id) {
    let inJson = JSON.stringify({
        "_id": _id
    });
    return ngenxx_store_sqlite_openJ(inJson);
}

function NGenXXStoreSQLiteExecute(conn, sql) {
    let inJson = JSON.stringify({
        "conn": conn,
        "sql": sql
    });
    return ngenxx_store_sqlite_executeJ(inJson);
}

function NGenXXStoreSQLiteQueryDo(conn, sql) {
    let inJson = JSON.stringify({
        "conn": conn,
        "sql": sql
    });
    return ngenxx_store_sqlite_query_doJ(inJson);
}

function NGenXXStoreSQLiteQueryReadRow(query_result) {
    let inJson = JSON.stringify({
        "query_result": query_result
    });
    return ngenxx_store_sqlite_query_read_rowJ(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnText(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return ngenxx_store_sqlite_query_read_column_textJ(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnInteger(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return ngenxx_store_sqlite_query_read_column_integerJ(inJson);
}

function NGenXXStoreSQLiteQueryReadColumnFloat(query_result, column) {
    let inJson = JSON.stringify({
        "query_result": query_result,
        "column": column
    });
    return ngenxx_store_sqlite_query_read_column_floatJ(inJson);
}

function NGenXXStoreSQLiteQueryDrop(query_result) {
    let inJson = JSON.stringify({
        "query_result": query_result
    });
    ngenxx_store_sqlite_query_dropJ(inJson);
}

function NGenXXStoreSQLiteClose(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    ngenxx_store_sqlite_closeJ(inJson);
}

// Store.KV

function NGenXXStoreKVOpen(_id) {
    let inJson = JSON.stringify({
        "_id": _id
    });
    return ngenxx_store_kv_openJ(inJson);
}

function NGenXXStoreKVReadString(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_read_stringJ(inJson);
}

function NGenXXStoreKVWriteString(conn, k, s) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": s
    });
    return ngenxx_store_kv_write_stringJ(inJson);
}

function NGenXXStoreKVReadInteger(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_read_integerJ(inJson);
}

function NGenXXStoreKVWriteInteger(conn, k, i) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": i
    });
    return ngenxx_store_kv_write_integerJ(inJson);
}

function NGenXXStoreKVReadFloat(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_read_floatJ(inJson);
}

function NGenXXStoreKVWriteFloat(conn, k, f) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k,
        "v": f
    });
    return ngenxx_store_kv_write_floatJ(inJson);
}

function NGenXXStoreKVContains(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    return ngenxx_store_kv_containsJ(inJson);
}

function NGenXXStoreKVRemove(conn, k) {
    let inJson = JSON.stringify({
        "conn": conn,
        "k": k
    });
    ngenxx_store_kv_removeJ(inJson);
}

function NGenXXStoreKVClear(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    ngenxx_store_kv_clearJ(inJson);
}

function NGenXXStoreKVClose(conn) {
    let inJson = JSON.stringify({
        "conn": conn
    });
    ngenxx_store_kv_closeJ(inJson);
}

// Coding

function NGenXXStr2Bytes(str) {
    return Array.from(str, char => char.charCodeAt(0))
}

function NGenXXBytes2Str(bytes) {
    return bytes.map((b) => {
        return b > 0 ? String.fromCharCode(b) : '';
    }).join('');
}

function NGenXXCodingHexBytes2Str(bytes) {
    bytes = bytes || [];
    let inJson = JSON.stringify({
        "inBytes": bytes,
        "inLen": bytes.length
    });
    return ngenxx_coding_hex_bytes2strJ(inJson);
}

function NGenXXCodingHexStr2Bytes(hexStr) {
    hexStr = hexStr || '';
    let inJson = JSON.stringify({
        "str": hexStr
    });
    let outJson = ngenxx_coding_hex_str2bytesJ(inJson);
    return _json2Array(outJson);
}

// Crypto

function NGenXXCryptoRand(len) {
    let inJson = JSON.stringify({
        "len": len
    });
    let outJson = ngenxx_crypto_randJ(inJson);
    return _json2Array(outJson);
}

// Crypto.AES

function NGenXXCryptoAesEncrypt(inBytes, keyBytes) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length,
        "keyBytes": keyBytes,
        "keyLen": keyBytes.length
    });
    let outJson = ngenxx_crypto_aes_encryptJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesDecrypt(inBytes, keyBytes) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length,
        "keyBytes": keyBytes,
        "keyLen": keyBytes.length
    });
    let outJson = ngenxx_crypto_aes_decryptJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, aadBytes, tagBits) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    ivBytes = ivBytes || [];
    aadBytes = aadBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length,
        "keyBytes": keyBytes,
        "keyLen": keyBytes.length,
        "initVectorBytes": ivBytes,
        "initVectorLen": ivBytes.length,
        "aadBytes": aadBytes,
        "aadLen": aadBytes.length,
        "tagBits": tagBits
    });
    let outJson = ngenxx_crypto_aes_gcm_encryptJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoAesGcmDecrypt(inBytes, keyBytes, ivBytes, aadBytes, tagBits) {
    inBytes = inBytes || [];
    keyBytes = keyBytes || [];
    ivBytes = ivBytes || [];
    aadBytes = aadBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length,
        "keyBytes": keyBytes,
        "keyLen": keyBytes.length,
        "initVectorBytes": ivBytes,
        "initVectorLen": ivBytes.length,
        "aadBytes": aadBytes,
        "aadLen": aadBytes.length,
        "tagBits": tagBits
    });
    let outJson = ngenxx_crypto_aes_gcm_decryptJ(inJson);
    return _json2Array(outJson);
}

// Crypto.Hash

function NGenXXCryptoHashMD5(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length
    });
    let outJson = ngenxx_crypto_hash_md5J(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoHashSHA256(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length
    });
    let outJson = ngenxx_crypto_hash_sha256J(inJson);
    return _json2Array(outJson);
}

// Crypto.Base64

function NGenXXCryptoBase64Encode(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length
    });
    let outJson = ngenxx_crypto_base64_encodeJ(inJson);
    return _json2Array(outJson);
}

function NGenXXCryptoBase64Decode(inBytes) {
    inBytes = inBytes || [];
    let inJson = JSON.stringify({
        "inBytes": inBytes,
        "inLen": inBytes.length
    });
    let outJson = ngenxx_crypto_base64_decodeJ(inJson);
    return _json2Array(outJson);
}
