function jTestNetHttpReq() {
    let url = "https://rinc.xyz";
    let params = "p0=1&p1=2";
    let method = 0;
    let headerV = ["Cache-Control: no-cache"];
    let timeout = 55555;
    return NGenXXNetHttpRequest(url, params, method, headerV, null, null, null, timeout);
}

function jTestStoreKV() {
    let kvId = 'test_kv';
    let conn = NGenXXStoreKVOpen(kvId);
    if (conn > 0) {
        let kS = "kS";
        if (NGenXXStoreKVContains(conn, kS)) {
            NGenXXStoreKVRemove(conn, kS);
        }
        NGenXXStoreKVWriteString(conn, kS, "NGenXX");
        let vS = NGenXXStoreKVReadString(conn, kS);
        NGenXXLogPrint(1, `KV read ${kS}: ${vS}`);
        
        let kI = "kI";
        if (NGenXXStoreKVContains(conn, kI)) {
            NGenXXStoreKVRemove(conn, kI);
        }
        NGenXXStoreKVWriteInteger(conn, kI, 12345678909666666);
        let vI = NGenXXStoreKVReadInteger(conn, kI);
        NGenXXLogPrint(1, `KV read ${kI}: ${vI}`);
        
        let kF = "kF";
        if (NGenXXStoreKVContains(conn, kF)) {
            NGenXXStoreKVRemove(conn, kF);
        }
        NGenXXStoreKVWriteFloat(conn, kF, -0.12345678987654321);
        let vF = NGenXXStoreKVReadFloat(conn, kF);
        NGenXXLogPrint(1, `KV read ${kF}: ${vF}`);
        
        NGenXXStoreKVClose(conn);
    } else {
        NGenXXLogPrint(1, `KV open failed!!!`);
    }
}

let sqlPrepareData = `
DROP TABLE IF EXISTS TestTable;
CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, s TEXT, i INTEGER, f FLOAT);
INSERT OR IGNORE INTO TestTable (s, i, f) VALUES
('iOS', 1, 0.111111111),
('Android', 2, 0.2222222222),
('HarmonyOS', 3, 0.3333333333);`;

let sqlQuery = `SELECT * FROM TestTable;`;

function jTestStoreSQLite() {
    let dbId = 'test_db';
    let conn = NGenXXStoreSQLiteOpen(dbId);
    if (conn > 0) {
        if (NGenXXStoreSQLiteExecute(conn, sqlPrepareData)) {
            let queryResult = NGenXXStoreSQLiteQueryDo(conn, sqlQuery);
            if (queryResult > 0) {
                while (NGenXXStoreSQLiteQueryReadRow(queryResult)) {
                    let s = NGenXXStoreSQLiteQueryReadColumnText(queryResult, 's');
                    let i = NGenXXStoreSQLiteQueryReadColumnInteger(queryResult, 'i');
                    let f = NGenXXStoreSQLiteQueryReadColumnFloat(queryResult, 'f');
                    NGenXXLogPrint(1, `s:${s} i:${i} f:${f}`);
                }
                
                NGenXXStoreSQLiteQueryDrop(queryResult);
            } else {
                NGenXXLogPrint(1, `SQLite query failed!!!`);
            }
        } else {
            NGenXXLogPrint(1, `SQLite execute failed!!!`);
        }
        NGenXXStoreSQLiteClose(conn);
    } else {
        NGenXXLogPrint(1, `SQLite open failed!!!`);
    }
}

function jTestCryptoBase64(s) {
    let inBytes = NGenXXStr2Bytes(s);
    let enBytes = NGenXXCryptoBase64Encode(inBytes);
    let enS = NGenXXBytes2Str(enBytes);
    NGenXXLogPrint(1, `Base64 encoded: ${enS}`);

    let deBytes = NGenXXCryptoBase64Decode(enBytes);
    let deS = NGenXXBytes2Str(deBytes);
    NGenXXLogPrint(1, `Base64 decoded: ${deS}`);
}

function jTestCryptoHash(s) {
    let inBytes = NGenXXStr2Bytes(s);

    let md5Bytes = NGenXXCryptoHashMD5(inBytes);
    let md5S = NGenXXCodingHexBytes2Str(md5Bytes);
    NGenXXLogPrint(1, `MD5: ${md5S}`);

    let sha256Bytes = NGenXXCryptoHashSHA256(s);
    let sha256S = NGenXXCodingHexBytes2Str(sha256Bytes);
    NGenXXLogPrint(1, `SHA256: ${sha256S}`);
}

let AES_KEY = "QWERTYUIOPASDFGH";

function jTestCryptoAes(s) {
    let inBytes = NGenXXStr2Bytes(s);
    let keyBytes = NGenXXStr2Bytes(AES_KEY);

    let enBytes = NGenXXCryptoAesEncrypt(inBytes, keyBytes);
    let enS = NGenXXCodingHexBytes2Str(enBytes);
    NGenXXLogPrint(1, `AES encoded: ${enS}`);

    let deBytes = NGenXXCryptoAesDecrypt(enBytes, keyBytes);
    let deS = NGenXXBytes2Str(deBytes);
    NGenXXLogPrint(1, `AES decoded: ${deS}`);
}

function jTestCryptoAesGcm(s) {
    let inBytes = NGenXXStr2Bytes(s);
    let keyBytes = NGenXXStr2Bytes(AES_KEY);
    let ivBytes = NGenXXCryptoRand(12);
    let aadBytes = null;
    let tagBits = 96;

    let enBytes = NGenXXCryptoAesGcmEncrypt(inBytes, keyBytes, ivBytes, aadBytes, tagBits);
    let enS = NGenXXCodingHexBytes2Str(enBytes);
    NGenXXLogPrint(1, `AES-GCM encoded: ${enS}`);

    let deBytes = NGenXXCryptoAesGcmDecrypt(enBytes, keyBytes, ivBytes, aadBytes, tagBits);
    let deS = NGenXXBytes2Str(deBytes);
    NGenXXLogPrint(1, `AES-GCM decoded: ${deS}`);
}
