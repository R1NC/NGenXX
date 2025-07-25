#ifndef NGENXX_SRC_CORE_VM_JSVM_HXX_
#define NGENXX_SRC_CORE_VM_JSVM_HXX_

#include <quickjs-libc.h>

#if defined(__cplusplus)

#include <unordered_set>

#include <NGenXXTypes.hxx>

#include "BaseVM.hxx"

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

#define DEF_JS_FUNC_CHECK_VM(vm)                                       \
  {                                                                            \
    if (vm == nullptr) [[unlikely]] {                                      \
      return JS_UNDEFINED;                                                     \
    }                                                                          \
  }

#define DEF_JS_FUNC_VOID_ASYNC(vm, fJ, fS)                                 \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    DEF_JS_FUNC_CHECK_VM(vm);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseVoid(                                             \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

#define DEF_JS_FUNC_BOOL_ASYNC(bridge, fJ, fS)                                 \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    DEF_JS_FUNC_CHECK_VM(vm);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseBool(                                             \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

#define DEF_JS_FUNC_INT32_ASYNC(bridge, fJ, fS)                                \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    DEF_JS_FUNC_CHECK_VM(vm);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseInt32(                                            \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

#define DEF_JS_FUNC_INT64_ASYNC(bridge, fJ, fS)                                \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    DEF_JS_FUNC_CHECK_VM(vm);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseInt64(                                            \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

#define DEF_JS_FUNC_FLOAT_ASYNC(bridge, fJ, fS)                                \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    DEF_JS_FUNC_CHECK_VM(vm);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseFloat(                                            \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

#define DEF_JS_FUNC_STRING_ASYNC(bridge, fJ, fS)                               \
  static JSValue fJ(JS_FUNC_PARAMS) {                                          \
    DEF_JS_FUNC_CHECK_VM(vm);                                          \
    std::string json = JS_FUNC_READ_JSON;                                      \
    return bridge->newPromiseString(                                           \
        [arg = json]() { return fS(arg.c_str()); });                           \
  }

namespace NGenXX::Core::VM {
    class JSVM final : public BaseVM {
    public:
        /**
         * Create JS VM
         */
        JSVM();

        JSVM(const JSVM &) = delete;

        JSVM &operator=(const JSVM &) = delete;

        JSVM(JSVM &&) = delete;

        JSVM &operator=(JSVM &&) = delete;

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
        [[nodiscard]] bool loadFile(const std::string &file, bool isModule);

        /**
         * @brief Load JS script
         * @param script JS script
         * @param name JS file name
         * @return success or not
         */
        [[nodiscard]] bool loadScript(const std::string &script, const std::string &name, bool isModule);

        /**
         * @brief Load JS ByteCode
         * @param bytes JS ByteCode
         */
        [[nodiscard]] bool loadBinary(const Bytes &bytes, bool isModule);

        /**
         * @brief call JS func
         * @param func func name
         * @param params parameters(json)
         * @param await Whether wait for the promise result or not
         */
        [[nodiscard]] std::optional<std::string> callFunc(std::string_view func, std::string_view params,
                                                          bool await);

        /**
         * @brief New JS `Promise`
         * @param f Callback to do work in background
         * @return JSValue of the `Promise`
         */
        JSValue newPromiseVoid(std::function<void()> &&f);

        JSValue newPromiseBool(std::function<bool()> &&f);

        JSValue newPromiseInt32(std::function<int32_t()> &&f);

        JSValue newPromiseInt64(std::function<int64_t()> &&f);

        JSValue newPromiseFloat(std::function<double()> &&f);

        JSValue newPromiseString(std::function<const std::string()> &&f);

        ~JSVM() override;

    private:
        JSRuntime *runtime{nullptr};
        JSContext *context{nullptr};
        JSValue jGlobal{JS_UNDEFINED};

        // Custom hash function for JSValue
        struct JSValueHash {
            std::size_t operator()(const JSValue &jv) const;
        };

        // Custom equality function for JSValue
        struct JSValueEqual {
            bool operator()(const JSValue &left, const JSValue &right) const;
        };

        std::unordered_set<JSValue, JSValueHash, JSValueEqual> jValueCache;

        JSValue newPromise(std::function<JSValue()> &&jf);

        JSValue jAwait(const JSValue obj);
    };
}

#endif

#endif // NGENXX_SRC_CORE_VM_JSVM_HXX_
