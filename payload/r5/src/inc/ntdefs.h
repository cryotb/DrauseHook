#pragma once

#define LOCAL_PEB() reinterpret_cast<nt::PPEB>(__readgsqword(decrypt_constant(encrypt_constant(0x60))))

typedef long LONG;
typedef u8 BYTE;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef u16 WORD;
typedef u32 ULONG;
typedef u32 DWORD;
typedef u64 ULONG64;
typedef uptr ULONG_PTR;
typedef uptr DWORD64;
typedef ptr PVOID;
typedef ptr HANDLE;
typedef u8 UCHAR;
typedef u8 BOOLEAN;
typedef unsigned short USHORT;

typedef char CHAR;
typedef CHAR *PCHAR;
typedef wchar_t WCHAR;
typedef WCHAR *PWSTR;

#define IMAGE_DOS_SIGNATURE 0x5A4D    // MZ
#define IMAGE_OS2_SIGNATURE 0x454E    // NE
#define IMAGE_OS2_SIGNATURE_LE 0x454C // LE
#define IMAGE_VXD_SIGNATURE 0x454C    // LE
#define IMAGE_NT_SIGNATURE 0x00004550 // PE00

#define IMAGE_DIRECTORY_ENTRY_EXPORT 0

#define EXCEPTION_MAXIMUM_PARAMETERS 15

#define EXCEPTION_EXECUTE_HANDLER      1
#define EXCEPTION_CONTINUE_SEARCH      0
#define EXCEPTION_CONTINUE_EXECUTION (-1)

//
//  These are the generic rights.
//
#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)

//
// Define access rights to files and directories
//

//
// The FILE_READ_DATA and FILE_WRITE_DATA constants are also defined in
// devioctl.h as FILE_READ_ACCESS and FILE_WRITE_ACCESS. The values for these
// constants *MUST* always be in sync.
// The values are redefined in devioctl.h because they must be available to
// both DOS and NT.
//

#define FILE_READ_DATA            ( 0x0001 )    // file & pipe
#define FILE_LIST_DIRECTORY       ( 0x0001 )    // directory

#define FILE_WRITE_DATA           ( 0x0002 )    // file & pipe
#define FILE_ADD_FILE             ( 0x0002 )    // directory

#define FILE_APPEND_DATA          ( 0x0004 )    // file
#define FILE_ADD_SUBDIRECTORY     ( 0x0004 )    // directory
#define FILE_CREATE_PIPE_INSTANCE ( 0x0004 )    // named pipe


#define FILE_READ_EA              ( 0x0008 )    // file & directory

#define FILE_WRITE_EA             ( 0x0010 )    // file & directory

#define FILE_EXECUTE              ( 0x0020 )    // file
#define FILE_TRAVERSE             ( 0x0020 )    // directory

#define FILE_DELETE_CHILD         ( 0x0040 )    // directory

#define FILE_READ_ATTRIBUTES      ( 0x0080 )    // all

#define FILE_WRITE_ATTRIBUTES     ( 0x0100 )    // all

#define FILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | SYNCHRONIZE | 0x1FF)

#define FILE_GENERIC_READ         (STANDARD_RIGHTS_READ     |\
                                   FILE_READ_DATA           |\
                                   FILE_READ_ATTRIBUTES     |\
                                   FILE_READ_EA             |\
                                   SYNCHRONIZE)


#define FILE_GENERIC_WRITE        (STANDARD_RIGHTS_WRITE    |\
                                   FILE_WRITE_DATA          |\
                                   FILE_WRITE_ATTRIBUTES    |\
                                   FILE_WRITE_EA            |\
                                   FILE_APPEND_DATA         |\
                                   SYNCHRONIZE)


#define FILE_GENERIC_EXECUTE      (STANDARD_RIGHTS_EXECUTE  |\
                                   FILE_READ_ATTRIBUTES     |\
                                   FILE_EXECUTE             |\
                                   SYNCHRONIZE)

#define FILE_SHARE_READ                 0x00000001  
#define FILE_SHARE_WRITE                0x00000002  
#define FILE_SHARE_DELETE               0x00000004  
#define FILE_ATTRIBUTE_READONLY             0x00000001  
#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
#define FILE_ATTRIBUTE_DEVICE               0x00000040  
#define FILE_ATTRIBUTE_NORMAL               0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  
#define FILE_ATTRIBUTE_OFFLINE              0x00001000  
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000  
#define FILE_ATTRIBUTE_INTEGRITY_STREAM     0x00008000  
#define FILE_ATTRIBUTE_VIRTUAL              0x00010000  
#define FILE_ATTRIBUTE_NO_SCRUB_DATA        0x00020000  
#define FILE_ATTRIBUTE_EA                   0x00040000  
#define FILE_ATTRIBUTE_PINNED               0x00080000  
#define FILE_ATTRIBUTE_UNPINNED             0x00100000  
#define FILE_ATTRIBUTE_RECALL_ON_OPEN       0x00040000  
#define FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS 0x00400000 
#define TREE_CONNECT_ATTRIBUTE_PRIVACY      0x00004000  
#define TREE_CONNECT_ATTRIBUTE_INTEGRITY    0x00008000  
#define TREE_CONNECT_ATTRIBUTE_GLOBAL       0x00000004  
#define TREE_CONNECT_ATTRIBUTE_PINNED       0x00000002  
#define FILE_ATTRIBUTE_STRICTLY_SEQUENTIAL  0x20000000  
#define FILE_NOTIFY_CHANGE_FILE_NAME    0x00000001   
#define FILE_NOTIFY_CHANGE_DIR_NAME     0x00000002   
#define FILE_NOTIFY_CHANGE_ATTRIBUTES   0x00000004   
#define FILE_NOTIFY_CHANGE_SIZE         0x00000008   
#define FILE_NOTIFY_CHANGE_LAST_WRITE   0x00000010   
#define FILE_NOTIFY_CHANGE_LAST_ACCESS  0x00000020   
#define FILE_NOTIFY_CHANGE_CREATION     0x00000040   
#define FILE_NOTIFY_CHANGE_SECURITY     0x00000100   
#define FILE_ACTION_ADDED                   0x00000001   
#define FILE_ACTION_REMOVED                 0x00000002   
#define FILE_ACTION_MODIFIED                0x00000003   
#define FILE_ACTION_RENAMED_OLD_NAME        0x00000004   
#define FILE_ACTION_RENAMED_NEW_NAME        0x00000005   
#define MAILSLOT_NO_MESSAGE             ((DWORD)-1) 
#define MAILSLOT_WAIT_FOREVER           ((DWORD)-1) 
#define FILE_CASE_SENSITIVE_SEARCH          0x00000001  
#define FILE_CASE_PRESERVED_NAMES           0x00000002  
#define FILE_UNICODE_ON_DISK                0x00000004  
#define FILE_PERSISTENT_ACLS                0x00000008  
#define FILE_FILE_COMPRESSION               0x00000010  
#define FILE_VOLUME_QUOTAS                  0x00000020  
#define FILE_SUPPORTS_SPARSE_FILES          0x00000040  
#define FILE_SUPPORTS_REPARSE_POINTS        0x00000080  
#define FILE_SUPPORTS_REMOTE_STORAGE        0x00000100  
#define FILE_RETURNS_CLEANUP_RESULT_INFO    0x00000200  
#define FILE_SUPPORTS_POSIX_UNLINK_RENAME   0x00000400  
#define FILE_SUPPORTS_BYPASS_IO             0x00000800  



#define FILE_VOLUME_IS_COMPRESSED           0x00008000  
#define FILE_SUPPORTS_OBJECT_IDS            0x00010000  
#define FILE_SUPPORTS_ENCRYPTION            0x00020000  
#define FILE_NAMED_STREAMS                  0x00040000  
#define FILE_READ_ONLY_VOLUME               0x00080000  
#define FILE_SEQUENTIAL_WRITE_ONCE          0x00100000  
#define FILE_SUPPORTS_TRANSACTIONS          0x00200000  
#define FILE_SUPPORTS_HARD_LINKS            0x00400000  
#define FILE_SUPPORTS_EXTENDED_ATTRIBUTES   0x00800000  
#define FILE_SUPPORTS_OPEN_BY_FILE_ID       0x01000000  
#define FILE_SUPPORTS_USN_JOURNAL           0x02000000  
#define FILE_SUPPORTS_INTEGRITY_STREAMS     0x04000000  
#define FILE_SUPPORTS_BLOCK_REFCOUNTING     0x08000000  
#define FILE_SUPPORTS_SPARSE_VDL            0x10000000  
#define FILE_DAX_VOLUME                     0x20000000  
#define FILE_SUPPORTS_GHOSTING              0x40000000  

//
// Constants
//
#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5
