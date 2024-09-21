//
// Created by drof on 12.02.24.
//

#ifndef CLIENT_PUMP_H
#define CLIENT_PUMP_H

#include <deque>

using Pump_read_callback_fn = void(*)(int /*SCK*/, proto::Packet_header_t /*PACK_HDR*/, std::vector<u8> /*VEC_DATA*/ );
struct Pump_read_handler_t
{
    u32 target_packet_id;
    Pump_read_callback_fn cbk;
};

struct Pump_queued_write_packet_t
{
    u32 id;
    std::vector<u8> vec_contents;
};

class Pump
{
public:
    Pump() = delete;
    Pump(int sck, unsigned char* client_pk, unsigned char* client_sk, unsigned char* server_pk);

    void add_read_handler( u32 target_packet_id, Pump_read_callback_fn cbk );

    void push_packet_to_write( u32 id, const std::vector<u8>& vec_contents );
    void push_packet_to_write( u32 id, const void* data, u32 len );
private:
    void packets_read();
    void packets_write();
private:
    int m_socket;
    bool m_is_activated;
    bool m_is_failed;
    std::vector<Pump_read_handler_t> m_vec_read_handlers;
    std::deque< Pump_queued_write_packet_t > m_vec_write_queue;
    unsigned char m_client_pk[crypto_kx_PUBLICKEYBYTES], m_client_sk[crypto_kx_SECRETKEYBYTES];
    unsigned char m_server_pk[crypto_kx_PUBLICKEYBYTES];
};

#endif //CLIENT_PUMP_H
