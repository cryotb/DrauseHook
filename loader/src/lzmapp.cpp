#include "inc/include.h"

#pragma warning(push, 0)
#define POCKETLZMA_LZMA_C_DEFINE
#include "inc/pocketlzma.h"
#pragma warning(pop)

namespace lzma
{
    std::vector<uint8_t> compress(const std::vector<uint8_t>& data)
    {
        std::vector<uint8_t> compressed_data;

        plz::PocketLzma p;
        p.usePreset(plz::Preset::Fast);
        plz::StatusCode status = p.compress(data, compressed_data);
        if (status != plz::StatusCode::Ok)
        {
            dbg("failed CALL to compress: %lld", status);
            compressed_data.clear();
            return compressed_data;
        }

        return compressed_data;
    }

    std::vector<uint8_t> decompress(const std::vector<uint8_t>& data)
    {
        std::vector<uint8_t> decompressed_data;

        plz::PocketLzma p;
        plz::StatusCode status = p.decompress(data, decompressed_data);
        if (status != plz::StatusCode::Ok)
        {
            dbg("failed CALL to compress: %lld", status);
            decompressed_data.clear();
            return decompressed_data;
        }

        return decompressed_data;
    }
}

