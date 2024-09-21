#pragma once

struct ELFSection
{
    ELFSection() = delete;
    ELFSection(const std::string& name, uptr addr, u32 len)
    {
        this->name = name;
        this->addr = addr;
        this->length = len;
    }

    std::string name;
    uptr addr;
    u32 length;
};

struct ELFSymbol
{
    ELFSymbol() = delete;
    ELFSymbol(const std::string &name, uptr addr)
    {
        this->name = name;
        this->addr = addr;
    }

    std::string name;
    uptr addr;
};

class ELFImage
{
public:
    ELFImage() = delete;
    ELFImage(const char *path)
    {
        _image = tools::aob_read(path);

        populate_sections();
        populate_symbols();
    }

    ELFImage(const std::vector<u8>& vec_contents)
    {
        _image = vec_contents;

        populate_sections();
        populate_symbols();
    }

    void populate_sections()
    {
        Elf *elf;
        Elf_Scn *scn = nullptr;
        GElf_Shdr shdr;

        elf_version(EV_CURRENT);
        elf = elf_memory(reinterpret_cast<char *>(_image.data()), _image.size());

        size_t shstrndx;
        elf_getshdrstrndx(elf, &shstrndx); // Get the section string table index

        while ((scn = elf_nextscn(elf, scn)) != nullptr)
        {
            gelf_getshdr(scn, &shdr);

            if (shdr.sh_type == SHT_PROGBITS && shdr.sh_flags == (SHF_ALLOC | SHF_EXECINSTR))
            {
                const char *section_name = elf_strptr(elf, shstrndx, shdr.sh_name);
                if (strcmp(section_name, ".text") == 0)
                {
                    // Obtain the address and size of the .text section
                    Elf64_Addr section_address = shdr.sh_addr;
                    Elf64_Xword section_size = shdr.sh_size;
                    // Use the text_section_address and text_section_size as needed
                    _sections.push_back( ELFSection(section_name, section_address, section_size) );
                    break;
                }
            }
        }

        elf_end(elf);
    }

    void populate_symbols()
    {
        Elf *elf;
        Elf_Scn *scn = nullptr;
        Elf_Data *data = nullptr;
        GElf_Sym sym;
        size_t num_symbols, i;

        elf_version(EV_CURRENT);
        elf = elf_memory(reinterpret_cast<char *>(_image.data()), _image.size());

        while ((scn = elf_nextscn(elf, scn)) != nullptr)
        {
            GElf_Shdr shdr;
            gelf_getshdr(scn, &shdr);

            if (shdr.sh_type == SHT_SYMTAB || shdr.sh_type == SHT_DYNSYM)
            {
                data = elf_getdata(scn, data);
                num_symbols = shdr.sh_size / shdr.sh_entsize;

                for (i = 0; i < num_symbols; i++)
                {
                    gelf_getsym(data, i, &sym);

                    const char *symbol_name = elf_strptr(elf, shdr.sh_link, sym.st_name);
                    Elf64_Addr symbol_address = sym.st_value;
                    _symbols.push_back(ELFSymbol(symbol_name, symbol_address));
                }
            }
        }

        elf_end(elf);
    }

    uintptr_t find_symbol(const char *name)
    {
        for (const auto &sym : _symbols)
        {
            if (sym.name == name)
            {
                return sym.addr;
            }
        }

        return 0;
    }

    auto find_section(const char* name) -> const ELFSection* const
    {
        for(const auto& scn : _sections)
        {
            if(scn.name == name)
            {
                return &scn;
            }
        }

        return nullptr;
    }

    auto buf() { return _image.data(); }
private:
    std::vector<u8> _image;
    std::vector<ELFSection> _sections;
    std::vector<ELFSymbol> _symbols;
};

