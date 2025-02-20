#ifndef NGENXX_SRC_NET_HTTP_CLIENT_HXX_
#define NGENXX_SRC_NET_HTTP_CLIENT_HXX_

#if defined(__cplusplus)

#include <cstdio>
#include <unordered_map>

#include <curl/curl.h>

#include <NGenXXTypes.hxx>

namespace NGenXX
{
    namespace Net
    {
        struct HttpFormField
        {
            std::string name;
            std::string mime;
            std::string data;
        };
        using HttpFormField = struct HttpFormField;

        struct HttpResponse
        {
            int code;
            std::string contentType;
            std::unordered_map<std::string, std::string> headers;
            std::string data;
        };
        using HttpResponse = struct HttpResponse;

        class HttpClient
        {
        public:
            HttpClient();
            HttpClient(const HttpClient &) = delete;
            HttpClient &operator=(const HttpClient &) = delete;
            HttpClient(HttpClient &&) = delete;
            HttpClient &operator=(HttpClient &&) = delete;

            [[nodiscard]] HttpResponse request(const std::string &url, const int method,
                                                     const std::vector<std::string> &headers,
                                                     const std::string &params,
                                                     const Bytes &rawBody,
                                                     const std::vector<HttpFormField> &formFields,
                                                     const std::FILE *cFILE, const size_t fileSize,
                                                     const size_t timeout);

            [[nodiscard]] bool download(const std::string &url, const std::string &filePath, const size_t timeout);

            ~HttpClient();

        private:
            bool checkUrlValid(const std::string &url);
            bool checkUrlHasSearch(const std::string &url);
            bool handleSSL(CURL *const curl, const std::string &url);
        };
    }
}

#endif

#endif // NGENXX_SRC_NET_HTTP_CLIENT_HXX_
