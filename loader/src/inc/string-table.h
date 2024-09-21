#pragma once

namespace string_table
{
    extern void stream_encode(uint8_t *buffer, size_t length, uint64_t key);
    extern void stream_decode(uint8_t *buffer, size_t length, uint64_t key);

#define STRREC_MAX_CONTENT_LENGTH 64

    struct String_record
    {
        uint32_t length;
        uint32_t hash;
        char content[STRREC_MAX_CONTENT_LENGTH];
    } __attribute__((packed));

    struct String_table
    {
        uint32_t magic;
        uint32_t table_total_length;
        uint32_t table_records_count;
        String_record table[1];
    } __attribute__((packed));

    extern String_table *Create(const std::vector<std::string> &vec_contents);
    extern void Dump_records(String_table *table);
    extern void Dump_contents(String_table *table, size_t table_len);

    extern void Encrypt(String_table* table);
}
