#ifndef _LS_H_
#define _LS_H_

#include <fts.h>

static void usage(void);
void traverse(char *[], int, int);
int main(int, char *[]);
int is_hidden(const char *);
int should_print(int, FTSENT *);
blkcnt_t get_dir_blk_size(const char *, int);

#endif
