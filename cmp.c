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
file_mtime(const FTSENT **entry1, const FTSENT **entry2)
{
    /* first check the difference in time at the second level, but if they are
     * the same, go down to nanosecond level */
    double diff = difftime((*entry2)->fts_statp->st_mtime, 
        (*entry1)->fts_statp->st_mtime);

    if (diff == 0) {
        long nano_diff = (*entry2)->fts_statp->st_mtimensec 
            - (*entry1)->fts_statp->st_mtimensec;
        if (nano_diff == 0) {
            return descending(entry1, entry2);
        }
        return nano_diff;
    }
    return diff;
}

int
file_atime(const FTSENT **entry1, const FTSENT **entry2)
{
    /* first check the difference in time at the second level, but if they are
     * the same, go down to nanosecond level */
    double diff = difftime((*entry2)->fts_statp->st_atime, 
        (*entry1)->fts_statp->st_atime);

    if (diff == 0) {
        long nano_diff = (*entry2)->fts_statp->st_atimensec 
            - (*entry1)->fts_statp->st_atimensec;
        if (nano_diff == 0) {
            return descending(entry1, entry2);
        }
        return nano_diff;
    }
    return diff;
}

int
file_ctime(const FTSENT **entry1, const FTSENT **entry2)
{
    /* first check the difference in time at the second level, but if they are
     * the same, go down to nanosecond level */
    double diff = difftime((*entry2)->fts_statp->st_ctime, 
        (*entry1)->fts_statp->st_ctime);

    if (diff == 0) {
        long nano_diff = (*entry2)->fts_statp->st_ctimensec 
            - (*entry1)->fts_statp->st_ctimensec;
        if (nano_diff == 0) {
            return descending(entry1, entry2);
        }
        return nano_diff;
    }
    return diff;
}
