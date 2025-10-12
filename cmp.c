#include <string.h>

#include "cmp.h"

int
ascending(const FTSENT **entry1, const FTSENT **entry2)
{
    return strcmp((*entry1)->fts_name, (*entry2)->fts_name);
}
