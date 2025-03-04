#ifndef NGENXX_INCLUDE_LOG_LOG_H_
#define NGENXX_INCLUDE_LOG_LOG_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     *
     */
    enum NGenXXLogLevel
    {
        NGenXXLogLevelDebug = 3,
        NGenXXLogLevelInfo,
        NGenXXLogLevelWarn,
        NGenXXLogLevelError,
        NGenXXLogLevelFatal,
        NGenXXLogLevelNone
    };

    /**
     * @brief Set log level
     * @warning Not accessible in JS/Lua!
     * @param level Log level
     */
    void ngenxx_log_set_level(int level);

    /**
     * @brief Set log callback
     * @warning Not accessible in JS/Lua!
     * @param callback log callback
     */
    void ngenxx_log_set_callback(void (*const callback)(int level, const char *content));

    /**
     * @brief Print log
     * @param level Log level
     * @param content Log content
     */
    void ngenxx_log_print(int level, const char *content);

#ifdef __cplusplus
}
#endif

#endif // NGENXX_INCLUDE_LOG_LOG_H_