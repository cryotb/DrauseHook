#pragma once

struct NetworkSessionContext_t
{
    bool received_ping;
    bool has_server_acknowledged_ping;
    
    bool received_authentication_result;
    proto::AUTH_STATES value_authentication_result;

    u32 requested_asset_id;
    
    bool received_requested_asset;
    std::vector<u8> vec_asset_contents;

    u32 requested_dlc_id;
    
    bool received_request_dlc_response;
    bool will_server_send_requested_dlc;

    bool received_dlc;
    std::vector<u8> vec_dlc_contents;
};
extern NetworkSessionContext_t* gNetSsnContext;

namespace packet_handlers
{
    extern void handle_response_to_ping(int sck, proto::Packet_header_t hdr, std::vector<u8> vec_contents);
    extern void handle_response_to_auth(int sck, proto::Packet_header_t hdr, std::vector<u8> vec_contents);
    extern void handle_response_to_request_dlc(int sck, proto::Packet_header_t hdr, std::vector<u8> vec_contents);
    extern void handle_server_drop_client(int sck, proto::Packet_header_t hdr, std::vector<u8> vec_contents);
    extern void handle_server_data_transfer(int sck, proto::Packet_header_t hdr, std::vector<u8> vec_contents);
}
