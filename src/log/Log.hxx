#ifndef NGENXX_SRC_LOG_HXX_
#define NGENXX_SRC_LOG_HXX_

#if defined(__cplusplus)

#include <string>
#include <functional>

namespace NGenXX::Log
    {
        void setLevel(int level);

        void setCallback(const std::function<void(int level, const char *content)> &callback);

        void print(int level, const std::string &content);
}

#endif
#endif // NGENXX_SRC_LOG_HXX_