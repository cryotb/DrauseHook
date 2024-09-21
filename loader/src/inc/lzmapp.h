//
// Created by drof on 27.01.24.
//

#ifndef BACKEND_LZMAPP_H
#define BACKEND_LZMAPP_H

namespace lzma
{
    extern std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    extern std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);
}


#endif //BACKEND_LZMAPP_H
