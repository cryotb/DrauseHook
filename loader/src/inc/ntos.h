#pragma once

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16
#define IMAGE_DOS_SIGNATURE 0x5A4D
#define IMAGE_NT_SIGNATURE 0x4550
#define IMAGE_SIZEOF_SHORT_NAME 8

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef int32_t LONG;
typedef uint32_t DWORD;
typedef unsigned long long ULONGLONG;
typedef unsigned short int USHORT;

typedef struct _IMAGE_BASE_RELOCATION
{
  DWORD VirtualAddress;
  DWORD SizeOfBlock;
  //  WORD    TypeOffset[1];
} __attribute__((aligned(1), packed)) IMAGE_BASE_RELOCATION;
typedef IMAGE_BASE_RELOCATION UNALIGNED, *PIMAGE_BASE_RELOCATION;

typedef struct BASE_RELOCATION_ENTRY
{
  USHORT Offset : 12;
  USHORT Type : 4;
} __attribute__((aligned(1), packed)) BASE_RELOCATION_ENTRY, *PBASE_RELOCATION_ENTRY;

typedef struct _IMAGE_DOS_HEADER
{
  WORD e_magic;
  WORD e_cblp;
  WORD e_cp;
  WORD e_crlc;
  WORD e_cparhdr;
  WORD e_minalloc;
  WORD e_maxalloc;
  WORD e_ss;
  WORD e_sp;
  WORD e_csum;
  WORD e_ip;
  WORD e_cs;
  WORD e_lfarlc;
  WORD e_ovno;
  WORD e_res[4];
  WORD e_oemid;
  WORD e_oeminfo;
  WORD e_res2[10];
  LONG e_lfanew;
} __attribute__((aligned(1), packed)) IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

typedef struct _IMAGE_FILE_HEADER
{
  WORD Machine;
  WORD NumberOfSections;
  DWORD TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD SizeOfOptionalHeader;
  WORD Characteristics;
} __attribute__((aligned(1), packed)) IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY
{
  DWORD VirtualAddress; // RVA of the data
  DWORD Size;           // Size of the data
} __attribute__((aligned(1), packed)) IMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_DEBUG_DIRECTORY {
  DWORD Characteristics;
  DWORD TimeDateStamp;
  WORD  MajorVersion;
  WORD  MinorVersion;
  DWORD Type;
  DWORD SizeOfData;
  DWORD AddressOfRawData;
  DWORD PointerToRawData;
} __attribute__((aligned(1), packed)) IMAGE_DEBUG_DIRECTORY, *PIMAGE_DEBUG_DIRECTORY;

typedef struct _IMAGE_OPTIONAL_HEADER64
{
  WORD Magic;
  BYTE MajorLinkerVersion;
  BYTE MinorLinkerVersion;
  DWORD SizeOfCode;
  DWORD SizeOfInitializedData;
  DWORD SizeOfUninitializedData;
  DWORD AddressOfEntryPoint;
  DWORD BaseOfCode;
  ULONGLONG ImageBase;
  DWORD SectionAlignment;
  DWORD FileAlignment;
  WORD MajorOperatingSystemVersion;
  WORD MinorOperatingSystemVersion;
  WORD MajorImageVersion;
  WORD MinorImageVersion;
  WORD MajorSubsystemVersion;
  WORD MinorSubsystemVersion;
  DWORD Win32VersionValue;
  DWORD SizeOfImage;
  DWORD SizeOfHeaders;
  DWORD CheckSum;
  WORD Subsystem;
  WORD DllCharacteristics;
  ULONGLONG SizeOfStackReserve;
  ULONGLONG SizeOfStackCommit;
  ULONGLONG SizeOfHeapReserve;
  ULONGLONG SizeOfHeapCommit;
  DWORD LoaderFlags;
  DWORD NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} __attribute__((aligned(1), packed)) IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef struct _IMAGE_NT_HEADERS_GENERIC
{
  DWORD Signature;
  IMAGE_FILE_HEADER FileHeader;
} __attribute__((aligned(1), packed)) IMAGE_NT_HEADERS_GENERIC, *PIMAGE_NT_HEADERS_GENERIC;

typedef struct _IMAGE_NT_HEADERS64
{
  DWORD Signature;
  IMAGE_FILE_HEADER FileHeader;
  IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} __attribute__((aligned(1), packed)) IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

typedef struct _IMAGE_SECTION_HEADER
{
  BYTE Name[IMAGE_SIZEOF_SHORT_NAME];
  union
  {
    DWORD PhysicalAddress;
    DWORD VirtualSize;
  } Misc;
  DWORD VirtualAddress;
  DWORD SizeOfRawData;
  DWORD PointerToRawData;
  DWORD PointerToRelocations;
  DWORD PointerToLinenumbers;
  WORD NumberOfRelocations;
  WORD NumberOfLinenumbers;
  DWORD Characteristics;
} __attribute__((aligned(1), packed)) IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define FIELD_OFFSET(CLASS, FIELD) offsetof(CLASS, FIELD)
#define IMAGE_FIRST_SECTION64(ntheader) ((PIMAGE_SECTION_HEADER)((u64)(ntheader) +                                  \
                                                                 FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader) + \
                                                                 ((ntheader))->FileHeader.SizeOfOptionalHeader))

#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
