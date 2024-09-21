#pragma once

#pragma pack(push, 1)

struct LoaderContext
{
    unsigned int magic;
    u8 inited;
    /*
     * Keep this here and if you change anything above,
     *  then ENSURE to update our trampoline offsets!
    */
    struct Origs
    {
        u64 init;
    } origs;
    u64 game_base;
    u64 game_len;
    u64 game_rdata_base;
    u64 game_rdata_len;
    u64 game_cs_base;
    u64 game_cs_len;
    u64 game_data_base;
    u64 game_data_len;
    u64 libc_base;
    u64 libm_base;
    String_table* strtbl;
    u64 strtbl_decryption_key;

    struct
    {
        uint32_t fopen;
        uint32_t fputs;
        uint32_t fclose;
        uint32_t sprintf;
        uint32_t sleep;
    } linuxcrt_cpp;

    struct
    {
        uint32_t acosf;
        uint32_t atan2f;
        uint32_t fabsf;
        uint32_t sinf;
        uint32_t cosf;
    } linuxcrt_math;

    struct
    {
        uptr create_file;
        uptr get_file_size;
        uptr close_handle;
        uptr read_file;
        uptr capture_stack_back_trace;
    } winapis;

    struct Offsets
    {
        uint32_t entity_list;
        uint32_t signon_state;
        uint32_t input_system;
        uint32_t mp_gamemode;
        uint32_t global_vars;
        uint32_t game_movement;
        uint32_t map_name;
        uint32_t name_list;
        uint32_t tag_list;
        uint32_t client_class_head;
        uint32_t mat_system_surface;
        uint32_t base_client;
        uint32_t view_render;
        uint32_t client_state;
        uint32_t client_mode;
        uint32_t kwewed_render_ctx;
        uint32_t rs_cvar_gamepad_checks_enabled;
        uint32_t rs_gamepad_checks_last;
        uint32_t string_dict_user_names;

        struct
        {
            uint32_t view_offset;
            uint32_t view_angle;
        } player;

        struct
        {
            uint32_t get_mem_alloc;
            uint32_t mss_draw_filled_rect;
            uint32_t math_ang2vec;
            uint32_t math_vec2ang;
        } fns;

        struct
        {
            uint32_t generic_1;
        } vft_caves;

        struct
        {
            u64 mkrctx_unk1;
            u64 create_move;
            u64 override_mouse;
        } ret_addrs;
    } offsets;

    struct
    {
        u64 base_client;
    } vft_orig_ptrs;
};
#pragma pack(pop)