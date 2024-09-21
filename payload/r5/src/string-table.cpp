#include "inc/include.h"

void stream_decode(uint8_t *buffer, size_t length, uint64_t key)
{
    rotate::decode(buffer, length);

    for (size_t i = 0; i < length; i++)
    {
        buffer[i] ^= (key >> (8 * (i % 8))) & 0xFF;
    }
}

void stream_encode(uint8_t *buffer, size_t length, uint64_t key)
{
    for (size_t i = 0; i < length; i++)
    {
        buffer[i] ^= (key >> (8 * (i % 8))) & 0xFF;
    }

    rotate::encode(buffer, length);
}

String_record *str_from_hash(uint32_t hash)
{
    auto table = gctx->strtbl;

    for (uint32_t i = 0; i < table->table_records_count; i++)
    {
        auto record = &table->table[i];
        if (record->hash == hash)
        {
            return record;
        }
    }

    return nullptr;
}

Crypted_string::Crypted_string(uint32_t hash)
{
    auto in_table_ptr = str_from_hash(hash);
    if (hash)
    {
        m_length = in_table_ptr->length;
        memcpy(m_contents, in_table_ptr->content, m_length);
    }
}

const char *Crypted_string::decrypt()
{
    stream_decode((u8 *)this->m_contents, this->m_length, gctx->strtbl_decryption_key );
    return this->m_contents;
}
