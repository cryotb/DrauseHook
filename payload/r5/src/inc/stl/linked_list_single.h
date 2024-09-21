#pragma once

#define CONTAINING_RECORD(address, type, field) \
    (reinterpret_cast<type*>(reinterpret_cast<char*>(address) - __builtin_offsetof(type, field)))

namespace stl
{
    // Define the LIST_ENTRY structure
    typedef struct _LIST_ENTRY
    {
        struct _LIST_ENTRY *Flink;
        struct _LIST_ENTRY *Blink;
    } LIST_ENTRY, *PLIST_ENTRY;

    LLVM_NOOPT LLVM_INLINE void InitializeListHead(PLIST_ENTRY listHead)
    {
        listHead->Flink = listHead->Blink = listHead;
    }

    LLVM_NOOPT LLVM_INLINE void InsertHeadList(PLIST_ENTRY listHead, PLIST_ENTRY entry)
    {
        entry->Flink = listHead->Flink;
        entry->Blink = listHead;
        listHead->Flink->Blink = entry;
        listHead->Flink = entry;
    }

    LLVM_NOOPT LLVM_INLINE void RemoveEntryList(PLIST_ENTRY entry)
    {
        entry->Blink->Flink = entry->Flink;
        entry->Flink->Blink = entry->Blink;
        entry->Flink = entry->Blink = NULL;
    }

    /*
        WARNING: DO NOT USE C++ CLASSES WITH DYNAMIC CONSTRUCTORS, DYNAMIC DATA TYPES ET CETERA.
        THIS IS INTENDED TO BE A REPLACEMENT FOR C STYLE ARRAYS BUT WITH DYNAMIC WIDTH, NOT A VECTOR REPLACEMENT!
    */
    template <typename storage_type>
    class linked_list_single
    {
    public:
        linked_list_single() = delete;
        linked_list_single(fn_mem_alloc alloc_fun, fn_mem_free free_fun)
            : _mm_fun_alloc(alloc_fun), _mm_fun_free(free_fun)
        {
            InitializeListHead(&_head);
        }

        LLVM_NOOPT void push(const storage_type &data)
        {
            list_entry_data<storage_type> *entry = static_cast<list_entry_data<storage_type> *>(_mm_fun_alloc(sizeof(list_entry_data<storage_type>)));
            if (entry != nullptr)
            {
                entry->entry.Flink = _head.Flink;
                _head.Flink = &(entry->entry);
                memcpy( &entry->data, &data, sizeof(storage_type));
            }
        }

        storage_type pop()
        {
            if (_head.Flink == nullptr)
                return storage_type();

            LIST_ENTRY *entry = _head.Flink;
            _head.Flink = entry->Flink;

            list_entry_data<storage_type> *entryData = reinterpret_cast<list_entry_data<storage_type> *>(entry);
            storage_type data = entryData->data;

            _mm_fun_free(entryData);
            return data;
        }

        void clear()
        {
            LIST_ENTRY *entry = _head.Flink;
            while (entry != head())
            {
                LIST_ENTRY *nextEntry = entry->Flink;
                list_entry_data<storage_type> *entryData = reinterpret_cast<list_entry_data<storage_type> *>(entry);
                _mm_fun_free(entryData);
                entry = nextEntry;
            }
        }

        auto head() { return &_head; }
        auto link_first() { return head()->Flink; }
        auto link_is_last(LIST_ENTRY *link) { return link == head(); }
        auto link_next(LIST_ENTRY **link) { *link = (*(link))->Flink; }
        
        LLVM_NOOPT storage_type* link_value(LIST_ENTRY *link)
        {
            return &(CONTAINING_RECORD(link, list_entry_data<storage_type>, entry)->data);
        }

    private : template <typename T>
              struct list_entry_data
        {
            LIST_ENTRY entry;
            T data;
        };

        fn_mem_alloc _mm_fun_alloc;
        fn_mem_free _mm_fun_free;
        LIST_ENTRY _head;
    };
}
