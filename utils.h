#ifndef _SIZE_H_
#define _SIZE_H_

#include <sys/stat.h>

/* the "st_blocks" field in the stat struct is in 512 byte units which will
 * be used to calculate the number of blocks based on BLOCKSIZE */
#define STAT_BLK_SIZE 512 

blkcnt_t get_dir_blk_size(const char *, int);
long get_file_blk_size(const struct stat *);
int is_hidden(const char *);

#endif
