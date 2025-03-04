#ifndef NGENXX_SRC_ZIP_HXX_
#define NGENXX_SRC_ZIP_HXX_

#include <zlib.h>

#if defined(__cplusplus)

#include <cstdio>

#include <istream>
#include <ostream>

#include <NGenXXZip.h>
#include <NGenXXTypes.hxx>

namespace NGenXX
{
    namespace Z
    {
        template <typename T>
        class ZBase
        {
        public:
            ZBase() = delete;
            explicit ZBase(size_t bufferSize, int format);
            ZBase(const ZBase &) = delete;
            ZBase &operator=(const ZBase &) = delete;
            ZBase(ZBase &&) = delete;
            ZBase &operator=(ZBase &&) = delete;
            virtual ~ZBase();

            size_t input(const Bytes &bytes, bool inFinish);
            Bytes processDo();
            bool processFinished() const;

        protected:
            z_stream zs;
            int ret{Z_OK};
            bool inFinish{false};
            int windowBits();

        private:
            byte *inBuffer{nullptr};
            byte *outBuffer{nullptr};
            size_t bufferSize;
            int format;
        };

        class Zip : public ZBase<Zip>
        {
        public:
            Zip() = delete;
            explicit Zip(int mode, size_t bufferSize, int format);
            Zip(const Zip &) = delete;
            Zip &operator=(const Zip &) = delete;
            Zip(Zip &&) = delete;
            Zip &operator=(Zip &&) = delete;
            ~Zip() override;

        private:
            friend class ZBase<Zip>;
            void processImp();
        };

        class UnZip : public ZBase<UnZip>
        {
        public:
            UnZip() = delete;
            explicit UnZip(size_t bufferSize, int format);
            UnZip(const UnZip &) = delete;
            UnZip &operator=(const UnZip &) = delete;
            UnZip(UnZip &&) = delete;
            UnZip &operator=(UnZip &&) = delete;
            ~UnZip() override;

        private:
            friend class ZBase<UnZip>;
            void processImp();
        };

        bool zip(int mode, size_t bufferSize, int format, std::istream *inStream, std::ostream *outStream);

        bool unzip(size_t bufferSize, int format, std::istream *inStream, std::ostream *outStream);

        bool zip(int mode, size_t bufferSize, int format, std::FILE *inFile, std::FILE *outFile);

        bool unzip(size_t bufferSize, int format, std::FILE *inFile, std::FILE *outFile);

        Bytes zip(int mode, size_t bufferSize, int format, const Bytes &bytes);

        Bytes unzip(size_t bufferSize, int format, const Bytes &bytes);
    }
}

#endif

#endif // NGENXX_SRC_ZIP_HXX_