#pragma once

namespace tools
{
    bool shell_execute(const char *cmd, char *buf, size_t max_len);
    int proc_name2id(const char *name);
    void proc_suspend(int pid);
    void proc_resume(int pid);
    u64 get_mapping_base(int pid, const char *name);
    char query_virt_mem(u64 addr, hr_procmaps &out_desc);
    std::string get_current_process_path();
    bool get_self_memory_bounds(uint64_t &base, uint64_t &length);

    inline uint64_t curtick()
    {
        static auto start = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
        return elapsed.count();
    }

    inline std::vector<u8> aob_read(const std::string &filename)
    {
        std::ifstream file(filename, std::ios::binary);
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return data;
    }

    inline bool aob_write_to_disk(const char *path, void *buf, size_t len)
    {
        FILE *file = fopen(path, "wb");
        if (fwrite(buf, 1, len, file) == len)
        {
            return true;
        }
        else
        {
            return false;
        }

        fclose(file);
    }

    inline void hex_dump(const void *ptr, int buflen)
    {
        unsigned char *buf = (unsigned char *)ptr;
        int i, j;
        for (i = 0; i < buflen; i += 16)
        {
            printf("%06x: ", i);
            for (j = 0; j < 16; j++)
                if (i + j < buflen)
                    printf("%02x ", buf[i + j]);
                else
                    printf("   ");
            printf(" ");
            for (j = 0; j < 16; j++)
                if (i + j < buflen)
                    printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
            printf("\n");
        }
    }

    inline bool is_running_as_root()
    {
        uid_t euid = geteuid();
        return (euid == 0);
    }
}
