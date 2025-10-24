#ifndef _CMP_H_
#define _CMP_H_

#include <sys/types.h>

#include <fts.h>

int ascending(const FTSENT **, const FTSENT **);
int descending(const FTSENT **, const FTSENT **);
int size(const FTSENT **, const FTSENT **);
int file_mtime(const FTSENT **, const FTSENT **);
int file_atime(const FTSENT **, const FTSENT **);
int file_ctime(const FTSENT **, const FTSENT **);

#endif
