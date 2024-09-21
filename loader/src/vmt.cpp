#include "inc/include.h"

size_t vft_get_count(u64 inst)
{
  auto table = gmm->tread<u64>(inst);
  if (!table)
    return 0;

  int result = 0;

  do
  {
    auto handler = gmm->tread<u64>(table + (result * sizeof(u64)));

    if (!handler || (handler < gctx->m_game_base || handler > gctx->m_game_base + gctx->m_game_len))
      break;

    hr_procmaps mbi;
    memset(&mbi, 0, sizeof(mbi));

    // if query failed, invalid region.
    if(!tools::query_virt_mem(handler, mbi)) break;

    // if not executable, no handler.
    if( (mbi.perms & PERMS_EXECUTE) == 0) break;
    
    result++;
  } while (true);

  return result;
}

bool make_remote_vft(u64 inst, u64 &remote_vft_addr, std::vector<u64> &dump, std::vector<u64> &dump_origs, const std::initializer_list<vft_override_t> &overrides)
{
    auto vft = gmm->tread<u64>(inst);

    if (!vft)
    {
        safeLog_msg("cannot make remote vft for %p, no table is present!", inst);
        return false;
    }

    auto vft_count = vft_get_count(inst);
    auto vft_len = (vft_count * sizeof(u64));

    if (!vft_count || !vft_len)
    {
        safeLog_msg("cannot make remote vft for %p, the table is empty.", inst);
        return false;
    }

    safeLog_msg("vft of %p has %lld entries.", inst, vft_count);

    if (!vft_dump(inst, vft_count, dump))
    {
        safeLog_msg("cannot make remote vft for %p, failed dump.", inst);
        return false;
    }

    auto remote_vft = pt::mmap(0, vft_len,
                               PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (!remote_vft)
    {
        safeLog_msg("failed to alloc remote vft for %p!", inst);
        return false;
    }

    safeLog_msg("remote vft for %p -> %lx", inst, remote_vft);

    if (!gmm->fill(remote_vft, 0, vft_len))
    {
        safeLog_msg("failed to zero vft for %p!", inst);
        return false;
    }

    dump_origs = dump;

    for (auto ovr : overrides)
    {
        dump[ovr.index] = ovr.handler;
        safeLog_msg("  - overriding vft entry for %p at %i with %p", inst, ovr.index, ovr.handler);
    }

    if (!gmm->write(remote_vft, dump.data(), vft_len))
    {
        safeLog_msg("failed to write vft for %p!", inst);
        return false;
    }

    remote_vft_addr = remote_vft;

    safeLog_msg("created remote vft with success for %p", inst);

    return true;
}
