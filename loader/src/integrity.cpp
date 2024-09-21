#include "inc/include.h"

#if defined R5I_PROD
Integrity* gInteg = new Integrity();
#else
Integrity* gInteg = nullptr;
#endif

bool Integrity::is_process_being_traced()
{
    bool result = false;

    std::string line;
    std::ifstream file("/proc/self/status");

    while (std::getline(file, line))
    {
        std::istringstream _stream(line);
        std::string tag, value;
        _stream >> tag >> value;
        
        if (tag == "TracerPid:" && value != "0")
            result = true;
    }

    return result;
}

bool Integrity::is_process_being_patched()
{
    bool result = false;

    uptr my_base = 0, my_length = 0;

    if(!tools::get_self_memory_bounds(my_base, my_length))
    {
        /* in case someone tries to tamper with some API or some shit. */
        return true;
    }

    dbg("myBase: %p, myLEn: %llx", my_base, my_length);

    if(!my_base || !my_length)
    {
        /* it could be an error, but not very likely. more likely is that someone tampers with us */
        return true;
    }

    auto image_ondisk = ELFImage(tools::get_current_process_path().c_str());
    auto pscn_text = image_ondisk.find_section(".text");

    if(!pscn_text)
    {
        /* no text section present? that is very fucking sus, at the VERY least. */
        return true;
    }

    std::vector<u8> vec_snapshot_text(pscn_text->length);
    memcpy( vec_snapshot_text.data(), PTR_OF(pscn_text->addr), vec_snapshot_text.size() );

    auto current_checksum = SSE_CRC32( vec_snapshot_text.data(), vec_snapshot_text.size(), 0 );
    if(!current_checksum)
    {
        /* this cannot possibly happen, but still check for it just in case. */
        return true;
    }

    if(!m_checksum_self_seg_text)
    {
        dbg("inited the checksum with %x", current_checksum);
        m_checksum_self_seg_text = current_checksum;
    } else
    {
        if(current_checksum != m_checksum_self_seg_text)
        {
            /* nice try */
            return true;
        }
    }

    return result;
}

Integrity::Integrity()
{
#if !defined(R5I_DISABLE_INTEGRITY_CHECKS)
    std::thread(&Integrity::check1_thread, this).detach();
    std::thread(&Integrity::check2_thread, this).detach();
#endif
}

void Integrity::check1_thread()
{
    while(true)
    {
        if(check_debugger_presence())
        {
            push_violation( proto::msg::integrity_event::IDS_VIOLATION_DETECTED_DEBUGGER_PRESENT, {} );
            break;
        }

        if(check_blacklisted_program_presence())
        {
            push_violation( proto::msg::integrity_event::IDS_VIOLATION_BLACKLISTED_PROGRAM_RUNNING, {} );
            break;
        }

        if(is_process_being_patched())
        {
            push_violation( proto::msg::integrity_event::IDS_VIOLATION_CHECKSUM_MISMATCH, {} );
            break;
        }

        std::this_thread::sleep_for(500ms);
    }

    std::this_thread::sleep_for(5s);
    terminate_safe();
}

void Integrity::check2_thread()
{
    while(true)
    {
        if(check_debugger_presence())
        {
            push_violation( proto::msg::integrity_event::IDS_VIOLATION_DETECTED_DEBUGGER_PRESENT, {} );
            break;
        }

        if(is_process_being_patched())
        {
            push_violation( proto::msg::integrity_event::IDS_VIOLATION_CHECKSUM_MISMATCH, {} );
            break;
        }

        if(check_blacklisted_program_presence())
        {
            push_violation( proto::msg::integrity_event::IDS_VIOLATION_BLACKLISTED_PROGRAM_RUNNING, {} );
            break;
        }

        std::this_thread::sleep_for(2000ms);
    }

      std::this_thread::sleep_for(5s);
    terminate_safe();
}

bool Integrity::check_debugger_presence()
{
    if(is_process_being_traced())
        return true;

    return false;
}

bool Integrity::check_blacklisted_program_presence()
{
    std::vector< std::string > vec_blacklisted_program_names = 
    {
        "ida",
        "gdb",
    };

    for(const auto& text : vec_blacklisted_program_names)
    {
        auto pid = tools::proc_name2id(text.c_str());

        if(pid != 0)
        {
            dbg("[Integrity] detected a blacklisted process currently executing: %s with PID %i", text.c_str(), pid);
            return true;
        }
    }    

    return false;
}

struct Queued_integ_record
{
    proto::msg::integrity_event::ids id;
    std::vector<u8> vec_contents;
};
std::vector< Queued_integ_record > gvQueuedIntegPackets;
void Integrity::push_violation(proto::msg::integrity_event::ids id, proto::msg::integrity_event::contents_t contents)
{
    proto::msg::integrity_event pack;
    memset(&pack, 0, sizeof(pack));

    pack.id = id;
    pack.component = proto::msg::integrity_event::COMPONENTS_LOADER;

    memcpy(&pack.contents, &contents, sizeof(pack.contents));

    auto pump = gNetSDK->pump();
    if(pump)
    {
        pump->push_packet_to_write( proto::PACKET_ID_CLIENT_INTEGRITY_EVENT, &pack, sizeof(pack) );

        if(!gvQueuedIntegPackets.empty())
        {
            for(auto rec : gvQueuedIntegPackets)
            {
                pump->push_packet_to_write( proto::PACKET_ID_CLIENT_INTEGRITY_EVENT, rec.vec_contents );
            }

            gvQueuedIntegPackets.clear( );
        }
    } else
    {
        std::vector<u8> vec_contents(sizeof(pack));
        memcpy(vec_contents.data(), &pack, vec_contents.size());

        Queued_integ_record rec{ };
        rec.id = id;
        rec.vec_contents = vec_contents;
        gvQueuedIntegPackets.push_back( rec );
    }
}

u32 Integrity::calc_self_code_checksum()
{
    uptr my_base = 0, my_length = 0;

    if(!tools::get_self_memory_bounds(my_base, my_length))
    {
        /* in case someone tries to tamper with some API or some shit. */
        return true;
    }

    dbg("myBase: %p, myLEn: %llx", my_base, my_length);

    if(!my_base || !my_length)
    {
        /* it could be an error, but not very likely. more likely is that someone tampers with us */
        return true;
    }

    auto image_ondisk = ELFImage(tools::get_current_process_path().c_str());
    auto pscn_text = image_ondisk.find_section(".text");

    if(!pscn_text)
    {
        /* no text section present? that is very fucking sus, at the VERY least. */
        return true;
    }

    std::vector<u8> vec_snapshot_text(pscn_text->length);
    memcpy( vec_snapshot_text.data(), PTR_OF(pscn_text->addr), vec_snapshot_text.size() );

    return SSE_CRC32( vec_snapshot_text.data(), vec_snapshot_text.size(), 0 );
}
