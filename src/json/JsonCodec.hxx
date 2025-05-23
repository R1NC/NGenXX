#ifndef NGENXX_SRC_JSON_CODEC_HXX_
#define NGENXX_SRC_JSON_CODEC_HXX_

#include <cJSON.h>

#if defined(__cplusplus)

#include <string>
#include <functional>

#include <NGenXXTypes.hxx>

namespace NGenXX::Json
    {
        std::string jsonFromDictAny(const DictAny &dict);
        DictAny jsonToDictAny(const std::string &json);

        std::string cJSON2Str(void *const cjson);
        
        class Decoder
        {
        public:
            Decoder() = delete;
            explicit Decoder(const std::string &json);
            Decoder(const Decoder &) = delete;
            Decoder &operator=(const Decoder &) = delete;
            Decoder(Decoder &&) noexcept;
            Decoder &operator=(Decoder &&) noexcept;

            bool isArray(const void *const node) const;

            bool isObject(const void *const node) const;

            void *readChild(const void *const node) const;

            void *readNext(const void *const node) const;

            void readChildren(const void *const node, std::function<void(size_t idx, const void * child)> &&callback) const;

            void *readNode(const void *const node, const std::string &k) const;
            void *operator[](const std::string &k) const;

            std::string readString(const void *const node) const;

            double readNumber(const void *const node) const;

            ~Decoder();

        private:
            cJSON *cjson{nullptr};

            void moveImp(Decoder&& other) noexcept;
            void cleanup() noexcept;

            const cJSON *reinterpretNode(const void *const node) const;
        };
}

#endif

#endif // NGENXX_SRC_JSON_CODEC_HXX_