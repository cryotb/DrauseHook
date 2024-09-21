#pragma once

FORWARD_DECLARE_CLASS(Pump);

class NetSDK
{
public:
    NetSDK() = delete;
    NetSDK(const std::string_view& hostname, int port)
    {
        m_hostname = hostname;
        m_port = port;
    }

    bool start();
    bool handshake();
    bool wait_for_activation();
    bool ping();
    bool start_pump();
    bool authenticate(const std::string& email, const std::string& password);
    bool request_dlc(u32 id);
    bool request_asset(u32 id);

    void push_message_to_cloud_log(const char* contents, ...);

    auto pump() { return m_pump; }
private:
    int m_sck;
    std::string m_hostname;
    int m_port;
    unsigned char m_client_pk[crypto_kx_PUBLICKEYBYTES], m_client_sk[crypto_kx_SECRETKEYBYTES];
    unsigned char m_client_rx[crypto_kx_SESSIONKEYBYTES], m_client_tx[crypto_kx_SESSIONKEYBYTES];
    unsigned char m_server_pk[crypto_kx_PUBLICKEYBYTES];
    Pump* m_pump;
};

extern NetSDK* gNetSDK;
