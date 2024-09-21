#pragma once

class Hwid
{
public:
    struct Disk_t
    {
        std::string model_name;
        std::string serial_number;
    };

    struct ArpParticipant_t
    {
        std::string mac_address_permanent;
        std::string device;
    };

public:
    Hwid();
    bool collect();
    bool query_all_disks();
    bool query_arp();

    bool query_disk_identity(const char *dev, hd_driveid &buffer);
    bool query_nvme_identity(const char* dev, Disk_t& buffer);
    std::vector<uint8_t> dump_smbios_data();

    bool m_available;
    std::vector<Disk_t> m_vec_disks;
    std::vector<ArpParticipant_t> m_vec_device_macs;
};

extern Hwid *gHwid;