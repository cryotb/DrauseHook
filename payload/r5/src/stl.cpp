#include "inc/include.h"

#define ASSIGN_REINTERPRET(DST, VAL) DST = reinterpret_cast<decltype(DST)>(VAL)

void __guaranteed_stosb(void *dest, unsigned char value, size_t count)
{
  __asm__ __volatile__(
      "rep stosb"
      : "+D"(dest), "+c"(count)
      : "a"(value)
      : "memory");
}

void __guaranteed_movsb(void *dest, const void *src, size_t count)
{
  __asm__ __volatile__(
      "rep movsb"
      : "+D"(dest), "+S"(src), "+c"(count)
      :
      : "memory");
}

namespace stl
{
  bool init(uptr libc_base, uptr libm_base)
  {
    baseof_libc = libc_base;
    baseof_libm = libm_base;
    if (!baseof_libc || !baseof_libm)
      return false;

    fps::sprintf = reinterpret_cast<decltype(fps::sprintf)>(baseof_libc + gctx->linuxcrt_cpp.sprintf);
    fps::sleep = reinterpret_cast<decltype(fps::sleep)>(baseof_libc + gctx->linuxcrt_cpp.sleep);

    return true;
  }

  void *global_mm_alloc(size_t len)
  {
    if (!g.ix.mm)
    {
    #if LOGGING_IS_ENABLED == 1
      msg("mm iface is null!");
    #endif
      __fastfail(0x3D2A);
    }

    auto ptr = g.ix.mm->alloc(len);
    if (ptr == nullptr)
    {
    #if LOGGING_IS_ENABLED == 1
      msg("mm alloc has failed, returned null!");
    #endif
      __fastfail(0x3D4A);
    }
    return ptr;
  }

  void global_mm_free(void *p)
  {
    if (!g.ix.mm)
    {
    #if LOGGING_IS_ENABLED == 1
      msg("mm iface is null!");
    #endif
      __fastfail(0x3D2A);
    }

    g.ix.mm->free(p);
  }
}
