#include "inc/include.h"

Pump::Pump(int sck, unsigned char *client_pk, unsigned char *client_sk, unsigned char *server_pk)
{
    m_socket = sck;
    m_is_activated = true;

    memcpy(m_client_pk, client_pk, sizeof(m_client_pk));
    memcpy(m_client_sk, client_sk, sizeof(m_client_sk));
    memcpy(m_server_pk, server_pk, sizeof(m_server_pk));

    std::thread(&Pump::packets_read, this).detach();
    std::thread(&Pump::packets_write, this).detach();
}

void Pump::add_read_handler(u32 target_packet_id, Pump_read_callback_fn cbk)
{
    Pump_read_handler_t rec;
    memset(&rec, 0, sizeof(rec));

    rec.target_packet_id = target_packet_id;
    rec.cbk = cbk;

    m_vec_read_handlers.push_back(rec);
}

void Pump::push_packet_to_write(u32 id, const std::vector<u8> &vec_contents)
{
    Pump_queued_write_packet_t rec;
    rec.id = id;
    rec.vec_contents = vec_contents;
    m_vec_write_queue.push_back(rec);
}

void Pump::push_packet_to_write(u32 id, const void *data, u32 len)
{
    std::vector<u8> vec_temp(len);
    memcpy(vec_temp.data(), data, vec_temp.size());
    return push_packet_to_write(id, vec_temp);
}

void Pump::packets_read()
{
    while (m_is_activated)
    {
        std::vector<u8> vec_contents;
        proto::Packet_header_t hdr;
        memset(&hdr, 0, sizeof(hdr));

        if (!proto::read_packet_sealed(m_socket, hdr, vec_contents, m_client_pk, m_client_sk))
        {
            m_is_failed = true;
            dbg("[pump] read operation has failed!\n");
            break;
        }

        // printf("[PUMP-DBG] received packet with ID %i\n", hdr.id);

        for (auto handler : m_vec_read_handlers)
        {
            if (handler.target_packet_id == hdr.id)
                std::thread(handler.cbk, m_socket, hdr, vec_contents).detach();
        }

        std::this_thread::sleep_for(50ms);
    }

    m_is_activated = false;
}

void Pump::packets_write()
{
    while (m_is_activated)
    {
        if (!m_vec_write_queue.empty())
        {
            auto curr = m_vec_write_queue.front();
            if (!proto::send_packet_sealed(m_socket, curr.id, curr.vec_contents.data(),
                                           curr.vec_contents.size(), m_server_pk))
            {
                m_is_failed = true;
                dbg("[pump] write operation has failed!\n");
                break;
            }
            m_vec_write_queue.pop_front();
        }

        std::this_thread::sleep_for(50ms);
    }

    m_is_activated = false;
}
