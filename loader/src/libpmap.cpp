#include "inc/include.h"

/**
 * @file parse_procmaps.c
 * @brief Source file for lib procmaps
 * @author Lilian VERLHAC
 * @version 1.0
 * @date 2020-04-18
 */

#include "inc/libpmap.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void destroy_procmaps(hr_procmaps **procmaps)
{
    size_t i = 0;

    while (procmaps[i]) {
        free(procmaps[i]->pathname);
        free(procmaps[i]);
        ++i;
    }
    free(procmaps);
}

static unsigned char make_perms(char const *perms)
{
    unsigned char perm = 0x00;

    if (perms[0] == 'r') {
        perm |= PERMS_READ;
    }
    if (perms[1] == 'w') {
        perm |= PERMS_WRITE;
    }
    if (perms[2] == 'x') {
        perm |= PERMS_EXECUTE;
    }
    if (perms[3] == 'p') {
        perm |= PERMS_PRIVATE;
    } else if (perms[3] == 's') {
        perm |= PERMS_SHARED;
    }
    return perm;
}

procmaps_row_t *parse_procmaps_line(char *line)
{
    auto procmaps_row = (procmaps_row_t*)malloc(sizeof(procmaps_row_t));
    char *pathname_token = NULL;

    if (procmaps_row == NULL)
        return NULL;
    procmaps_row->addr_begin = strtoull(strtok(line, "-"), NULL, 16);
    procmaps_row->addr_end = strtoull(strtok(NULL, " "), NULL, 16);
    procmaps_row->perms = make_perms(strtok(NULL, " "));
    procmaps_row->offset = strtoull(strtok(NULL, " "), NULL, 16);
    procmaps_row->dev.major = atoi(strtok(NULL, ":"));
    procmaps_row->dev.minor = atoi(strtok(NULL, " "));
    procmaps_row->inode = atoi(strtok(NULL, " "));
    pathname_token = strtok(NULL, " \n");
    if (pathname_token != NULL) {
        procmaps_row->pathname = strdup(pathname_token);
        if (procmaps_row->pathname == NULL) {
            free(procmaps_row);
            return NULL;
        }
    } else {
        procmaps_row->pathname = NULL;
    }
    return procmaps_row;
}

static hr_procmaps **make_procmaps_array(FILE *procmaps_file)
{
    size_t array_size = PROCMAPS_ARRAY_BASE_SIZE;
    hr_procmaps **procmaps_array = (hr_procmaps**)calloc(array_size, sizeof(hr_procmaps));
    size_t i = 0;
    size_t size = 0;
    char *line = NULL;

    if (procmaps_array == NULL)
        return NULL;
    while (getline(&line, &size, procmaps_file) != -1) {
        if (line != NULL)
            procmaps_array[i] = parse_procmaps_line(line);
        ++i;
        if (i == array_size) {
            procmaps_array = (hr_procmaps**)realloc(procmaps_array, (array_size + 16) * sizeof(*procmaps_array));
            if (procmaps_array == NULL) {
                destroy_procmaps(procmaps_array);
                return NULL;
            }
            memset(procmaps_array + array_size, 0, 16 * sizeof(*procmaps_array));
            array_size += 16;
        }
        free(line), line = NULL;
    }
    free(line);
    if (errno == EINVAL || errno == ENOMEM) {
        destroy_procmaps(procmaps_array);
        return NULL;
    }
    return procmaps_array;
}

std::mutex procmaps_lock;

hr_procmaps **contruct_procmaps(int pid)
{
    

    char path[100];
    FILE *procmaps_file = NULL;
    int errno_saver = 0;
    hr_procmaps **procmaps_array = NULL;

    if (pid <= 0) {
        strcpy(path, "/proc/self/maps");
    } else {
        sprintf(path, "/proc/%d/maps", pid);
    }
    procmaps_file = fopen(path, "r");
    if (procmaps_file == NULL) {
        perror("fopen");
        return NULL;
    }
    procmaps_array = make_procmaps_array(procmaps_file);
    errno_saver = errno;
    if (fclose(procmaps_file) != -1)
        errno = errno_saver;
    return procmaps_array;
}

void procmaps_begin_transaction()
{
    procmaps_lock.lock();
}

void procmaps_end_transaction()
{
    procmaps_lock.unlock();
}
