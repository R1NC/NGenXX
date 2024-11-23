#include "Zip.hxx"
#include "../../include/NGenXXLog.hxx"
#include <algorithm>
#include <stdexcept>
#include <cstring>
#include <functional>
#include <vector>

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

int NGenXX::Z::ZBase::windowBits()
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

NGenXX::Z::ZBase::ZBase(const size_t bufferSize, const int format) : bufferSize{bufferSize}, format{format}
{
    if (bufferSize <= 0)
    {
        throw std::invalid_argument("bufferSize invalid");
    }
    if (format != NGenXXZFormatRaw && format != NGenXXZFormatZLib && format != NGenXXZFormatGZip)
    {
        throw std::invalid_argument("format invalid");
    }
    this->zs = {
        .zalloc = Z_NULL,
        .zfree = Z_NULL,
        .opaque = Z_NULL
    };
    this->inBuffer = reinterpret_cast<byte *>(malloc(sizeof(byte) * bufferSize + 1));
    this->outBuffer = reinterpret_cast<byte *>(malloc(sizeof(byte) * bufferSize + 1));
}

const size_t NGenXX::Z::ZBase::input(const Bytes &bytes, bool inFinish)
{
    if (bytes.empty())
    {
        return 0;
    }
    size_t dataLen = std::min(bytes.size(), this->bufferSize);

    std::memset(this->inBuffer, 0, this->bufferSize);
    memcpy(this->inBuffer, bytes.data(), dataLen);

    (this->zs).avail_in = static_cast<uint>(dataLen);
    (this->zs).next_in = reinterpret_cast<Bytef *>(this->inBuffer);
    this->inFinish = inFinish;
    return dataLen;
}

const Bytes NGenXX::Z::ZBase::processDo()
{
    std::memset(this->outBuffer, 0, this->bufferSize);
    (this->zs).avail_out = static_cast<uint>(this->bufferSize);
    (this->zs).next_out = reinterpret_cast<Bytef *>(this->outBuffer);

    ngenxxLogPrint(NGenXXLogLevelX::Debug, "z process before avIn:" + std::to_string((this->zs).avail_in) + " avOut:" + std::to_string((this->zs).avail_out));
    this->processImp();
    ngenxxLogPrint(NGenXXLogLevelX::Debug, "z process after avIn:" + std::to_string((this->zs).avail_in) + " avOut:" + std::to_string((this->zs).avail_out));

    if (this->ret != Z_OK && ret != Z_STREAM_END)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "z process error:" + std::to_string(this->ret));
        return BytesEmpty;
    }

    size_t outLen = this->bufferSize - (this->zs).avail_out;
    return wrapBytes(this->outBuffer, outLen);
}

const bool NGenXX::Z::ZBase::processFinished()
{
    return (this->zs).avail_out != 0;
}

NGenXX::Z::ZBase::~ZBase()
{
    free((void *)inBuffer);
    free((void *)outBuffer);
}

NGenXX::Z::Zip::Zip(int mode, const size_t bufferSize, const int format) : ZBase(bufferSize, format)
{
    if (mode != NGenXXZipCompressModeDefault && mode != NGenXXZipCompressModePreferSize && mode != NGenXXZipCompressModePreferSpeed)
    {
        throw std::invalid_argument("mode invalid");
    }
    this->ret = deflateInit2(&(this->zs), mode, Z_DEFLATED, this->windowBits(), 8, Z_DEFAULT_STRATEGY);
    if (this->ret != Z_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "deflateInit error:" + std::to_string(this->ret));
        throw std::runtime_error("deflateInit failed");
    }
}

void NGenXX::Z::Zip::processImp()
{
    this->ret = deflate(&(this->zs), this->inFinish ? Z_FINISH : Z_NO_FLUSH);
}

NGenXX::Z::Zip::~Zip()
{
    deflateEnd(&(this->zs));
}

NGenXX::Z::UnZip::UnZip(const size_t bufferSize, const int format) : ZBase(bufferSize, format)
{
    this->ret = inflateInit2(&(this->zs), this->windowBits());
    if (this->ret != Z_OK)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "inflateInit error:" + std::to_string(this->ret));
        throw std::runtime_error("inflateInit failed");
    }
}

void NGenXX::Z::UnZip::processImp()
{
    this->ret = inflate(&(this->zs), Z_NO_FLUSH);
}

NGenXX::Z::UnZip::~UnZip()
{
    inflateEnd(&(this->zs));
}

#pragma mark Stream

bool zProcess(const size_t bufferSize,
              std::function<Bytes()> sReadF,
              std::function<void(Bytes)> sWriteF,
              std::function<void()> sFlushF,
              NGenXX::Z::ZBase &zb)
{
    bool inputFinished;
    do
    {
        auto in = sReadF();
        inputFinished = in.size() < bufferSize;
        auto ret = zb.input(in, inputFinished);
        if (ret == 0L)
        {
            return false;
        }

        bool processFinished;
        do
        {
            auto outData = zb.processDo();
            if (outData.empty())
            {
                return false;
            }
            processFinished = zb.processFinished();

            sWriteF(outData);
        } while (!processFinished);
    } while (!inputFinished);
    sFlushF();

    return true;
}

#pragma mark Cxx stream

bool zProcessCxxStream(const size_t bufferSize, std::istream *inStream, std::ostream *outStream, NGenXX::Z::ZBase &zb)
{
    return zProcess(bufferSize, 
        [bufferSize, &inStream]() -> Bytes 
        {
            Bytes in;
            inStream->readsome(reinterpret_cast<char *>(in.data()), bufferSize);
            return in; 
        }, 
        [&outStream](Bytes bytes) -> void
        { 
            outStream->write(reinterpret_cast<char *>(const_cast<byte *>(bytes.data())), bytes.size());
        }, 
        [&outStream]() -> void
        { 
            outStream->flush(); 
        }, 
        zb
    );
}

bool NGenXX::Z::zip(int mode, const size_t bufferSize, const int format, std::istream *inStream, std::ostream *outStream)
{
    auto zip = Zip(mode, bufferSize, format);
    return zProcessCxxStream(bufferSize, inStream, outStream, zip);
}

bool NGenXX::Z::unzip(const size_t bufferSize, const int format, std::istream *inStream, std::ostream *outStream)
{
    auto unzip = UnZip(bufferSize, format);
    return zProcessCxxStream(bufferSize, inStream, outStream, unzip);
}

#pragma mark C FILE

bool zProcessCFILE(const size_t bufferSize, std::FILE *inFile, std::FILE *outFile, NGenXX::Z::ZBase &zb)
{
    return zProcess(bufferSize, 
        [bufferSize, &inFile]() -> Bytes
        {
            Bytes in;
            std::fread(static_cast<void *>(in.data()), sizeof(byte), bufferSize, inFile);
            return in;
        }, 
        [&outFile](Bytes bytes) -> void
        { 
            std::fwrite(bytes.data(), sizeof(byte), bytes.size(), outFile);
        }, 
        [&outFile]() -> void
        { 
            std::fflush(outFile);
        }, 
        zb
    );
}

bool NGenXX::Z::zip(int mode, const size_t bufferSize, const int format, std::FILE *inFile, std::FILE *outFile)
{
    auto zip = Zip(mode, bufferSize, format);
    return zProcessCFILE(bufferSize, inFile, outFile, zip);
}

bool NGenXX::Z::unzip(const size_t bufferSize, const int format, std::FILE *inFile, std::FILE *outFile)
{
    auto unzip = UnZip(bufferSize, format);
    return zProcessCFILE(bufferSize, inFile, outFile, unzip);
}

#pragma mark Bytes

const Bytes zProcessBytes(const size_t bufferSize, const Bytes &in, NGenXX::Z::ZBase &zb)
{
    size_t pos = 0;
    std::vector<byte> outBytes;
    auto b = zProcess(bufferSize, 
        [bufferSize, &in, &pos]() -> Bytes
        {
            auto len = std::min(bufferSize, in.size() - pos);
            Bytes bytes;
            for (auto i = 0; i < len; i++)
            {
                bytes.push_back(in[pos]);
                pos++;
            }
            return bytes; 
        }, 
        [&outBytes](Bytes bytes) -> void
        {
            outBytes.insert(outBytes.end(), bytes.begin(), bytes.end());
        }, 
        []() -> void {}, 
        zb
    );

    if (!b)
    {
        return BytesEmpty;
    }
    return outBytes;
}

const Bytes NGenXX::Z::zip(int mode, const size_t bufferSize, const int format, const Bytes &bytes)
{
    auto zip = Zip(mode, bufferSize, format);
    return zProcessBytes(bufferSize, bytes, zip);
}

const Bytes NGenXX::Z::unzip(const size_t bufferSize, const int format, const Bytes &bytes)
{
    auto unzip = UnZip(bufferSize, format);
    return zProcessBytes(bufferSize, bytes, unzip);
}