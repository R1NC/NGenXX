#include "HttpClient.hxx"

#include "../../../external/curl/include/curl/curl.h"
#include "../../include/NGenXXLog.h"
#include "../../include/NGenXXNetHttp.h"
#include "../log/Log.hxx"
#include <algorithm>
#include <vector>
#include <string>

#define DEFAULT_TIMEOUT 5000L

size_t _NGenXX_Net_HttpClient_WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

NGenXX::Net::HttpClient::HttpClient()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

NGenXX::Net::HttpClient::~HttpClient()
{
    curl_global_cleanup();
}

const std::string NGenXX::Net::HttpClient::request(const std::string &url, const std::string &params, int method, std::vector<std::string> &headers, long timeout)
{
    std::string rsp;
    if (timeout <= 0)
    {
        timeout = DEFAULT_TIMEOUT;
    }

    CURL *curl = curl_easy_init();
    if (curl)
    {
        Log::print(NGenXXLogLevelDebug, "HttpClient.request url: " + url + " params: " + params);

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, timeout);
        curl_easy_setopt(curl, CURLOPT_SERVER_RESPONSE_TIMEOUT_MS, timeout);

        if (method == NGenXXNetHttpMethodPost)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, params.c_str());
        }
        else
        {
            auto loc = url.find("?", 0);
            if (loc == std::string::npos)
            {
                curl_easy_setopt(curl, CURLOPT_URL, (url + "?" + params).c_str());
            }
            else
            {
                curl_easy_setopt(curl, CURLOPT_URL, (url + params).c_str());
            }
        }

        struct curl_slist *headerList = NULL;
        for (auto it = headers.begin(); it != headers.end(); ++it)
        {
            Log::print(NGenXXLogLevelDebug, "HttpClient.request header: " + (*it));
            headerList = curl_slist_append(headerList, (*it).c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

        static const std::string prefix("https://");
        auto searchRes = std::mismatch(prefix.begin(), prefix.end(), url.begin());
        if (searchRes.first == prefix.end())
        { // Ignore SSL cet verify
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _NGenXX_Net_HttpClient_WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rsp);

        CURLcode curlCode = curl_easy_perform(curl);
        if (curlCode != CURLE_OK)
        {
            Log::print(NGenXXLogLevelError, "HttpClient.request error:" + std::string(curl_easy_strerror(curlCode)));
        }

        curl_easy_cleanup(curl);
    }

    return rsp;
}