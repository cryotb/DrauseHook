#pragma once

namespace mem
{
    struct range {
        uptr m_begin{};
        uptr m_end{};
        uptr m_size{};
    };

    class addr
    {
    public:
        addr() = default;
        ~addr() = default;

        bool operator==(const uptr& other) const
        {
            return (m_base == other);
        }

        bool operator==(const uptr& lhs)
        {
            return (lhs == m_base);
        }

        addr(const uptr base)
        {
            m_base = base;
        }

        addr(void* const ptr)
        {
            m_base = reinterpret_cast<uptr>(ptr);
        }

        [[nodiscard]] auto Get() const
        {
            return addr(m_base);
        }

        [[nodiscard]] auto Base() const
        {
            return m_base;
        }

        [[nodiscard]] auto Valid() const
        {
            return !(m_base == 0);
        }

        [[nodiscard]] auto Add(const uptr t) const
        {
            return addr(m_base + t);
        }

        [[nodiscard]] auto Sub(const uptr t) const
        {
            return addr(m_base - t);
        }

        [[nodiscard]] auto Deref() const
        {
            return addr(*reinterpret_cast<uptr*>(m_base));
        }

        [[nodiscard]] auto Ptr() const
        {
            return reinterpret_cast<void*>(m_base);
        }

        template < class T >
        [[nodiscard]] auto As()
        {
            return reinterpret_cast<T>(m_base);
        }

        template < class T >
        [[nodiscard]] auto Retrieve_as()
        {
            return *reinterpret_cast<T*>(m_base);
        }

        template < class T >
        void Set(const T& value)
        {
            *As<T*>() = value;
        }
    private:
        uptr m_base;
    };

    inline uptr resolve_operandof_lea(const u8 *rip)
    {
        u8 offset = 0;
        u8 skip = 3;

        if (rip[0] == 0x48)
        {
            /* REX.W PREFIX */
            offset = 1;
            --skip;
        }

        auto disp = *(int32_t *)(rip + offset + skip);
        return reinterpret_cast<uptr>(rip + 7 + disp);
    }
}
