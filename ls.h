#ifndef _LS_H_
#define _LS_H_

#include <fts.h>

static void usage(void);
void traverse(char *[], int, int);
int main(int, char *[]);
int should_print(int, FTSENT *);

#endif
