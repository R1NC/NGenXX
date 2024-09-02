#ifndef NGENXX_H_
#define NGENXX_H_

#include "NGenXXLog.h"
#include "NGenXXNet.h"
#include "NGenXXLua.h"

/**
 * @brief Read version
 * @return version name
 */
const char *ngenxx_get_version(void);

/**
 * @brief Initialize SDK
 * @warning Not accessible in Lua!
 * @param use_lua Use Lua or not
 * @return SDK handle
 */
void *ngenxx_init(bool use_lua);

/**
 * @brief Release SDK
 * @warning Not accessible in Lua!
 * @param handle SDK handle
 */
void ngenxx_release(void *handle);

#endif // NGENXX_H_