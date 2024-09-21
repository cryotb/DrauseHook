#pragma once

namespace pt
{
  extern int _status;
  extern int _pid;
  extern u64 _gadget;

  inline int sync(int pid)
  {
    waitpid(pid, &_status, 0);
    return _status;
  }

  inline bool attach(int pid)
  {
    auto rs = ptrace(PTRACE_ATTACH, pid, 0, 0) > -1;

    if(!rs)
    {
      safeLog_msg("pt::attach failed -> %i", errno);
      return false;
    }

    sync(pid);

    return true;
  }

  inline bool detach(int pid)
  {
    auto rs = ptrace(PTRACE_DETACH, pid, 0, 0) > -1;

    if(!rs)
    {
      safeLog_msg("pt::detach failed -> %i", errno);
      return false;
    }

    return true;
  }

  inline bool context_capture(int pid, user_regs_struct* buf)
  {
    auto rs = ptrace(PTRACE_GETREGS, pid, 0, buf) > -1;

    if(!rs)
    {
      safeLog_msg("pt::context_capture failed -> %i", errno);
      return false;
    }

    return true;
  }

  inline bool context_swap(int pid, user_regs_struct* buf)
  {
    auto rs = ptrace(PTRACE_SETREGS, pid, 0, buf) > -1;

    if(!rs)
    {
      safeLog_msg("pt::context_swap failed -> %i", errno);
      return false;
    }

    return true;
  }

  inline user_regs_struct context(int pid)
  {
    user_regs_struct rs;
    memset(&rs, 0, sizeof(rs));

    if(!context_capture(pid, &rs))
        safeLog_msg("failed capture context.");

    return rs;
  }

  inline bool resume(int pid)
  {
    auto rs = ptrace(PTRACE_CONT, pid, 0, 0) > -1;

    if(!rs)
    {
      safeLog_msg("pt::Resume failed -> %i", errno);
      return false;
    }

    return true;
  }

  inline bool single_step(int pid)
  {
    auto rs = ptrace(PTRACE_SINGLESTEP, pid, 0, 0) > -1;

    if(!rs)
    {
      safeLog_msg("pt::SingleStep failed -> %i", errno);
      return false;
    }

    return true;
  }

  inline bool write(int pid, u64 addr, u64 value)
  {
    auto rs = ptrace(PTRACE_POKETEXT, pid, addr, value) > -1;

    if(!rs)
    {
      safeLog_msg("pt::Write failed -> %i", errno);
      return false;
    }

    return true;
  }

  bool syscall(int pid, u64 gadget, int sid, 
  u64 param_rdi, u64 param_rsi, u64 param_rdx, 
  u64 param_r10, u64 param_r8, u64 param_r9, u64* result);

  u64 mmap(void* addr, size_t len, int prot, int flags, int fd, off_t offset);
  bool mprotect(u64 addr, size_t len, int prot);
}
