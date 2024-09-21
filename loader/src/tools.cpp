#include "inc/include.h"
#include <atomic>

class SpinLock {
public:
    SpinLock() : lock_(false) {}

    void lock() {
        while (lock_.exchange(true, std::memory_order_acquire)) {
            std::this_thread::yield();
        }
    }

    void unlock() {
        lock_.store(false, std::memory_order_release);
    }

private:
    std::atomic<bool> lock_;
};

SpinLock lck_shell_execute;

bool tools::shell_execute(const char *cmd, char *buf, size_t max_len)
{
    lck_shell_execute.lock();

    size_t bytes_read = 0;
    char intermediate[16];
    memset(intermediate, 0, sizeof(intermediate));
    FILE *file = popen(cmd, "r");
    if (file)
    {
        while (bytes_read < max_len && fgets(intermediate, sizeof(intermediate), file) != 0)
        {
            sprintf(buf + strlen(buf), "%s", intermediate);
            memset(intermediate, 0, sizeof(intermediate));
            bytes_read += sizeof(intermediate);
        }
        pclose(file);
    }

    lck_shell_execute.unlock();

    return (bytes_read > 0);
}

int tools::proc_name2id(const char *name)
{
    // pgrep <proc_name>
    const char *fmt = "pgrep %s";
    char cmd[sizeof(fmt) + 32];
    char rs[33];

    memset(cmd, 0, sizeof(cmd));
    memset(rs, 0, sizeof(rs));

    sprintf(cmd, fmt, name);
    if(!shell_execute(cmd, rs, sizeof(rs)-1))
        return 0;
        
    int pid = 0;
    sscanf(rs, "%d", &pid);
    //msg("[dbg] %s", rs);

    return pid;
}

void tools::proc_suspend(int pid)
{
  const char* fmt = "kill -STOP %i";
  char cmd[sizeof(fmt) + 32];
  char rs[32+1];

  memset(cmd, 0, sizeof(cmd));
  memset(rs, 0, sizeof(rs));
  
  sprintf(cmd, fmt, pid);
  
  if(!shell_execute(cmd, rs, sizeof(rs)-1))
    return;
}

void tools::proc_resume(int pid)
{
  const char* fmt = "kill -CONT %i";
  char cmd[sizeof(fmt) + 32];
  char rs[32+1];

  memset(cmd, 0, sizeof(cmd));
  memset(rs, 0, sizeof(rs));
  
  sprintf(cmd, fmt, pid);
  
  if(!shell_execute(cmd, rs, sizeof(rs)-1))
    return;
}

u64 tools::get_mapping_base(int pid, const char* name)
{
    uint64_t        base = 0;
    const char*     fmt = "cat /proc/%d/maps | grep %s";
    char            cmd[  sizeof(fmt) + 32  ];
    char            rs[256];

    memset(cmd, 0, sizeof(cmd));
    memset(rs, 0, sizeof(rs));

    sprintf(cmd, fmt, pid, name);
    shell_execute(cmd, rs, sizeof(rs));        
    sscanf(rs, "%lx", &base);

    return base;
}

char tools::query_virt_mem(u64 addr, hr_procmaps &out_desc)
{
    procmaps_begin_transaction();

    char result = 0;
    auto pmap = contruct_procmaps(gctx->m_pid);

    if (pmap)
    {
        for (auto ptr = pmap; ptr != nullptr; ptr++)
        {
            auto desc = ptr[0];
            if (!desc)
                break;
            if (addr >= desc->addr_begin && addr < desc->addr_end)
            {
                out_desc = *desc;
                result = 1;
                break;
            }
        }

        destroy_procmaps(pmap);
    }

    procmaps_end_transaction();

    return result;
}

std::string tools::get_current_process_path()
{
    char path[1024];
    ssize_t length = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (length == -1) {
        return ""; // Return an empty string in case of failure
    }
    path[length] = '\0';  // Null-terminate the string

    return std::string(path);
}

bool tools::get_self_memory_bounds(uint64_t &base, uint64_t &length)
{
    procmaps_begin_transaction();

    char result = 0;
    auto pmap = contruct_procmaps(getpid());

    if (pmap)
    {
        uintptr_t last_mapping_occurrence_addr = 0;
        auto my_path = get_current_process_path();
        if(!my_path.empty())
        {
            for (auto ptr = pmap; ptr != nullptr; ptr++)
            {
                auto desc = ptr[0];
                if (!desc)
                    break;

                if(desc->pathname != nullptr)
                {
                    if( std::string(desc->pathname) == my_path)
                    {
                        if(!base)
                        {
                            base = desc->addr_begin;
                        } else
                        {
                            // base has already been inited, count now.
                            if(!last_mapping_occurrence_addr || desc->addr_end > last_mapping_occurrence_addr)
                            {
                                last_mapping_occurrence_addr = desc->addr_end;
                            }
                        }
                    }
                }
            }
        }

        length = (last_mapping_occurrence_addr - base);
        if(length > 0)
            result = true;

        destroy_procmaps(pmap);
    }

    procmaps_end_transaction();

    return result;
}
