#include "inc/include.h"

namespace rs::netvars
{
    struct netvar_record
    {
        char name[128 + 1];
        n32 offset;
    };

    struct netvar_table
    {
        char name[32 + 1];
        stl::linked_list_single<netvar_record> *contents;
    };

    stl::linked_list_single<netvar_table> *gtable;

    auto new_table(const char *name)
    {
        netvar_table rec;
        memset(rec.name, 0, sizeof(rec.name));
        strcpy(rec.name, name);
        rec.contents = stl::alloc_type<stl::linked_list_single<netvar_record>>(stl::global_mm_alloc, stl::global_mm_free);

        if (rec.contents == nullptr)
        {
        #if LOGGING_IS_ENABLED == 1
            msg("new_table: failed to allocate memory.");
        #endif
            __fastfail(0xDAC);
        }

        gtable->push(rec);
        return gtable->link_value(gtable->link_first());
    }

    auto new_netvar(netvar_table *storage, const char *name, n32 offset)
    {
        netvar_record rec;
        memset(&rec, 0, sizeof(rec));

        if (strlen(name) > sizeof(rec.name) - 1)
        {
        #if LOGGING_IS_ENABLED == 1
            msg("new_netvar: name exceeds maximum buffer cap!");
        #endif
            __fastfail(0xDAC);
        }

        strcpy(rec.name, name);
        rec.offset = offset;
        storage->contents->push(rec);
        return storage->contents->link_value(storage->contents->link_first());
    }

    bool is_prop_garbage(const char *name)
    {
        if (strstr(name, _XS("baseclass")) ||
            strstr(name, _XS("0")) ||
            strstr(name, _XS("1")) ||
            strstr(name, _XS("2")))
            return true;

        return false;
    }

    int32_t search_within_table(stl::linked_list_single<netvar_record> *table, const char *name)
    {
        for (auto link = table->link_first(); !table->link_is_last(link); table->link_next(&link))
        {
            auto value = table->link_value(link);

            if (!strcmp(value->name, name))
                return value->offset;
        }

        return 0;
    }

    int32_t find(const char *table, const char *name)
    {
        for (auto link = gtable->link_first(); !gtable->link_is_last(link); gtable->link_next(&link))
        {
            auto value = gtable->link_value(link);

            if (!strcmp(value->name, table))
            {
                auto off = search_within_table(value->contents, name);
                if (off != 0)
                    return off;
            }
        }

        return 0;
    }

    void process_prop(netvar_table *storage, rs::recv_prop *prop)
    {
        if (!prop->offset)
            return;
        if (is_prop_garbage(prop->name))
            return;
        new_netvar(storage, prop->name, prop->offset);
    }

    void process_table(rs::recv_table *table)
    {
        auto storage = new_table(table->name);
        auto head = table->props;

        for (auto i = 0ull; i < table->num_props; i++)
        {
            auto *const prop = head[i];

            if (prop == nullptr)
                continue;

            if (is_prop_garbage(prop->name))
                continue;

            process_prop(storage, prop);

            if (prop->data_table != nullptr)
                process_table(prop->data_table);
        }
    }

    bool init()
    {
        gtable = stl::alloc_type<stl::linked_list_single<netvar_table>>(
            stl::global_mm_alloc, stl::global_mm_free);

        auto cc_head = *reinterpret_cast<rs::client_class **>(gctx->game_base + gctx->offsets.client_class_head);
        if (cc_head == nullptr)
            return false;

        for (auto *client_class = cc_head; client_class != nullptr; client_class = client_class->next)
        {
            auto *const table = client_class->table;

            if (table == nullptr)
                continue;

            process_table(table);
        }

        /* init the global net vars. */
        nv::entity::init();
        nv::collision::init();

        /* after we're done initing, get rid off the global table.*/
        gtable->clear();

        return true;
    }
}
