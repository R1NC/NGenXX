--[[
    Required [json.lua](https://gist.github.com/tylerneylon/59f4bcf316be525b30ab)
]]

NGenXX = {}

local InJsonVoid = ''

function NGenXX.version()
    return ngenxx_get_version()
end

function NGenXX.root()
    return ngenxx_root_path()
end

NGenXX.Log = {}

NGenXX.Log.Level = {
    Debug = 3,
    Info = 4,
    Warn = 5,
    Error = 6,
    Fatal = 7,
    None = 8
}

function NGenXX.Log.print(level, content)
    local inJson = JSON.stringify({
        ["level"] = level,
        ["content"] = content
    })
    ngenxx_log_print(inJson)
end

NGenXX.Device = {}

NGenXX.Device.Platform = {
    Unknown = 0,
    Android = 1,
    ApplePhone = 2,
    ApplePad = 3,
    AppleMac = 4,
    AppleWatch = 5,
    ApplocalV = 6,
    HarmonyOS = 7,
    Windows = 8,
    Linux = 9,
    Web = 10
}

NGenXX.Device.CpuArch = {
    Unknown = 0,
    X86 = 1,
    X86_64 = 2,
    IA64 = 3,
    ARM = 4,
    ARM_6 = 5
}

function NGenXX.Device.platform()
    return ngenxx_device_type(InJsonVoid)
end

function NGenXX.Device.name()
    return ngenxx_device_name(InJsonVoid)
end

function NGenXX.Device.manufacturer()
    return ngenxx_device_manufacturer(InJsonVoid)
end

function NGenXX.Device.osVersion()
    return ngenxx_device_os_version(InJsonVoid)
end

function NGenXX.Device.cpuArch()
    return ngenxx_device_cpu_arch(InJsonVoid)
end

NGenXX.Net = {}
NGenXX.Net.Http = {}

NGenXX.Net.Http.Method = {
    Get = 0,
    Post = 1,
    Put = 2
}

function NGenXX.Net.Http.request(url, method, param_map, header_map, raw_body_bytes, timeout)
    param_map = param_map or {}
    local paramStr = ""
    for k, v in pairs(param_map) do
        if string.len(paramStr) == 0 then
            paramStr = paramStr .. '?'
        else
            paramStr = paramStr .. '&'
        end
        paramStr = paramStr .. k .. '=' .. v
    end

    local headerArray = {}
    header_map = header_map or {}
    for k, v in pairs(header_map) do
        table.insert(headerArray, k .. '=' .. v)
    end

    timeout = timeout or (15 * 1000)

    local inDict = {
        ["url"] = url,
        ["method"] = method,
        ["params"] = paramStr,
        ["header_v"] = headerArray,
        ["timeout"] = timeout
    }

    if (raw_body_bytes ~= nil and #raw_body_bytes > 0) then
        inDict["rawBodyBytes"] = raw_body_bytes
    end

    local inJson = JSON.stringify(inDict)
    return ngenxx_net_http_request(inJson)
end

function NGenXX.Net.Http.download(url, file, timeout)
    timeout = timeout or (15 * 1000)
    local inJson = JSON.stringify({
        ["url"] = url,
        ["file"] = file,
        ["timeout"] = timeout
    })
    return ngenxx_net_http_download(inJson)
end

NGenXX.Coding = {}

NGenXX.Coding.Case = {}

function NGenXX.Coding.Case.upper(str)
    local inJson = JSON.stringify({
        ["str"] = str
    })
    return ngenxx_coding_case_upper(inJson)
end

function NGenXX.Coding.Case.lower(str)
    local inJson = JSON.stringify({
        ["str"] = str
    })
    return ngenxx_coding_case_lower(inJson)
end

NGenXX.Coding.Hex = {}

function NGenXX.Coding.Hex.bytes2Str(bytes)
    local inJson = JSON.stringify({
        ["inBytes"] = bytes
    })
    return ngenxx_coding_hex_bytes2str(inJson)
end

function NGenXX.Coding.Hex.str2Bytes(str)
    local inJson = JSON.stringify({
        ["str"] = str
    })
    local outJson = ngenxx_coding_hex_str2bytes(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Coding.bytes2Str(bytes)
    local inJson = JSON.stringify({
        ["inBytes"] = bytes
    })
    return ngenxx_coding_bytes2str(inJson)
end

function NGenXX.Coding.str2Bytes(str)
    local inJson = JSON.stringify({
        ["str"] = str
    })
    local outJson = ngenxx_coding_str2bytes(inJson)
    return JSON.parse(outJson)
end

NGenXX.Crypto = {}

function NGenXX.Crypto.rand(len)
    local inJson = JSON.stringify({
        ["len"] = len
    })
    local outJson = ngenxx_crypto_rand(inJson)
    return JSON.parse(outJson)
end

NGenXX.Crypto.Aes = {}

function NGenXX.Crypto.Aes.encrypt(inBytes, keyBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes
    })
    local outJson = ngenxx_crypto_aes_encrypt(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Crypto.Aes.decrypt(inBytes, keyBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes
    })
    local outJson = ngenxx_crypto_aes_decrypt(inJson)
    return JSON.parse(outJson)
end

NGenXX.Crypto.Aes.Gcm = {}

function NGenXX.Crypto.Aes.Gcm.encrypt(inBytes, keyBytes, ivBytes, tagBits, aadBytes)
    local inDict = {
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes,
        ["initVectorBytes"] = ivBytes,
        ["tagBits"] = tagBits
    }
    if (aadBytes ~= nil and #aadBytes > 0) then
        inDict["aadBytes"] = aadBytes
    end
    local inJson = JSON.stringify(inDict)
    local outJson = ngenxx_crypto_aes_gcm_encrypt(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Crypto.Aes.Gcm.decrypt(inBytes, keyBytes, ivBytes, tagBits, aadBytes)
    local inDict = {
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes,
        ["initVectorBytes"] = ivBytes,
        ["tagBits"] = tagBits
    }
    if (aadBytes ~= nil and #aadBytes > 0) then
        inDict["aadBytes"] = aadBytes
    end
    local inJson = JSON.stringify(inDict)
    local outJson = ngenxx_crypto_aes_gcm_decrypt(inJson)
    return JSON.parse(outJson)
end

NGenXX.Crypto.Rsa = {}

function NGenXX.Crypto.Rsa.genKey(base64, isPublic)
    local inJson = JSON.stringify({
        ["base64"] = base64,
        ["isPublic"] = isPublic
    })
    local outJson = ngenxx_crypto_rsa_gen_key(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Crypto.Rsa.encrypt(inBytes, keyBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes
    })
    local outJson = ngenxx_crypto_rsa_encrypt(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Crypto.Rsa.decrypt(inBytes, keyBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes
    })
    local outJson = ngenxx_crypto_rsa_decrypt(inJson)
    return JSON.parse(outJson)
end

NGenXX.Crypto.Hash = {}

function NGenXX.Crypto.Hash.md5(inBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes
    })
    local outJson = ngenxx_crypto_hash_md5(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Crypto.Hash.sha256(inBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes
    })
    local outJson = ngenxx_crypto_hash_sha256(inJson)
    return JSON.parse(outJson)
end

NGenXX.Crypto.Base64 = {}

function NGenXX.Crypto.Base64.encode(inBytes, noNewLines)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["noNewLines"] = noNewLines and 1 or 0
    })
    local outJson = ngenxx_crypto_base64_encode(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Crypto.Base64.decode(inBytes, noNewLines)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["noNewLines"] = noNewLines and 1 or 0
    })
    local outJson = ngenxx_crypto_base64_decode(inJson)
    return JSON.parse(outJson)
end

NGenXX.Store = {}

NGenXX.Store.SQLite = {}

function NGenXX.Store.SQLite.open(id)
    local inJson = JSON.stringify({
        ["_id"] = id
    })
    return ngenxx_store_sqlite_open(inJson)
end

function NGenXX.Store.SQLite.execute(conn, sql)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["sql"] = sql
    })
    return ngenxx_store_sqlite_execute(inJson)
end

NGenXX.Store.SQLite.Query = {}

function NGenXX.Store.SQLite.Query.create(conn, sql)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["sql"] = sql
    })
    return ngenxx_store_sqlite_query_do(inJson)
end

function NGenXX.Store.SQLite.Query.readRow(query_result)
    local inJson = JSON.stringify({
        ["query_result"] = query_result
    })
    return ngenxx_store_sqlite_query_read_row(inJson)
end

function NGenXX.Store.SQLite.Query.readColumnText(query_result, column)
    local inJson = JSON.stringify({
        ["query_result"] = query_result,
        ["column"] = column
    })
    return ngenxx_store_sqlite_query_read_column_text(inJson)
end

function NGenXX.Store.SQLite.Query.readColumnInteger(query_result, column)
    local inJson = JSON.stringify({
        ["query_result"] = query_result,
        ["column"] = column
    })
    return ngenxx_store_sqlite_query_read_column_integer(inJson)
end

function NGenXX.Store.SQLite.Query.readColumnFloat(query_result, column)
    local inJson = JSON.stringify({
        ["query_result"] = query_result,
        ["column"] = column
    })
    return ngenxx_store_sqlite_query_read_column_float(inJson)
end

function NGenXX.Store.SQLite.Query.drop(query_result)
    local inJson = JSON.stringify({
        ["query_result"] = query_result
    })
    ngenxx_store_sqlite_query_drop(inJson)
end

function NGenXX.Store.SQLite.close(conn)
    local inJson = JSON.stringify({
        ["conn"] = conn
    })
    ngenxx_store_sqlite_close(inJson)
end

NGenXX.Store.KV = {}

function NGenXX.Store.KV.open(id)
    local inJson = JSON.stringify({
        ["_id"] = id
    })
    return ngenxx_store_kv_open(inJson)
end

function NGenXX.Store.KV.readString(conn, k)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k
    })
    return ngenxx_store_kv_read_string(inJson)
end

function NGenXX.Store.KV.writeString(conn, k, s)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k,
        ["v"] = s
    })
    return ngenxx_store_kv_write_string(inJson)
end

function NGenXX.Store.KV.readInteger(conn, k)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k
    })
    return ngenxx_store_kv_read_integer(inJson)
end

function NGenXX.Store.KV.writeInteger(conn, k, i)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k,
        ["v"] = i
    })
    return ngenxx_store_kv_write_integer(inJson)
end

function NGenXX.Store.KV.readFloat(conn, k)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k
    })
    return ngenxx_store_kv_read_float(inJson)
end

function NGenXX.Store.KV.writeFloat(conn, k, f)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k,
        ["v"] = f
    })
    return ngenxx_store_kv_write_float(inJson)
end

function NGenXX.Store.KV.allKeys(conn)
    local inJson = JSON.stringify({
        ["conn"] = conn
    })
    local outJson = ngenxx_store_kv_all_keys(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Store.KV.contains(conn, k)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k
    })
    return ngenxx_store_kv_contains(inJson)
end

function NGenXX.Store.KV.remove(conn, k)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k
    })
    ngenxx_store_kv_remove(inJson)
end

function NGenXX.Store.KV.clear(conn)
    local inJson = JSON.stringify({
        ["conn"] = conn
    })
    ngenxx_store_kv_clear(inJson)
end

function NGenXX.Store.KV.close(conn)
    local inJson = JSON.stringify({
        ["conn"] = conn
    })
    ngenxx_store_kv_close(inJson)
end

NGenXX.Z = {}

NGenXX.Z.Format = {
    ZLib = 0,
    GZip = 1,
    Raw = 2
}

NGenXX.Z.ZipMode = {
    Default = -1,
    PreferSpeed = 1,
    PreferSize = 9
}

NGenXX.Z.DefaultBufferSize = 16 * 1024

function NGenXX.Z.zipBytes(inBytes, format, mode)
    format = format or NGenXX.Z.Format.ZLib
    mode = mode or NGenXX.Z.ZipMode.Default
    local inJson = JSON.stringify({
        ["mode"] = mode,
        ["bufferSize"] = NGenXX.Z.DefaultBufferSize,
        ["format"] = format,
        ["inBytes"] = inBytes
    })
    local outJson = ngenxx_z_bytes_unzip(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Z.unZipBytes(inBytes, format)
    format = format or NGenXX.Z.Format.ZLib
    local inJson = JSON.stringify({
        ["bufferSize"] = NGenXX.Z.DefaultBufferSize,
        ["format"] = format,
        ["inBytes"] = inBytes
    })
    local outJson = ngenxx_z_bytes_unzip(inJson)
    return JSON.parse(outJson)
end

NGenXX.Z._ = {}

function NGenXX.Z._.zipInit(mode, bufferSize, format)
    local inJson = JSON.stringify({
        ["mode"] = mode,
        ["bufferSize"] = bufferSize,
        ["format"] = format
    })
    return ngenxx_z_zip_init(inJson)
end

function NGenXX.Z._.zipInput(zip, bytes, finish)
    local inJson = JSON.stringify({
        ["zip"] = zip,
        ["inBytes"] = bytes,
        ["inFinish"] = finish and 1 or 0
    })
    return ngenxx_z_zip_input(inJson)
end

function NGenXX.Z._.zipProcessDo(zip)
    local inJson = JSON.stringify({
        ["zip"] = zip
    })
    local outJson = ngenxx_z_zip_process_do(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Z._.zipProcessFinished(zip)
    local inJson = JSON.stringify({
        ["zip"] = zip
    })
    return ngenxx_z_zip_process_finished(inJson)
end

function NGenXX.Z._.zipRelease(zip)
    local inJson = JSON.stringify({
        ["zip"] = zip
    })
    ngenxx_z_zip_release(inJson)
end

function NGenXX.Z._.unZipInit(bufferSize, format)
    local inJson = JSON.stringify({
        ["bufferSize"] = bufferSize,
        ["format"] = format
    })
    return ngenxx_z_unzip_init(inJson)
end

function NGenXX.Z._.unZipInput(unzip, bytes, finish)
    local inJson = JSON.stringify({
        ["unzip"] = unzip,
        ["inBytes"] = bytes,
        ["inFinish"] = finish and 1 or 0
    })
    return ngenxx_z_unzip_input(inJson)
end

function NGenXX.Z._.unZipProcessDo(unzip)
    local inJson = JSON.stringify({
        ["unzip"] = unzip
    })
    local outJson = ngenxx_z_unzip_process_do(inJson)
    return JSON.parse(outJson)
end

function NGenXX.Z._.unZipProcessFinished(unzip)
    local inJson = JSON.stringify({
        ["unzip"] = unzip
    })
    return ngenxx_z_unzip_process_finished(inJson)
end

function NGenXX.Z._.unZipRelease(unzip)
    local inJson = JSON.stringify({
        ["unzip"] = unzip
    })
    ngenxx_z_unzip_release(inJson)
end

function NGenXX.Z._.stream(bufferSize, readFunc, writeFunc, flushFunc, z, inputFunc, processDoFunc, processFinishedFunc)
    local inputFinished = false
    local processFinished = false
    repeat
        local inBytes = readFunc()
        inputFinished = #inBytes < bufferSize
        NGenXX.Log.print(NGenXX.Log.Level.Debug, 'z <- len:' .. #inBytes .. ' finished:' .. inputFinished)

        local inputRet = inputFunc(z, inBytes, inputFinished)
        if (inputRet <= 0) then
            NGenXX.Log.print(NGenXX.Log.Level.Error, 'z input failed!')
            return false
        end

        processFinished = false
        repeat
            local outBytes = processDoFunc(z)
            if (#outBytes == 0) then
                NGenXX.Log.print(NGenXX.Log.Level.Error, 'z process failed!')
                return false
            end
            processFinished = processFinishedFunc(z)
            NGenXX.Log.print(NGenXX.Log.Level.Debug, 'z -> len:' .. #outBytes .. ' finished:' .. processFinished)

            writeFunc(outBytes)
        until (~processFinished)
    until (~inputFinished)

    flushFunc()
    return true
end

function NGenXX.Z._.zipStream(readFunc, writeFunc, flushFunc, mode, bufferSize, format)
    local zip = NGenXX.Z._.zipInit(mode, bufferSize, format)

    local res = NGenXX.Z._.Stream(bufferSize, readFunc, writeFunc, flushFunc, zip,
        function(z, buffer, inputFinished)
            return NGenXX.Z._.zipInput(z, buffer, inputFinished)
        end,
        function(z)
            return NGenXX.Z._.zipProcessDo(z)
        end,
        function(z)
            return NGenXX.Z._.zipProcessFinished(z)
        end
    )

    NGenXX.Z._.zipRelease(zip)
    return res
end

function NGenXX.Z._.unZipStream(readFunc, writeFunc, flushFunc, bufferSize, format)
    local unzip = NGenXX.Z._.unZipInit(bufferSize, format)

    local res = NGenXX.Z._.Stream(bufferSize, readFunc, writeFunc, flushFunc, unzip,
        function(z, buffer, inputFinished)
            return NGenXX.Z._.unZipInput(z, buffer, inputFinished)
        end,
        function(z)
            return NGenXX.Z._.unZipProcessDo(z)
        end,
        function(z)
            return NGenXX.Z._.unZipProcessFinished(z)
        end
    )

    NGenXX.Z._.unZipRelease(unzip)
    return res
end

function NGenXX.Z.zipFile(inFilePath, outFilePath, mode, bufferSize, format)
    local inF = io.open(inFilePath, 'r')
    local outF = io.open(outFilePath, 'w')
    if (inF == nil or outF == nil) then
        return false
    end

    local res = NGenXX.Z._.zipStream(
        function()
            return inF:read(bufferSize)
        end,
        function(bytes)
            outF:write(bytes)
        end,
        function()
            outF:flush()
        end,
        mode,
        bufferSize,
        format
    )

    outF:close()
    inF:close()
    return res
end

function NGenXX.Z.unZipFile(inFilePath, outFilePath, bufferSize, format)
    local inF = io.open(inFilePath, 'r')
    local outF = io.open(outFilePath, 'w')
    if (inF == nil or outF == nil) then
        return false
    end

    local res = NGenXX.Z._.unZipStream(
        function()
            return inF:read(bufferSize)
        end,
        function(bytes)
            outF:write(bytes)
        end,
        function()
            outF:flush()
        end,
        bufferSize,
        format
    )

    outF:close()
    inF:close()
    return res
end
