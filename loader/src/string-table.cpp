#include "inc/include.h"

void string_table::stream_encode(uint8_t *buffer, size_t length, uint64_t key)
{
    for (size_t i = 0; i < length; i++)
    {
        buffer[i] ^= (key >> (8 * (i % 8))) & 0xFF;
    }

    rotate::encode(buffer, length);
}

void string_table::stream_decode(uint8_t *buffer, size_t length, uint64_t key)
{
    rotate::decode(buffer, length);

    for (size_t i = 0; i < length; i++)
    {
        buffer[i] ^= (key >> (8 * (i % 8))) & 0xFF;
    }
}

string_table::String_table *string_table::Create(const std::vector<std::string> &vec_contents)
{
    auto num_strings = vec_contents.size();
    auto table_alloc_base_length = sizeof(String_table) - sizeof(String_record);
    auto table_alloc_final_length = table_alloc_base_length + (sizeof(String_record) * num_strings);
    auto table = reinterpret_cast<String_table *>(malloc(table_alloc_final_length));

    if (table)
    {
        table->table_total_length = table_alloc_final_length;
        table->table_records_count = num_strings;

        for (size_t i = 0; i < num_strings; i++)
        {
            auto curr_table_record = &table->table[i];
            auto curr_string = vec_contents[i];

            curr_table_record->length = curr_string.size() + 1 /* plus 1 for null terminator */;
            curr_table_record->hash = string_hash(curr_string.c_str(), curr_string.size());
            strcpy(curr_table_record->content, curr_string.c_str());
        }

        return table;
    }

    return nullptr;
}

uint64_t gen_rand_u64()
{
    uint64_t randomNum;
    RAND_bytes(reinterpret_cast<unsigned char *>(&randomNum), sizeof(randomNum));
    return randomNum;
}

void string_table::Dump_records(String_table *table)
{
    for (uint32_t i = 0; i < table->table_records_count; i++)
    {
        auto record = table->table[i];
        msg("[S] H:%x L%llx '%s'", record.hash, record.length, record.content);
    }

    auto table_length = table->table_total_length;
}

void string_table::Dump_contents(String_table *table, size_t table_len)
{
    tools::hex_dump(table, table_len);
}

void string_table::Encrypt(String_table *table)
{
    auto key = gen_rand_u64();
    gctx->m_lctx.strtbl_decryption_key = key;

    dbg("[STRING-TABLE] encoded using key 0x%llx:", key);
    for (uint32_t i = 0; i < table->table_records_count; i++)
    {
        auto &record = table->table[i];
        stream_encode((uint8_t *)record.content, record.length, key);
    }
}
