#pragma once

inline u64 vft_get_handler(u64 inst, int idx)
{
  auto vft_base = gmm->tread<u64>(inst);
  if (!vft_base)
    return 0;

  return gmm->tread<u64>(vft_base + (idx * 8));
}

inline bool vft_set_handler(int pid, u64 inst, int idx, u64 value)
{
  auto vft_base = gmm->tread<u64>(inst);
  if (!vft_base)
    return 0;

  return pt::write(pid, vft_base + (idx * 8), value);
}

size_t vft_get_count(u64 inst);

inline bool vft_dump(u64 inst, size_t count, std::vector<u64> &buf)
{
  auto table = gmm->tread<u64>(inst);
  if (!table)
    return false;

  buf.resize(count);

  size_t index = 0;

  while(index < count)
  {
    buf[index] = gmm->tread<u64>(table + (index * sizeof(u64)));

    index++;
  }

  return index == count;
}

struct vft_override_t
{
    u64 index;
    u64 handler;
};
bool make_remote_vft(u64 inst, u64 &remote_vft_addr, std::vector<u64> &dump, std::vector<u64> &dump_origs, const std::initializer_list<vft_override_t> &overrides);