#include "inc/include.h"

#if defined(R5I_PROD)
Hwid *gHwid = new Hwid();
#else
Hwid *gHwid = nullptr;
#endif

class File_Descriptor_Guard
{
public:
    File_Descriptor_Guard() = delete;
    File_Descriptor_Guard(int fd)
    {
        m_fd = fd;
    }
    ~File_Descriptor_Guard()
    {
        close(m_fd);
    }

private:
    int m_fd;
};

Hwid::Hwid()
{
    m_available = collect();
}

bool Hwid::collect()
{
    if (!query_all_disks())
        return false;

    if(!query_arp())
        return false;

    if(this->m_vec_device_macs.empty())
    {
        error("HWE001");
        return false;
    }

    if(this->m_vec_disks.empty())
    {
        error("HWE002");
        return false;
    }

    return true;
}

bool Hwid::query_all_disks()
{
    const char *prefix_ssd = "/dev/sd";
    const char* prefix_nvme_device = "/dev/nvme";

    const char letters[] =
        {
            'a',
            'b',
            'c',
            'd',
            'e',
            'f',
        };

    for (int i = 0; i < 6; i++)
    {
        char fmt[32];
        memset(fmt, 0, sizeof(fmt));
        sprintf(fmt, "%s%c", prefix_ssd, letters[i]);

        hd_driveid identity;
        memset(&identity, 0, sizeof(identity));

        if (!query_disk_identity(fmt, identity))
        {
            continue;
        }

        Disk_t rec{};
        rec.model_name.resize(sizeof(identity.model));
        rec.serial_number.resize(sizeof(identity.serial_no));
        sprintf(rec.model_name.data(), "%.40s", identity.model);
        sprintf(rec.serial_number.data(), "%.20s", identity.serial_no);
        m_vec_disks.push_back(rec);
    }

    for (int i = 0; i < 6; i++)
    {
        char fmt[32];
        memset(fmt, 0, sizeof(fmt));
        sprintf(fmt, "%s%in1", prefix_nvme_device, i);

        Disk_t rec{};

        if (!query_nvme_identity(fmt, rec))
        {
            continue;
        }
        m_vec_disks.push_back(rec);
    }

    return true;
}

bool Hwid::query_arp()
{
    char line[512];     
    char ip_address[256];
    int hw_type;
    int flags;
    char mac_address[64];
    char mask[64];
    char device[64];

    FILE *fp = fopen("/proc/net/arp", "r");
    if (fp)
    {
        fgets(line, sizeof(line), fp);
        while (fgets(line, sizeof(line), fp))
        {
            sscanf(line, "%s 0x%x 0x%x %s %s %s\n",
                   ip_address,
                   &hw_type,
                   &flags,
                   mac_address,
                   mask,
                   device);

            ArpParticipant_t record;
            record.device = ip_address;
            record.mac_address_permanent = mac_address;
            m_vec_device_macs.push_back(record);
        }
    } else
    {
        return false;
    }

    fclose(fp);
    return true;
}

bool Hwid::query_disk_identity(const char *dev, hd_driveid &buffer)
{
    static struct hd_driveid hd;
    int fd;

    if ((fd = open(dev, O_RDONLY | O_NONBLOCK)) < 0)
    {
        return false;
    }

    File_Descriptor_Guard _(fd);

    if (!ioctl(fd, HDIO_GET_IDENTITY, &hd))
    {
        memcpy(&buffer, &hd, sizeof(buffer));
    }
    else
    {
        return false;
    }

    return true;
}

bool Hwid::query_nvme_identity(const char *dev, Disk_t &buffer)
{
    int fd = open(dev, O_RDWR);
    if (fd == 0 || fd < 0)
    {
        return false;
    }

    File_Descriptor_Guard _(fd);

    char buf[4096] = {0};
    struct nvme_admin_cmd mib = {0};
    mib.opcode = 0x06; // identify
    mib.nsid = 0;
    mib.addr = (__u64)buf;
    mib.data_len = sizeof(buf);
    mib.cdw10 = 1; // controller

    int ret = ioctl(fd, NVME_IOCTL_ADMIN_CMD, &mib);
    if (ret)
    {
        return false;
    }

    buffer.serial_number.resize(21);
    snprintf(buffer.serial_number.data(), 21, "%.20s", &buf[4]);

    buffer.model_name.resize(41);
    snprintf(buffer.model_name.data(), 41, "%.40s", &buf[24]);

    return true;
}

#define SMBIOS_START_ADDRESS 0xE0000 // Start address of SMBIOS data in physical memory
#define SMBIOS_END_ADDRESS   0xFFFFF // End address of SMBIOS data in physical memory
#define BUFFER_SIZE 16 // Size of buffer for reading from /dev/mem
std::vector<uint8_t> hwid::dump_smbios_data()
{
    std::vector<uint8_t> smbios_data;
    unsigned char buffer[BUFFER_SIZE];

    // Open /dev/mem for reading
    int fd = open("/dev/mem", O_RDONLY);
    if (fd < 0) {
        return smbios_data;
    }

    // Seek to the start of SMBIOS data
    if (lseek(fd, SMBIOS_START_ADDRESS, SEEK_SET) < 0) {
        return smbios_data;
    }

    // Read and dump SMBIOS data until SMBIOS_END_ADDRESS is reached
    while (lseek(fd, 0, SEEK_CUR) < SMBIOS_END_ADDRESS) {
        ssize_t bytesRead = read(fd, buffer, BUFFER_SIZE);
        if (bytesRead < 0) {
           return smbios_data;
        }

        // Append the read buffer to the vector
        smbios_data.insert(smbios_data.end(), buffer, buffer + bytesRead);
    }

    // Close /dev/mem
    close(fd);

    return smbios_data;
}
