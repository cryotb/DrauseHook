#pragma once

namespace mm::detail
{
    char vm_read(int pid, void* dst, u64 src, u64 len);
    char vm_write(int pid, u64 dst, const void* src, u64 len);

    u64 aob_search_buf_single(void *pHayStack, void *pNeedle, size_t uHayStackLength, size_t uNeedleLength);
}

namespace mm
{
    inline u64 aob_search_buf_single_u64(void *pHayStack, size_t uHayStackLength, u64 uNeedle)
    {
        return detail::aob_search_buf_single(pHayStack, &uNeedle, uHayStackLength, sizeof(uNeedle));
    }
}

class MemoryManager
{
public:
    MemoryManager() = delete;
    MemoryManager(int pid)
    {
        m_pid = pid;
    }

    bool read(void* dst, u64 src, size_t len);
    bool write(u64 dst, const void* src, size_t len);

    bool fill(u64 dst, u8 ch, size_t len)
    {
        auto tmp = malloc(len);
        if(tmp)
        {
            memset(tmp, ch, len);
            auto rs = write(dst, tmp, len);
            free(tmp);
            return rs;
        }

        return false;
    }

    template < typename T >
    T tread(u64 src)
    {
        T result;
        memset(&result, 0, sizeof(result));

        read(&result, src, sizeof(result));
        return result;
    }

    template < typename T >
    bool twrite(u64 dst, const T& src)
    {
        return write(dst, &src, sizeof(src));
    }
private:
    int m_pid;
};

extern MemoryManager* gmm;
