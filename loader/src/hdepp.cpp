#include "inc/include.h"

namespace hde
{
    size_t disasm_buffer(void *data, size_t length, hde64s *list, size_t list_length)
    {
        size_t left = length;
        size_t count = 0;
        size_t offset = 0;
        hde64s info{};

        do
        {
            if (mem::addr(data).Add(offset).Base() >
                mem::addr(data).Add(length).Base())
                break;

            if (!hde64_disasm(mem::addr(data).Add(offset).Ptr(), &info))
                break;

            const auto delta = left - info.len;

            if (static_cast<int>(delta) <= 0)
                break;

            memcpy(&list[count], &info, sizeof(hde64s));

            count++;
            left -= info.len;
            offset += info.len;
        } while (left > 0 && (count + 1) < list_length);

        return count;
    }

    std::vector<hde64s> disasm_buffer_ex(void *data, size_t length)
    {
        auto result = std::vector<hde64s>();

        size_t left = length;
        size_t count = 0;
        size_t offset = 0;
        hde64s info{};

        do
        {
            if (mem::addr(data).Add(offset).Base() >
                mem::addr(data).Add(length).Base())
                break;

            if (!hde64_disasm(mem::addr(data).Add(offset).Ptr(), &info))
                break;

            const auto delta = left - info.len;

            if (static_cast<int>(delta) <= 0)
                break;

            result.push_back(info);

            count++;
            left -= info.len;
            offset += info.len;
        } while (left > 0);

        return result;
    }
}
