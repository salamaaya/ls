#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flags.h"
#include "utils.h"

/*
 * checks whether a file is hidden or not.
 * return values:
 *  - 1: file is hidden
 *  - 0: file is not hidden
 */
int
is_hidden(const char *filename)
{
    if (filename[0] == '.') {
        return 1;
    }
    return 0;
}

/*
 * calculates the total number of blocks a directory takes
 */
blkcnt_t
get_dir_blk_size(const char *dir, int flags)
{
    blkcnt_t total = 0;
    char path[PATH_MAX];
    DIR *dp;
    long blk_size, proportion;
    struct stat info;
    struct dirent *entry;

    (void)getbsize(NULL, &blk_size);

    if ((dp = opendir(dir)) == NULL) {
        (void)fprintf(stderr, "ls: opendir: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    while ((entry = readdir(dp))) {
        /* only count hidden files if -A or -a is set
         * and only count "." and ".." if -a is set */
        if (!(flags & FLAG_a) && (strcmp(entry->d_name, ".") == 0 
            || strcmp(entry->d_name, "..") == 0)) {
                continue;
        } else if (!(flags & (FLAG_a | FLAG_A)) && is_hidden(entry->d_name)) {
            continue;
        }

        /* construct the full path to the subdir */
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (stat(path, &info) < 0) {
            (void)fprintf(stderr, "ls: stat: %s: %s\n", path, strerror(errno));
            continue;
        }

        /* if -h is set, we only care about the actual size to be humanized */
        if (flags & FLAG_h) {
            total += info.st_size;
        } else {
            total += info.st_blocks;
        }
    }

    if (closedir(dp) < 0) {
        (void)fprintf(stderr, "ls: closedir: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* total is in the unit of 512 byte blocks, which is half a KB */
    if (flags & FLAG_k) {
        return total / 2;
    }

    if (flags & FLAG_h) {
        return total;
    }

    /* total is in the form of 512 byte blocks, we can the number of BLOCKSIZE
     * blocks by first finding how many 512 byte blocks go into BLOCKSIZE, and
     * then dividing that by the total */
    proportion = blk_size / STAT_BLK_SIZE;

    /* this handles the rounding since it doesn't makes sense to have portions
     * of a block */
    return (total + (proportion - 1)) / proportion;
}

/* 
 * finds the number of blocks a file takes up based on the env variable 
 * BLOCKSIZE
 */
long
get_file_blk_size(const struct stat *sb)
{
    long blk_size;
    long proportion;
    (void)getbsize(NULL, &blk_size);

    /* total is in the form of 512 byte blocks, we can the number of BLOCKSIZE
     * blocks by first finding how many 512 byte blocks go into BLOCKSIZE, and
     * then dividing that by the total */
    proportion = blk_size / STAT_BLK_SIZE;

    /* this handles the rounding since it doesn't makes sense to have portions
     * of a block */
    return (sb->st_blocks + (proportion - 1)) / proportion;
}

/*char *
humanize(blkcnt_t blocks)
{
    char buf[BUFSIZ];
    unsigned long int num;

    if ((num = blocks * STAT_BLK_SIZE / EXA) > 0) {
        humanize_number(buf, sizeof(buf), &num, "E", HN_AUTOSCALE, 
            HN_DECIMAL | HN_NOSPACE);
}*/
