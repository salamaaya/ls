#ifndef _PRINT_H_
#define _PRINT_H_

#include <sys/stat.h>

void print_file(const char *);
void print_file_long(const char *, const struct stat *);
void print_indicator(const char  *, const struct stat *);

#endif
