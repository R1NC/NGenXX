#ifndef NGENXX_NET_H_
#define NGENXX_NET_H_

/**
 *
 */
enum NGenXXNetHttpMethod
{
    Get,
    Post,
};

/**
 * @brief http request
 * @param url URL
 * @param params params(transfer multiple params like `v1=a&v2=b`)
 * @param method HTTP method, see `NGenXXNetHttpMethod`
 * @param headers_v HTTP headers vector, max length is 8190
 * @param headers_c HTTP headers count, max count is 100
 * @param timeout Timeout(milliseconds)
 * @return response
 */
const char *ngenxx_net_http_request(const char *url, const char *params, int method, char **headers_v, int headers_c, long timeout);

#endif // NGENXX_NET_H_