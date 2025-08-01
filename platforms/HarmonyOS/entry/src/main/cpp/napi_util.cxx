#include "napi_util.hxx"

#include <cstring>

Args::Args(napi_env env, napi_callback_info info) : env(env), info(info) {
    auto status = napi_get_cb_info(env, info, &this->c, nullptr, nullptr, nullptr);
    this->v = mallocX<napi_value>(this->c);
    status = napi_get_cb_info(env, info, &this->c, this->v, nullptr, nullptr);
}

Args::~Args() { freeX(this->v); }

size_t napiValueArrayLen(napi_env env, napi_value nv) {
    uint32_t len;
    auto status = napi_get_array_length(env, nv, &len);
    CHECK_NAPI_STATUS_RETURN_NUM(env, status, "napi_get_array_length() failed");
    return len;
}

const char *napiValue2chars(napi_env env, napi_value nv) {
    size_t len;
    auto status = napi_get_value_string_utf8(env, nv, nullptr, 0, &len);
    CHECK_NAPI_STATUS_RETURN_PTR(env, status, "napi_get_value_string_utf8() failed");
    auto cStr = mallocX<char>(len);
    status = napi_get_value_string_utf8(env, nv, cStr, len + 1, &len);
    CHECK_NAPI_STATUS_RETURN_PTR(env, status, "napi_get_value_string_utf8() failed");
    return cStr;
}

bool napiValue2bool(napi_env env, napi_value nv) {
    bool b = false;
    auto status = napi_get_value_bool(env, nv, &b);
    CHECK_NAPI_STATUS_RETURN_NUM(env, status, "napi_get_value_bool() failed");
    return b;
}

int napiValue2int(napi_env env, napi_value nv) {
    int i;
    auto status = napi_get_value_int32(env, nv, &i);
    CHECK_NAPI_STATUS_RETURN_NUM(env, status, "napi_get_value_int32() failed");
    return i;
}

long napiValue2long(napi_env env, napi_value nv) {
    long l;
    auto status = napi_get_value_int64(env, nv, &l);
    CHECK_NAPI_STATUS_RETURN_NUM(env, status, "napi_get_value_int64() failed");
    return l;
}

double napiValue2double(napi_env env, napi_value nv) {
    double d;
    auto status = napi_get_value_double(env, nv, &d);
    CHECK_NAPI_STATUS_RETURN_NUM(env, status, "napi_get_value_double() failed");
    return d;
}

const byte *napiValue2byteArray(napi_env env, napi_value nv, size_t len) {
    if (len == 0) {
        return nullptr;
    }
    auto byteArray = mallocX<byte>(len);
    for (decltype(len) i = 0; i < len; i++) {
        napi_value vByte;
        auto status = napi_get_element(env, nv, i, &vByte);
        CHECK_NAPI_STATUS_RETURN_PTR(env, status, "napi_get_element() failed");
        byteArray[i] = napiValue2int(env, vByte);
    }
    return byteArray;
}

const char **napiValue2charsArray(napi_env env, napi_value nv, size_t len) {
    if (len == 0) {
        return nullptr;
    }
    const auto charsArray = mallocX<const char *>(len);
    for (decltype(len) i = 0; i < len; i++) {
        napi_value vChars;
        auto status = napi_get_element(env, nv, i, &vChars);
        CHECK_NAPI_STATUS_RETURN_PTR(env, status, "napi_get_element() failed");
        charsArray[i] = napiValue2chars(env, vChars);
    }
    return charsArray;
}

napi_value chars2NapiValue(napi_env env, const char *c) {
    if (c == nullptr) {
        c = "";
    }
    napi_value v;
    auto status = napi_create_string_utf8(env, c, std::strlen(c), &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_string_utf8() failed");
    return v;
}

napi_value long2NapiValue(napi_env env, long l) {
    napi_value v;
    auto status = napi_create_int64(env, l, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_int64() failed");
    return v;
}

napi_value int2NapiValue(napi_env env, int i) {
    napi_value v;
    auto status = napi_create_int32(env, i, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_int32() failed");
    return v;
}

napi_value bool2NapiValue(napi_env env, bool b) {
    napi_value v;
    auto status = napi_get_boolean(env, b, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_boolean() failed");
    return v;
}

napi_value double2NapiValue(napi_env env, double d) {
    napi_value v;
    auto status = napi_create_double(env, d, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_double() failed");
    return v;
}

napi_value byteArray2NapiValue(napi_env env, const byte *byteArray, size_t len) {
    napi_value v;
    auto status = napi_create_array_with_length(env, len, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_array_with_length() failed");
    for (decltype(len) i = 0; i < len; i++) {
        status = napi_set_element(env, v, i, int2NapiValue(env, byteArray[i]));
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_set_element() failed");
    }
    return v;
}

napi_value charsArray2NapiValue(napi_env env, const char **charsArray, size_t len) {
    napi_value v;
    auto status = napi_create_array_with_length(env, len, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_array_with_length() failed");
    for (decltype(len) i = 0; i < len; i++) {
        status = napi_set_element(env, v, i, chars2NapiValue(env, charsArray[i]));
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_set_element() failed");
    }
    return v;
}