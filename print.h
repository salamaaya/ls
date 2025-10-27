#ifndef _PRINT_H_
#define _PRINT_H_

#include <sys/stat.h>

void print_file(char *, char *, const struct stat *, int);
void print_file_long(char *, char *, const struct stat *, int);
void print_indicator(const struct stat *);
void humanize(off_t);

#endif
