#pragma once

namespace r5::ac
{
    struct ac_record_t
    {
        const char *name;             // 0x00 - name of the 'flag'
        int tick_triggered; // 0x08 - flag trigger time or more likely count
        int unk2;           // 0x0C - unknown (2)
    };
    static_assert((sizeof(ac_record_t) == 0x10));

    struct ac_globals_t
    {
        int num_records_queued;
        int num_records_max;
        ac_record_t* records;
    };

    extern bool disarm_stack_walker();
}
