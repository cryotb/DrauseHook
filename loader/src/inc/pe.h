#pragma once

typedef struct _IMAGE_EXPORT_DIRECTORY
{
    DWORD Characteristics;
    DWORD TimeDateStamp;
    WORD MajorVersion;
    WORD MinorVersion;
    DWORD Name;
    DWORD Base;
    DWORD NumberOfFunctions;
    DWORD NumberOfNames;
    DWORD AddressOfFunctions;    // RVA from base of image
    DWORD AddressOfNames;        // RVA from base of image
    DWORD AddressOfNameOrdinals; // RVA from base of image
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION 3
#define IMAGE_DIRECTORY_ENTRY_DEBUG 6

bool pe_dump_image(int pid, u64 base, u64 *out_len);
void pe_dump_fix(void *image);

namespace pe
{
    template <class T>
    PIMAGE_SECTION_HEADER GetEnclosingSectionHeader(
        DWORD rva,
        T *pNTHeader) // 'T' == PIMAGE_NT_HEADERS
    {
        PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION64(pNTHeader);
        unsigned i;

        for (i = 0; i < pNTHeader->FileHeader.NumberOfSections; i++, section++)
        {
            // This 3 line idiocy is because Watcom's linker actually sets the
            // Misc.VirtualSize field to 0.  (!!! - Retards....!!!)
            DWORD size = section->Misc.VirtualSize;
            if (0 == size)
                size = section->SizeOfRawData;

            // Is the RVA within this section?
            if ((rva >= section->VirtualAddress) &&
                (rva < (section->VirtualAddress + size)))
                return section;
        }

        return 0;
    }

    template <class T>
    void *get_ptr_from_va(
        DWORD rva,
        T *pNTHeader,
        u8 *imageBase) // 'T' = PIMAGE_NT_HEADERS
    {
        PIMAGE_SECTION_HEADER pSectionHdr;
        int delta;

        pSectionHdr = GetEnclosingSectionHeader(rva, pNTHeader);
        if (!pSectionHdr)
            return 0;

        delta = (int)(pSectionHdr->VirtualAddress - pSectionHdr->PointerToRawData);
        return (void *)(imageBase + rva - delta);
    }

    template <typename _pt, typename _nth>
    _pt *get_data_directory_ptr(uintptr_t base, _nth nt_hdr, int index)
    {
        const auto offset =
            nt_hdr->OptionalHeader.DataDirectory[index].VirtualAddress;

        return reinterpret_cast<_pt *>((base + offset));
    }

    template <typename _nth>
    bool get_data_directory_info(uintptr_t base, _nth nt_hdr, int index, IMAGE_DATA_DIRECTORY &out)
    {
        out = nt_hdr->OptionalHeader.DataDirectory[index];

        return true;
    }

    extern PIMAGE_SECTION_HEADER find_section_base_virt(void *image, const char *name);
    inline u64 find_export_address(uintptr_t base, PIMAGE_DOS_HEADER dos_hdr, PIMAGE_NT_HEADERS64 nt_hdr, const char *exp_name, bool file = true)
    {
        auto export_ddir = &nt_hdr->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        if (!export_ddir->VirtualAddress && !export_ddir->Size)
            return 0;

        auto export_descriptor = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(get_ptr_from_va(export_ddir->VirtualAddress, nt_hdr, (u8 *)base));

        const auto export_descriptor_rva_raw = reinterpret_cast<uintptr_t>(export_descriptor) - base;
        const auto export_descriptor_rva = get_ptr_from_va(export_descriptor_rva_raw, nt_hdr, (u8 *)base);

        const uintptr_t names_rva = export_descriptor->AddressOfNames;
        const uintptr_t addresses_rva = export_descriptor->AddressOfFunctions;
        const uintptr_t ordinals_rva = export_descriptor->AddressOfNameOrdinals;

        const auto names_address = get_ptr_from_va(static_cast<uintptr_t>(names_rva), nt_hdr, (u8 *)base);
        const auto addresses_address = get_ptr_from_va(static_cast<uintptr_t>(addresses_rva), nt_hdr, (u8 *)base);
        const auto ordinals_address = get_ptr_from_va(static_cast<uintptr_t>(ordinals_rva), nt_hdr, (u8 *)base);

        auto *const names_array = reinterpret_cast<std::uint32_t *>(names_address);
        auto *const addresses_array = reinterpret_cast<std::uint32_t *>(addresses_address);
        auto *const ordinals_array = reinterpret_cast<std::uint16_t *>(ordinals_address);

        const auto export_descriptor_rva_begin = (u64)export_descriptor_rva;
        const auto export_descriptor_rva_end = (u64)(BASE_OF(export_descriptor_rva) + export_ddir->Size);

        /*msg("export -> exportDescVA: %p", export_ddir->VirtualAddress);
        msg("export -> exportDescLN: %p", export_ddir->Size);

        msg("export -> aNames: %p", names_array);
        msg("export -> aAddrs: %p", addresses_array);
        msg("export -> aOrdinals: %p", ordinals_array);

        msg("export-> nFuncs: %i", export_descriptor->NumberOfFunctions);*/

        for (size_t i = 0; i < export_descriptor->NumberOfFunctions; i++)
        {
            auto address_virt = addresses_array[ordinals_array[i]];
            auto address = (u64)get_ptr_from_va(address_virt, nt_hdr, (u8 *)base);
            auto name = (char *)get_ptr_from_va(names_array[i], nt_hdr, (u8 *)base);

            if (strcmp(name, exp_name) == 0)
            {
                if (file)
                    return address;
                else
                    return base + address_virt;
            }
        }

        return 0;
    }

    inline uptr find_export_address_virt(uptr base, PIMAGE_DOS_HEADER dos_hdr, PIMAGE_NT_HEADERS64 nt_hdr, const char *exp_name)
    {
        auto *const export_descriptor =
            get_data_directory_ptr<IMAGE_EXPORT_DIRECTORY>(base, nt_hdr,
                                                           IMAGE_DIRECTORY_ENTRY_EXPORT);

        IMAGE_DATA_DIRECTORY export_descriptor_info{};

        if (export_descriptor == nullptr)
            return 0;
        if (!get_data_directory_info(base, nt_hdr,
                                     IMAGE_DIRECTORY_ENTRY_EXPORT, export_descriptor_info))
            return 0;
        const auto export_descriptor_rva =
            (reinterpret_cast<uptr>(export_descriptor) - base);

        const uptr names_rva = export_descriptor->AddressOfNames;
        const uptr addresses_rva = export_descriptor->AddressOfFunctions;
        const uptr ordinals_rva = export_descriptor->AddressOfNameOrdinals;

        const auto names_address = static_cast<uptr>(base + names_rva);
        const auto addresses_address = static_cast<uptr>(base + addresses_rva);
        const auto ordinals_address = static_cast<uptr>(base + ordinals_rva);

        auto *const names_array = reinterpret_cast<u32 *>(names_address);
        auto *const addresses_array = reinterpret_cast<u32 *>(addresses_address);
        auto *const ordinals_array = reinterpret_cast<u16 *>(ordinals_address);

        const auto export_descriptor_rva_begin = export_descriptor_rva;
        const auto export_descriptor_rva_end = (export_descriptor_rva + export_descriptor_info.Size);

        for (size_t i = 0; i < export_descriptor->NumberOfFunctions; i++)
        {
            const auto ordinal = ordinals_array[i];
            if (ordinal == 0)
            {
            }

            const auto address = static_cast<uptr>(base + addresses_array[i]);
            auto *const name = reinterpret_cast<char *>(base + names_array[i]);

            if(!strcmp(name, exp_name)) 
                return address;
        }

        return 0;
    }
}
