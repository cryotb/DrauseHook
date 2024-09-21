#pragma once

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

extern String_record* str_from_hash(uint32_t hash);

#define hash2str(HASH) Crypted_string( _encoded_const(HASH) ).decrypt()

class Crypted_string
{
public:
    Crypted_string() = delete;
    Crypted_string(uint32_t hash);

    const char* decrypt();
private:
    uint32_t m_length;
    char m_contents[STRREC_MAX_CONTENT_LENGTH];
};

