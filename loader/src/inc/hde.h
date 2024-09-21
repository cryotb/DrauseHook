#pragma once

#include "table64.h"
#include "hde64.h"

namespace hde
{
    size_t disasm_buffer(void *data, size_t length, hde64s *list, size_t list_length);
    std::vector<hde64s> disasm_buffer_ex(void *data, size_t length);
}

