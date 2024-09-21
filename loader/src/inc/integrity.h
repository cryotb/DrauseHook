#pragma once

class Integrity
{
public:
    Integrity();

    virtual void check1_thread();
    virtual void check2_thread();

    virtual bool is_process_being_traced();
    virtual bool is_process_being_patched();

    virtual bool check_debugger_presence();
    virtual bool check_blacklisted_program_presence();

    virtual void push_violation(proto::msg::integrity_event::ids id, proto::msg::integrity_event::contents_t contents);

    virtual u32 calc_self_code_checksum();
private:
    u32 m_checksum_self_seg_text;
};

extern Integrity* gInteg;
