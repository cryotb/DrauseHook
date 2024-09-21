#include "inc/include.h"

int pt::_status = 0;
int pt::_pid = 0;
u64 pt::_gadget = 0;

bool pt::syscall(int pid, u64 gadget, int sid,
                 u64 param_rdi, u64 param_rsi, u64 param_rdx,
                 u64 param_r10, u64 param_r8, u64 param_r9, u64 *result)
{
  user_regs_struct orig_regs, regs, result_regs;

  memset(&orig_regs, 0, sizeof(orig_regs));
  memset(&regs, 0, sizeof(regs));
  memset(&result_regs, 0, sizeof(result_regs));

  if (!context_capture(pid, &orig_regs))
    return false;

  memcpy(&regs, &orig_regs, sizeof(regs));

  // setup params
  regs.rax = sid; // SYSCALL ID
  regs.rdi = param_rdi;
  regs.rsi = param_rsi;
  regs.rdx = param_rdx;
  regs.r10 = param_r10;
  regs.r8 = param_r8;
  regs.r9 = param_r9;
  regs.rip = gadget;

  if (!context_swap(pid, &regs))
    return false;

  pt::single_step(pid);
  auto sync_rs = pt::sync(pid);

  *result = context(pid).rax;

  // SYSCALL -> EXIT
  regs.rax = 0x60;
  regs.rdi = 0;
  regs.rip = gadget;

  if (!context_swap(pid, &regs))
    return false;

  pt::single_step(pid);
  sync_rs = pt::sync(pid);

  if (!context_swap(pid, &orig_regs))
    return false;

  return true;
}

u64 pt::mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
  u64 result = 0;
  if (!pt::syscall(_pid, _gadget, 9,
                   (u64)addr, len, prot, flags, fd, offset,
                   &result))
  {
    safeLog_msg("syscall_wrap -> mmap failed!");
    return 0;
  }

  return result;
}

bool pt::mprotect(u64 addr, size_t len, int prot)
{
  u64 result = 0;
  if (!pt::syscall(_pid, _gadget, 10,
                   addr, len, prot, 0, 0, 0,
                   &result))
  {
    safeLog_msg("syscall_wrap -> mmap failed!");
    return false;
  }

  /*
   * On success, mprotect() return zero.  On
       error, these system calls return -1, and errno is set to indicate
       the error.
  */
  return (result == 0);
}
