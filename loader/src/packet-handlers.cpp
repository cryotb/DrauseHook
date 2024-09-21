#include "inc/include.h"

NetworkSessionContext_t *gNetSsnContext = nullptr;

void packet_handlers::handle_response_to_ping(int sck, proto::Packet_header_t hdr, std::vector<u8> vec_contents)
{
    auto presponse = reinterpret_cast<proto::rsp::Client_Ping_t *>(vec_contents.data());

    if (!presponse->ack)
    {
        gNetSsnContext->has_server_acknowledged_ping = false;
        gNetSsnContext->received_ping = true;
        return;
    }

    gNetSsnContext->has_server_acknowledged_ping = true;
    gNetSsnContext->received_ping = true;
}

void packet_handlers::handle_response_to_auth(int sck, proto::Packet_header_t hdr, std::vector<u8> vec_contents)
{
    auto result = *reinterpret_cast<uint32_t *>(vec_contents.data());

    gNetSsnContext->value_authentication_result = (proto::AUTH_STATES)result;
    gNetSsnContext->received_authentication_result = true;
}

void packet_handlers::handle_server_drop_client(int sck, proto::Packet_header_t hdr, std::vector<u8> vec_contents)
{
    auto presponse = reinterpret_cast<proto::msg::Server_drop_client_t *>(vec_contents.data());

    if (strlen(presponse->reason) > 0)
    {
        printf("dropped: %s\n", presponse->reason);
    }

    close(sck);
}

void packet_handlers::handle_server_data_transfer(int sck, proto::Packet_header_t hdr, std::vector<u8> vec_contents)
{
    auto dbuf = new DrauseDataBuf(vec_contents.data(), vec_contents.size());
    if (dbuf)
    {
        auto id = dbuf->read<u32>();
        auto length_compressed = dbuf->read<size_t>();
        auto length_decompressed = dbuf->read<size_t>();

        if (length_compressed > 0 && length_decompressed > 0)
        {
            std::vector<u8> vec_dlc_contents(length_compressed );
            dbuf->read_data(vec_dlc_contents.data(), vec_dlc_contents.size());

            std::vector<u8> vec_dlc_contents_decompressed(length_decompressed );
            vec_dlc_contents_decompressed = lzma::decompress(vec_dlc_contents);

            rotate::decode(vec_dlc_contents_decompressed.data(), vec_dlc_contents_decompressed.size() );
            //tools::hex_dump(vec_dlc_contents_decompressed.data(), 0x1000 );

            if(id == gNetSsnContext->requested_dlc_id)
            {
                gNetSsnContext->vec_dlc_contents = vec_dlc_contents_decompressed;
                gNetSsnContext->received_dlc = true;
            } else if(id == gNetSsnContext->requested_asset_id)
            {
                gNetSsnContext->vec_asset_contents = vec_dlc_contents_decompressed;
                gNetSsnContext->received_requested_asset = true;
            } else
            {
                dbg("unknown file transfer received, with ID %x", id);
            }
        }

        delete dbuf;
    }
}

void packet_handlers::handle_response_to_request_dlc(int sck, proto::Packet_header_t hdr, std::vector<u8> vec_contents)
{
    auto dbuf = new DrauseDataBuf(vec_contents.data(), vec_contents.size());
    if (!dbuf)
    {
        return;
    }

    bool will_send = dbuf->read<bool>();
    gNetSsnContext->will_server_send_requested_dlc = will_send;
    gNetSsnContext->received_request_dlc_response = true;
}
