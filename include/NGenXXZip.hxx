#ifndef NGENXX_INCLUDE_Z_HXX_
#define NGENXX_INCLUDE_Z_HXX_

#include "NGenXXTypes.hxx"

constexpr size_t NGenXXZDefaultBufferSize = 16 * 1024;

enum class NGenXXZipCompressModeX : int
{
    Default = -1,
    PreferSpeed = 1,
    PreferSize = 9,
};

enum class NGenXXZFormatX : int
{
    ZLib = 0,
    GZip = 1,
    Raw = 2,
};

void *ngenxxZZipInit(const NGenXXZipCompressModeX mode = NGenXXZipCompressModeX::Default,
                     const size_t bufferSize = NGenXXZDefaultBufferSize,
                     const NGenXXZFormatX format = NGenXXZFormatX::ZLib);

size_t ngenxxZZipInput(void *const zip, const Bytes &inBytes, const bool inFinish);

Bytes ngenxxZZipProcessDo(void *const zip);

bool ngenxxZZipProcessFinished(void *const zip);

void ngenxxZZipRelease(void *const zip);

void *ngenxxZUnzipInit(const size_t bufferSize = NGenXXZDefaultBufferSize,
                       const NGenXXZFormatX format = NGenXXZFormatX::ZLib);

size_t ngenxxZUnzipInput(void *const unzip, const Bytes &inBytes, const bool inFinish);

Bytes ngenxxZUnzipProcessDo(void *const unzip);

bool ngenxxZUnzipProcessFinished(void *const unzip);

void ngenxxZUnzipRelease(void *const unzip);

bool ngenxxZCFileZip(std::FILE *cFILEIn, std::FILE *cFILEOut,
                     const NGenXXZipCompressModeX mode = NGenXXZipCompressModeX::Default,
                     const size_t bufferSize = NGenXXZDefaultBufferSize,
                     const NGenXXZFormatX format = NGenXXZFormatX::ZLib);

bool ngenxxZCFileUnzip(std::FILE *cFILEIn, std::FILE *cFILEOut,
                       const size_t bufferSize = NGenXXZDefaultBufferSize,
                       const NGenXXZFormatX format = NGenXXZFormatX::ZLib);

bool ngenxxZCxxStreamZip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut,
                         const NGenXXZipCompressModeX mode = NGenXXZipCompressModeX::Default,
                         const size_t bufferSize = NGenXXZDefaultBufferSize,
                         const NGenXXZFormatX format = NGenXXZFormatX::ZLib);

bool ngenxxZCxxStreamUnzip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut,
                           const size_t bufferSize = NGenXXZDefaultBufferSize,
                           const NGenXXZFormatX format = NGenXXZFormatX::ZLib);

Bytes ngenxxZBytesZip(const Bytes &inBytes,
                            const NGenXXZipCompressModeX mode = NGenXXZipCompressModeX::Default,
                            const size_t bufferSize = NGenXXZDefaultBufferSize,
                            const NGenXXZFormatX format = NGenXXZFormatX::ZLib);

Bytes ngenxxZBytesUnzip(const Bytes &inBytes,
                              const size_t bufferSize = NGenXXZDefaultBufferSize,
                              const NGenXXZFormatX format = NGenXXZFormatX::ZLib);

#endif // NGENXX_INCLUDE_Z_HXX_