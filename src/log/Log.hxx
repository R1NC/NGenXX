#ifndef NGENXX_LOG_HXX_
#define NGENXX_LOG_HXX_

#ifdef __cplusplus

#include <string>
#include <functional>

namespace NGenXX
{
    namespace Log
    {
        void setLevel(const int level);

        void setCallback(std::function<void(const int level, const char *content)> callback);

        void print(const int level, const std::string &content);
    }
}

#endif
#endif // NGENXX_LOG_HXX_