#ifndef _LS_H_
#define _LS_H_

#include <fts.h>

static void usage(void);
void traverse(char *[], int);
void traverse_children(FTS *, int, int);
int main(int, char *[]);
int should_print(FTSENT *, int);
int print_hidden(const char *, int);
int print_header(FTSENT *, int);

#endif
