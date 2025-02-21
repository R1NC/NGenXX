#ifndef NGENXX_SRC_JS_BRIDGE_HXX_
#define NGENXX_SRC_JS_BRIDGE_HXX_

#if defined(__cplusplus)

#include <cstdint>
#include <functional>

#include <quickjs-libc.h>

#include <NGenXXTypes.hxx>

#define JS_FUNC_PARAMS                                                         \
  JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv

#define JS_FUNC_READ_JSON JS_ToCString(ctx, argv[0])

#define DEF_JS_FUNC_VOID(fJ, fS)                                               \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    auto json = JS_FUNC_READ_JSON;                                             \
    fS(json);                                                                  \
    JS_FreeCString(ctx, json);                                                 \
    return JS_UNDEFINED;                                                       \
  }

#define DEF_JS_FUNC_STRING(fJ, fS)                                             \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    auto json = JS_FUNC_READ_JSON;                                             \
    const std::string res = fS(json);                                          \
    JS_FreeCString(ctx, json);                                                 \
    return JS_NewString(ctx, res.c_str());                                     \
  }

#define DEF_JS_FUNC_BOOL(fJ, fS)                                               \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    auto json = JS_FUNC_READ_JSON;                                             \
    auto res = fS(json);                                                       \
    JS_FreeCString(ctx, json);                                                 \
    return JS_NewBool(ctx, res);                                               \
  }

#define DEF_JS_FUNC_INT32(fJ, fS)                                              \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    auto json = JS_FUNC_READ_JSON;                                             \
    auto res = fS(json);                                                       \
    JS_FreeCString(ctx, json);                                                 \
    return JS_NewInt32(ctx, res);                                              \
  }

#define DEF_JS_FUNC_INT64(fJ, fS)                                              \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    auto json = JS_FUNC_READ_JSON;                                             \
    auto res = fS(json);                                                       \
    JS_FreeCString(ctx, json);                                                 \
    return JS_NewInt64(ctx, res);                                              \
  }

#define DEF_JS_FUNC_FLOAT(fJ, fS)                                              \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    auto json = JS_FUNC_READ_JSON;                                             \
    auto res = fS(json);                                                       \
    JS_FreeCString(ctx, json);                                                 \
    return JS_NewFloat64(ctx, res);                                            \
  }

#define DEF_JS_FUNC_CHECK_BRIDGE(bridge)                                       \
  {                                                                            \
    if (bridge == nullptr) [[unlikely]] {                                      \
      return JS_UNDEFINED;                                                     \
    }                                                                          \
  }

#define DEF_JS_FUNC_VOID_ASYNC(bridge, fJ, fS)                                 \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \             
    DEF_JS_FUNC_CHECK_BRIDGE(bridge);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseVoid(                                             \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

#define DEF_JS_FUNC_BOOL_ASYNC(bridge, fJ, fS)                                 \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    DEF_JS_FUNC_CHECK_BRIDGE(bridge);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseBool(                                             \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

#define DEF_JS_FUNC_INT32_ASYNC(bridge, fJ, fS)                                \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    DEF_JS_FUNC_CHECK_BRIDGE(bridge);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseInt32(                                            \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

#define DEF_JS_FUNC_INT64_ASYNC(bridge, fJ, fS)                                \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \      
    DEF_JS_FUNC_CHECK_BRIDGE(bridge);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseInt64(                                            \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

#define DEF_JS_FUNC_FLOAT_ASYNC(bridge, fJ, fS)                                \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    DEF_JS_FUNC_CHECK_BRIDGE(bridge);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseFloat(                                            \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

#define DEF_JS_FUNC_STRING_ASYNC(bridge, fJ, fS)                               \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    DEF_JS_FUNC_CHECK_BRIDGE(bridge);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseString(                                           \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

namespace NGenXX
{
    class JsBridge
    {
    public:
        /**
         * Create JS VM
         */
        JsBridge();
        JsBridge(const JsBridge &) = delete;
        JsBridge &operator=(const JsBridge &) = delete;
        JsBridge(JsBridge &&) = delete;
        JsBridge &operator=(JsBridge &&) = delete;

        /**
         * @brief Export C func for JS
         * @param funcJ func name
         * @param funcC func implemention
         * @return success or not
         */
        bool bindFunc(const std::string &funcJ, JSCFunction *funcC);

        /**
         * @brief Load JS file
         * @param file JS file path
         * @return success or not
         */
        [[nodiscard]] bool loadFile(const std::string &file, const bool isModule);

        /**
         * @brief Load JS script
         * @param script JS script
         * @param name JS file name
         * @return success or not
         */
        [[nodiscard]] bool loadScript(const std::string &script, const std::string &name, const bool isModule);

        /**
         * @brief Load JS ByteCode
         * @param bytes JS ByteCode
         */
        [[nodiscard]] bool loadBinary(const Bytes &bytes, const bool isModule);

        /**
         * @brief call JS func
         * @param func func name
         * @param params parameters(json)
         * @param await Whether wait for the promise result or not
         */
        std::string callFunc(const std::string &func, const std::string &params, const bool await);

        /**
         * @brief New JS `Promise`
         * @param f Callback to do work in background
         * @return JSValue of the `Promise`
         */
        JSValue newPromiseVoid(const std::function<void()> &f);
        JSValue newPromiseBool(const std::function<bool()> &f);
        JSValue newPromiseInt32(const std::function<int32_t()> &f);
        JSValue newPromiseInt64(const std::function<int64_t()> &f);
        JSValue newPromiseFloat(const std::function<double()> &f);
        JSValue newPromiseString(const std::function<const std::string()> &f);

        /**
         * Relase JS VM
         */
        ~JsBridge();

    private:
        JSRuntime *runtime{NULL};
        JSContext *context{NULL};
        std::vector<JSValue> jValues;

        JSValue newPromise(const std::function<JSValue()> &jf);
    };
}

#endif

#endif // NGENXX_SRC_JS_BRIDGE_HXX_
