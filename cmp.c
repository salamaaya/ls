#include <sys/stat.h>

#include <string.h>

#include "cmp.h"

int
ascending(const FTSENT **entry1, const FTSENT **entry2)
{
    return strcmp((*entry1)->fts_name, (*entry2)->fts_name);
}

int
descending(const FTSENT **entry1, const FTSENT **entry2)
{
    return strcmp((*entry2)->fts_name, (*entry1)->fts_name);
}

int
size(const FTSENT **entry1, const FTSENT **entry2)
{
    if ((*entry1)->fts_statp->st_size > (*entry2)->fts_statp->st_size) {
        return -1;
    } else if ((*entry1)->fts_statp->st_size < (*entry2)->fts_statp->st_size) {
        return 1;
    }
    return 0; 
}

int
mtime(const FTSENT **entry1, const FTSENT **entry2)
{
    if ((*entry1)->fts_statp->st_mtime > (*entry2)->fts_statp->st_mtime) {
        return -1;
    } else if ((*entry1)->fts_statp->st_mtime < (*entry2)->fts_statp->st_mtime) {
        return 1;
    }
    return 0; 
}

int
atime(const FTSENT **entry1, const FTSENT **entry2)
{
    if ((*entry1)->fts_statp->st_atime > (*entry2)->fts_statp->st_atime) {
        return -1;
    } else if ((*entry1)->fts_statp->st_atime < (*entry2)->fts_statp->st_atime) {
        return 1;
    }
    return 0; 
}
