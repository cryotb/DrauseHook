//
// Created by drof on 27.01.24.
//

#ifndef BACKEND_ROTATE_H
#define BACKEND_ROTATE_H

#pragma once

namespace rotate
{
    template<class T>
    constexpr T left(T value, int count)
    {
        const uint nbits = sizeof(T) * 8;

        if (count > 0)
        {
            count %= nbits;
            T high = value >> (nbits - count);
            if (T(-1) < 0) // signed value
                high &= ~((T(-1) << count));
            value <<= count;
            value |= high;
        }
        else
        {
            count = -count % nbits;
            T low = value << (nbits - count);
            value >>= count;
            value |= low;
        }
        return value;
    }

    template<class T>
    constexpr T right(T value, int count)
    {
        count = -count; // invert it for this op.

        const uint nbits = sizeof(T) * 8;

        if (count > 0)
        {
            count %= nbits;
            T high = value >> (nbits - count);
            if (T(-1) < 0) // signed value
                high &= ~((T(-1) << count));
            value <<= count;
            value |= high;
        }
        else
        {
            count = -count % nbits;
            T low = value << (nbits - count);
            value >>= count;
            value |= low;
        }
        return value;
    }

    inline auto encode(void* data, u32 len)
    {
        for (u32 i = 0; i < len; i++)
        {
            reinterpret_cast<u8*>(data)[i] =
                    left<u8>(reinterpret_cast<u8*>(data)[i], 1);
            reinterpret_cast<u8*>(data)[i] ^= len;
        }
    }

    inline auto decode(void* data, u32 len)
    {
        for (u32 i = 0; i < len; i++)
        {
            reinterpret_cast<u8*>(data)[i] ^= len;
            reinterpret_cast<u8*>(data)[i] =
                    right<u8>(reinterpret_cast<u8*>(data)[i], 1);
        }
    }
}


#endif //BACKEND_ROTATE_H
