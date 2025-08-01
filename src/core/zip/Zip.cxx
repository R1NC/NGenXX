#include "Zip.hxx"

#include <algorithm>
#include <stdexcept>
#include <functional>

#include <NGenXXLog.hxx>
#include "NGenXXZip.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif


namespace {
    template <typename T>
    bool process(const size_t bufferSize,
              std::function<const Bytes()> &&sReadF,
              std::function<void(const Bytes &)> &&sWriteF,
              std::function<void()> &&sFlushF,
              NGenXX::Core::Z::ZBase<T> &zb)
    {
        auto inputFinished = false;
        do
        {
            auto in = std::move(sReadF)();
            inputFinished = in.size() < bufferSize;
            auto ret = zb.input(in, inputFinished);
            if (ret == 0L) [[unlikely]]
            {
                return false;
            }

            auto processFinished = false;
            do
            {
                auto outData = zb.processDo();
                if (outData.empty()) [[unlikely]]
                {
                    return false;
                }
                processFinished = zb.processFinished();

                std::move(sWriteF)(outData);
            } while (!processFinished);
        } while (!inputFinished);
        std::move(sFlushF)();

        return true;
    }

    template <typename T>
    bool processCxxStream(size_t bufferSize, std::istream *inStream, std::ostream *outStream, NGenXX::Core::Z::ZBase<T> &zb)
    {
        return process(bufferSize,
            [bufferSize, inStream]
            {
                Bytes in;
                inStream->readsome(reinterpret_cast<char *>(in.data()), static_cast<std::streamsize>(bufferSize));
                return in;
            },
            [outStream](const Bytes &bytes)
            {
                outStream->write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            },
            [outStream]
            {
                outStream->flush();
            },
            zb
        );
    }

    template <typename T>
    bool processCFILE(size_t bufferSize, std::FILE *inFile, std::FILE *outFile, NGenXX::Core::Z::ZBase<T> &zb)
    {
        return process(bufferSize,
            [bufferSize, inFile]
            {
                Bytes in;
                std::fread(in.data(), sizeof(byte), bufferSize, inFile);
                return in;
            },
            [outFile](const Bytes &bytes)
            {
                std::fwrite(bytes.data(), sizeof(byte), bytes.size(), outFile);
            },
            [outFile]
            {
                std::fflush(outFile);
            },
            zb
        );
    }

    template <typename T>
    Bytes processBytes(size_t bufferSize, const Bytes &in, NGenXX::Core::Z::ZBase<T> &zb)
    {
        long pos(0);
        Bytes outBytes;
        auto b = process(bufferSize,
            [bufferSize, &in, &pos]
            {
                const auto len = static_cast<long>(std::min<size_t>(bufferSize, in.size() - pos));
                Bytes bytes(in.begin() + pos, in.begin() + pos + len);
                pos += len;
                return bytes;
            },
            [&outBytes](const Bytes &bytes)
            {
                outBytes.insert(outBytes.end(), bytes.begin(), bytes.end());
            },
            [] {},
            zb
        );
        if (!b)
        {
            return {};
        }
        return outBytes;
    }
}

template <typename T>
int NGenXX::Core::Z::ZBase<T>::windowBits() const
{
    if (this->format == NGenXXZFormatGZip)
    {
        return 16 | MAX_WBITS;
    }
    if (this->format == NGenXXZFormatRaw)
    {
        return -MAX_WBITS;
    }
    return MAX_WBITS;
}

template <typename T>
NGenXX::Core::Z::ZBase<T>::ZBase(size_t bufferSize, int format) : bufferSize{bufferSize}, format{format}
{
    if (bufferSize == 0) [[unlikely]]
    {
        throw std::invalid_argument("bufferSize invalid");
    }
    if (format != NGenXXZFormatRaw && format != NGenXXZFormatZLib && format != NGenXXZFormatGZip) [[unlikely]]
    {
        throw std::invalid_argument("format invalid");
    }
    this->inBuffer = mallocX<byte>(bufferSize);
    this->outBuffer = mallocX<byte>(bufferSize);
}

template <typename T>
size_t NGenXX::Core::Z::ZBase<T>::input(const Bytes &bytes, bool inFinish)
{
    if (bytes.empty()) [[unlikely]]
    {
        return 0;
    }
    auto dataLen = std::min<size_t>(bytes.size(), this->bufferSize);

    std::memset(this->inBuffer, 0, this->bufferSize);
    std::memcpy(this->inBuffer, bytes.data(), dataLen);

    this->zs.avail_in = static_cast<unsigned int>(dataLen);
    this->zs.next_in = this->inBuffer;
    this->inFinish = inFinish;
    return dataLen;
}

template <typename T>
Bytes NGenXX::Core::Z::ZBase<T>::processDo()
{
    std::memset(this->outBuffer, 0, this->bufferSize);
    this->zs.avail_out = static_cast<unsigned int>(this->bufferSize);
    this->zs.next_out = this->outBuffer;

    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "z process before avIn:{} avOut:{}", (this->zs).avail_in, (this->zs).avail_out);
    static_cast<T *>(this)->processImp();
    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "z process after avIn:{} avOut:{}", (this->zs).avail_in, (this->zs).avail_out);

    if (this->ret != Z_OK && ret != Z_STREAM_END) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "z process error:{}", this->ret);
        return {};
    }

    auto outLen = this->bufferSize - (this->zs).avail_out;
    return makeBytes(this->outBuffer, outLen);
}

template <typename T>
bool NGenXX::Core::Z::ZBase<T>::processFinished() const
{
    return this->zs.avail_out != 0;
}

template <typename T>
NGenXX::Core::Z::ZBase<T>::~ZBase()
{
    freeX(inBuffer);
    freeX(outBuffer);
}

// Explicit template instantiation
template class NGenXX::Core::Z::ZBase<NGenXX::Core::Z::Zip>;
template class NGenXX::Core::Z::ZBase<NGenXX::Core::Z::UnZip>;

NGenXX::Core::Z::Zip::Zip(int mode, size_t bufferSize, int format) : ZBase(bufferSize, format)
{
    if (mode != NGenXXZipCompressModeDefault && mode != NGenXXZipCompressModePreferSize && mode != NGenXXZipCompressModePreferSpeed) [[unlikely]]
    {
        throw std::invalid_argument("mode invalid");
    }
    this->ret = deflateInit2(&(this->zs), mode, Z_DEFLATED, this->windowBits(), 8, Z_DEFAULT_STRATEGY);
    if (this->ret != Z_OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "deflateInit error:{}", this->ret);
        throw std::runtime_error("deflateInit failed");
    }
}

void NGenXX::Core::Z::Zip::processImp()
{
    this->ret = deflate(&(this->zs), this->inFinish ? Z_FINISH : Z_NO_FLUSH);
}

NGenXX::Core::Z::Zip::~Zip()
{
    deflateEnd(&(this->zs));
}

NGenXX::Core::Z::UnZip::UnZip(size_t bufferSize, int format) : ZBase(bufferSize, format)
{
    this->ret = inflateInit2(&this->zs, this->windowBits());
    if (this->ret != Z_OK) [[unlikely]]
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "inflateInit error:{}", this->ret);
        throw std::runtime_error("inflateInit failed");
    }
}

void NGenXX::Core::Z::UnZip::processImp()
{
    this->ret = inflate(&(this->zs), Z_NO_FLUSH);
}

NGenXX::Core::Z::UnZip::~UnZip()
{
    inflateEnd(&this->zs);
}

#pragma mark Cxx stream

bool NGenXX::Core::Z::zip(int mode, size_t bufferSize, int format, std::istream *inStream, std::ostream *outStream)
{
    Zip zip(mode, bufferSize, format);
    return processCxxStream(bufferSize, inStream, outStream, zip);
}

bool NGenXX::Core::Z::unzip(size_t bufferSize, int format, std::istream *inStream, std::ostream *outStream)
{
    UnZip unzip(bufferSize, format);
    return processCxxStream(bufferSize, inStream, outStream, unzip);
}

#pragma mark C FILE

bool NGenXX::Core::Z::zip(int mode, size_t bufferSize, int format, std::FILE *inFile, std::FILE *outFile)
{
    Zip zip(mode, bufferSize, format);
    return processCFILE(bufferSize, inFile, outFile, zip);
}

bool NGenXX::Core::Z::unzip(size_t bufferSize, int format, std::FILE *inFile, std::FILE *outFile)
{
    UnZip unzip(bufferSize, format);
    return processCFILE(bufferSize, inFile, outFile, unzip);
}

#pragma mark Bytes

Bytes NGenXX::Core::Z::zip(int mode, size_t bufferSize, int format, const Bytes &bytes)
{
    Zip zip(mode, bufferSize, format);
    return processBytes(bufferSize, bytes, zip);
}

Bytes NGenXX::Core::Z::unzip(size_t bufferSize, int format, const Bytes &bytes)
{
    UnZip unzip(bufferSize, format);
    return processBytes(bufferSize, bytes, unzip);
}