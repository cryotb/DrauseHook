#include "inc/include.h"

NetSDK *gNetSDK = nullptr;

bool NetSDK::start()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        return false;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(m_port);
    inet_pton(AF_INET, m_hostname.c_str(), &serverAddress.sin_addr);

    if (connect(clientSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        close(clientSocket);
        return false;
    }

    m_sck = clientSocket;

    return true;
}

bool NetSDK::handshake()
{
    proto::Packet_header_t hdr;
    std::vector<uint8_t> contents;
    if (!proto::read_packet(m_sck, hdr, contents))
        return false;

    auto pack = reinterpret_cast<proto::msg::Server_hello_t *>(contents.data());

    memcpy(m_server_pk, pack->public_key, sizeof(m_server_pk));

    crypto_kx_keypair(m_client_pk, m_client_sk);

    if (crypto_kx_client_session_keys(m_client_rx, m_client_tx,
                                      m_client_pk, m_client_sk, pack->public_key) != 0)
    {
        msg("networking error: CE001.");
        return false;
    }

    proto::msg::Client_hello_t hello;
    memset(&hello, 0, sizeof(hello));
    hello.version = 100;
    memcpy(hello.public_key, m_client_pk, sizeof(hello.public_key));
    strcpy(hello.compilation_aes_key, STRINGIFIED_DEF(_LOADER_COMPILATION_AES_KEY));
    strcpy(hello.build_timestamp, STRINGIFIED_DEF(_BUILD_TIMESTAMP));
    if (!proto::send_packet(m_sck, proto::PACKET_ID_CLIENT_HELLO, &hello, sizeof(hello)))
    {
        msg("networking error: CE002.");
        return false;
    }

    return true;
}

bool NetSDK::wait_for_activation()
{
    proto::Packet_header_t hdr;
    std::vector<uint8_t> vec_contents;
    if (proto::read_packet_sealed(m_sck, hdr, vec_contents, m_client_pk, m_client_sk))
    {
        // tools::hex_dump(vec_contents.data(), vec_contents.size());

        if (strcmp((char *)vec_contents.data(), "activated!") == 0)
        {
            return true;
        } else
        {
            printf("%s\n", (char *)vec_contents.data());
        }
    }

    return false;
}

bool NetSDK::ping()
{
    proto::msg::Client_Ping_t pack;
    pack.syn = true;
    pack.tick = tools::curtick();
    m_pump->push_packet_to_write(proto::PACKET_ID_CLIENT_PING, &pack, sizeof(pack));
    return true;
}

bool NetSDK::start_pump()
{
    if(!gNetSsnContext)
    {
        gNetSsnContext = new NetworkSessionContext_t();
    }
    
    m_pump = new Pump(m_sck, m_client_pk, m_client_sk, m_server_pk);
    if (!m_pump)
        return false;

    m_pump->add_read_handler(proto::PACKET_ID_SERVER_RESPONSE_TO_PING, packet_handlers::handle_response_to_ping);
    m_pump->add_read_handler(proto::PACKET_ID_SERVER_RESPONSE_TO_AUTH, packet_handlers::handle_response_to_auth);
    m_pump->add_read_handler(proto::PACKET_ID_SERVER_RESPONSE_TO_REQUEST_DLC, packet_handlers::handle_response_to_request_dlc);
    m_pump->add_read_handler(proto::PACKET_ID_SERVER_DROP_CLIENT, packet_handlers::handle_server_drop_client);
    m_pump->add_read_handler(proto::PACKET_ID_SERVER_DATA_TRANSFER, packet_handlers::handle_server_data_transfer);

    return true;
}

static inline void native_cpuid(unsigned int *eax, unsigned int *ebx,
                                unsigned int *ecx, unsigned int *edx)
{
        /* ecx is often an input as well as an output. */
        asm volatile("cpuid"
            : "=a" (*eax),
              "=b" (*ebx),
              "=c" (*ecx),
              "=d" (*edx)
            : "0" (*eax), "2" (*ecx));
}

bool NetSDK::authenticate(const std::string &email, const std::string &password)
{
    if(!gHwid->m_available)
    {
        error("internal error: AW1267");
        return false;
    }

    auto dbuf = new DrauseDataBuf();
    if (!dbuf)
        return false;

    dbuf->write_string(email);
    dbuf->write_string(password);

    // TODO: Add HWID down here.
    dbuf->write<uint32_t>(gHwid->m_vec_disks.size());
    dbuf->write<uint32_t>(gHwid->m_vec_device_macs.size());

    // construct the list of DISKS first.
    for (const auto &rec : gHwid->m_vec_disks)
    {
        dbuf->write_string(rec.serial_number); // SERIAL
        dbuf->write_string(rec.model_name);   // VERSION_OFFSET
        char dummy[40];
        memset(dummy, 0, sizeof(dummy));
        dbuf->write_data(dummy, sizeof(dummy));
    }

    // afterwards we need to construct the list of NICS to ensure data completeness.
    for (const auto &rec : gHwid->m_vec_device_macs)
    {
        dbuf->write_string(rec.device);           // MAC_DESC
        dbuf->write_string(rec.mac_address_permanent); // MAC_CURR
        dbuf->write_string(rec.mac_address_permanent); // MAC_PERM
    }

    /* additionally we can collect some VM checks related data. */

    // CPUID Capabilities and CPU Info

    unsigned int cpuid_regs[4] = {0, 0, 0, 0};

    memset(cpuid_regs, 0, sizeof(cpuid_regs));
    cpuid_regs[0] = 0; /* processor vendor */
    native_cpuid(&cpuid_regs[0], &cpuid_regs[1], &cpuid_regs[2], &cpuid_regs[3]);
    dbuf->write_data(cpuid_regs, sizeof(cpuid_regs));
    
    memset(cpuid_regs, 0, sizeof(cpuid_regs));
    cpuid_regs[0] = 1; /* processor info and feature bits */
    native_cpuid(&cpuid_regs[0], &cpuid_regs[1], &cpuid_regs[2], &cpuid_regs[3]);
    dbuf->write_data(cpuid_regs, sizeof(cpuid_regs));

    // Firmware Tables of SMBIOS
    auto vec_smbios_dump = gHwid->dump_smbios_data();
    if(vec_smbios_dump.empty())
    {
        error("internal error: AW1268");
        return false;
    }
    
    dbuf->write<size_t>(vec_smbios_dump.size());
	if (!vec_smbios_dump.empty())
	{
		dbuf->write_data_vec(vec_smbios_dump);
	}

    auto vec_hwid_buffer = dbuf->tovec();

    m_pump->push_packet_to_write(proto::PACKET_ID_CLIENT_AUTH, vec_hwid_buffer.data(), vec_hwid_buffer.size());
    delete dbuf;

    return true;
}

bool NetSDK::request_dlc(u32 id)
{
    proto::msg::Request_Dlc_t req;
    memset(&req, 0, sizeof(req));
    req.id = id;
    req.remote_base = 0;

    gNetSsnContext->requested_dlc_id = id;
    m_pump->push_packet_to_write(proto::PACKET_ID_CLIENT_REQUEST_DLC, &req, sizeof(req));
    return true;
}

bool NetSDK::request_asset(u32 id)
{
    gNetSsnContext->requested_asset_id = id;
    m_pump->push_packet_to_write(proto::PACKET_ID_CLIENT_REQUEST_ASSET, &id, sizeof id);

    return true;
}

#define CLOUD_LOG_MAX_MESSAGE_BUFFER_LENGTH 2048
#define CLOUD_LOG_MAX_MESSAGE_LENGTH (CLOUD_LOG_MAX_MESSAGE_BUFFER_LENGTH-100)
void NetSDK::push_message_to_cloud_log(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    va_list args_copy;
    va_copy(args_copy, args);
    int len = vsnprintf(nullptr, 0, fmt, args_copy) + 1;
    va_end(args_copy);

    char *buffer = new char[len];

    vsnprintf(buffer, len, fmt, args);

    if(strlen(buffer) > CLOUD_LOG_MAX_MESSAGE_LENGTH || len > CLOUD_LOG_MAX_MESSAGE_BUFFER_LENGTH)
        return;

    proto::msg::cloud_log_push_message_t req;
    memset(&req, 0, sizeof(req));

    req.severity = 0; // 0 - MESSAGE, 1 - WARNING, 2 - ERROR, 3 - CRITICAL
    req.bucket_id = 0x6523;
    strcpy(req.contents, buffer);

    delete[] buffer;
    va_end(args);

    m_pump->push_packet_to_write(proto::PACKET_ID_CLIENT_PUSH_MESSAGE_TO_CLOUD_LOG, &req, sizeof(req));
}
