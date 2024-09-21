#include "inc/include.h"

MemoryManager* gmm;

u64 mm::detail::aob_search_buf_single(void *pHayStack, void *pNeedle, size_t uHayStackLength, size_t uNeedleLength)
{
  if (pHayStack == nullptr || pNeedle == nullptr)
    return 0;

  if (uHayStackLength <= 0 || uNeedleLength <= 0)
    return 0;

  for (auto *pBuffer = static_cast<u8 *>(pHayStack); uHayStackLength >= uNeedleLength; ++pBuffer, --uHayStackLength)
  {
    if (memcmp(pBuffer, pNeedle, uNeedleLength) == 0)
      return reinterpret_cast<u64>(pBuffer);
  }

  return 0;
}

char mm::detail::vm_read(int pid, void* dst, u64 src, u64 len)
{
    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_len = len;
    remote[0].iov_len = len;

    local[0].iov_base = dst;
    remote[0].iov_base = PTR_OF(src);

    size_t rs = process_vm_readv(
        pid,
        local, 1,
        remote, 1,
        0
    );

    if(rs == -1)
    {
        //msg("vm_read failed: %i", errno);
        return 0;
    }

    return (rs > 0);
}

char mm::detail::vm_write(int pid, u64 dst, const void* src, u64 len)
{
    struct iovec local[1];
    struct iovec remote[1];

    local[0].iov_len = len;
    remote[0].iov_len = len;

    local[0].iov_base = (void*)src;
    remote[0].iov_base = PTR_OF(dst);

    size_t rs = process_vm_writev(
        pid,
        local, 1,
        remote, 1,
        0
    );

    if(rs == -1)
    {
        //msg("vm_read failed: %i", errno);
        return 0;
    }

    return (rs > 0);
}

bool MemoryManager::read(void* dst, u64 src, size_t len)
{
    if(!mm::detail::vm_read(m_pid, dst, src, len)) return false;
    return true;
}

bool MemoryManager::write(u64 dst, const void* src, size_t len)
{
    if(!mm::detail::vm_write(m_pid, dst, src, len)) return false;
    return true;
}

