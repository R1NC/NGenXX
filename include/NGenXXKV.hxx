#ifndef NGENXX_INCLUDE_KV_HXX_
#define NGENXX_INCLUDE_KV_HXX_

#include "NGenXXTypes.hxx"

void *ngenxxStoreKvOpen(const std::string &_id);

std::optional<std::string> ngenxxStoreKvReadString(void *const conn, std::string_view k);

bool ngenxxStoreKvWriteString(void *const conn, std::string_view k, const std::string &v);

std::optional<int64_t> ngenxxStoreKvReadInteger(void *const conn, std::string_view k);

bool ngenxxStoreKvWriteInteger(void *const conn, std::string_view k, int64_t v);

std::optional<double> ngenxxStoreKvReadFloat(void *const conn, std::string_view k);

bool ngenxxStoreKvWriteFloat(void *const conn, std::string_view k, double v);

std::vector<std::string> ngenxxStoreKvAllKeys(void *const conn);

bool ngenxxStoreKvContains(void *const conn, std::string_view k);

bool ngenxxStoreKvRemove(void *const conn, std::string_view k);

void ngenxxStoreKvClear(void *const conn);

void ngenxxStoreKvClose(void *const conn);

#endif // NGENXX_INCLUDE_KV_HXX_
