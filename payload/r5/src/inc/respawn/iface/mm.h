#pragma once

namespace rs
{
    class Mem_alloc
    {
    public:
        void *alloc(size_t len)
        {
            return vfunc::call<0, void *>(this, len);
        }

        void free(void *p)
        {
            return vfunc::call<5, void>(this, p);
        }
    };
}
