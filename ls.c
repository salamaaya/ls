#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cmp.h"
#include "flags.h"
#include "ls.h"
#include "print.h"

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
    struct stat info;
    struct dirent *entry;

    if ((dp = opendir(dir)) == NULL) {
        (void)fprintf(stderr, "ls: opendir: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    while ((entry = readdir(dp))) {
        if (!(flags & FLAG_a) && is_hidden(entry->d_name)) {
            continue;
        }

        /*if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }*/
        /* construct the full path to the subdir */
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (stat(path, &info) < 0) {
            (void)fprintf(stderr, "ls: stat: %s: %s\n", path, strerror(errno));
            continue;
        }

        total += info.st_blocks;
    }

    if (closedir(dp) < 0) {
        (void)fprintf(stderr, "ls: closedir: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return total;
}

/*
 * determines whether a file should be printed based on the current flags
 * set and the entry's FTSENT struct info.
 * return values:
 *  0: file info should not be printed
 *  1: file info should be printed
 */
int
should_print(int flags, FTSENT *entry)
{
    ushort info = entry->fts_info;

    /* only print directory with -d */
    if (info == FTS_DP && (flags & FLAG_d) && entry->fts_level == 0) {
        return 1;
    } else if (entry->fts_level == 0 && info == FTS_F) { /* single file ls */
        return 1;
    } else if (!(flags & FLAG_d) && entry->fts_level == 1) {
        /* check if entry is being visited in post-order, a regular file,
         * ".", or ".." */
        if (info == FTS_DP || info == FTS_F || info == FTS_DOT) {
            /* hidden files should only be printed in the case of -a and -A */
            if (!(flags & (FLAG_A | FLAG_a)) && is_hidden(entry->fts_name)) {
                    return 0;
            }
            return 1;
        }
    } else if (flags & FLAG_R) {
        return 1;
    }
    return 0;
}

/*
 * traverses the given paths based on the given flags, prints each file name
 * along the traversal.
 */
void
traverse(char *paths[], int argc, int flags)
{
    FTS *fts;
    FTSENT *entry;
    int (*compar)(const FTSENT **, const FTSENT **) = ascending;
    int options = FTS_WHITEOUT;

    /* root i, used to keep track of the number of roots traversed,
     * for printing purposes */
    int ri = 0; 

    if (flags & FLAG_f) {
        compar = NULL;
    } else if (flags & FLAG_r) {
        compar = descending;
    } else if (flags & FLAG_S) {
        compar = size;
    } else if (flags & FLAG_t) {
        compar = file_mtime;
        if (flags & FLAG_u) {
            compar = file_atime;
        } else if (flags & FLAG_c) {
            compar = file_ctime;
        }
    }

    if (flags & FLAG_a) {
        options |= FTS_SEEDOT;
    }

    if ((fts = fts_open(paths, options, compar)) == NULL) {
        (void)fprintf(stderr, "ls: fts_open: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while ((entry = fts_read(fts))) {
        if (entry->fts_info == FTS_D && entry->fts_level == 0) { 
            /* if there are multiple paths to read, the first one has no \n */
            if (ri >= 1) {
                printf("\n");
            }
            if (argc > 1) {
                printf("%s:\n", entry->fts_path);
            }
            if (flags & FLAG_l) {
                printf("total %ld\n", get_dir_blk_size(entry->fts_path, flags));
            }
            ri++;
        }
        if (should_print(flags, entry)) {
            print_file(entry->fts_name, entry->fts_statp, flags);
        }
    }

    if (fts_close(fts) < 0) {
        (void)fprintf(stderr, "ls: fts_close: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void
usage()
{
    (void)fprintf(stderr, "usage: ls [-AacdFfhiklnqRrSstuw] [file ...]\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int ch, flags = 0;

    while ((ch = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
        case 'A':
            flags |= FLAG_A;
            break;
        case 'a':
            flags |= FLAG_a;
            break;
        case 'c':
            flags |= FLAG_c;
            flags &= ~FLAG_u; /* if -c is set, turn off -u */ 
            break;
        case 'd':
            flags |= FLAG_d;
            flags &= ~FLAG_R; /* if -d is set, -R must be turned off */
            break;
        case 'F':
            flags |= FLAG_F;
            break;
        case 'f':
            flags |= FLAG_f;
            flags |= FLAG_a; /* like NetBSD, -f implies -a */
            break;
        case 'h':
            flags |= FLAG_h;
            break;
        case 'i':
            flags |= FLAG_i;
            break;
        case 'k':
            flags |= FLAG_k;
            break;
        case 'l':
            flags |= FLAG_l;
            break;
        case 'n':
            flags |= FLAG_n;
            flags |= FLAG_l; /* -n implies -l */
            break;
        case 'q':
            flags |= FLAG_q;
            break;
        case 'R':
            /* if -d is set, don't turn on -R */
            flags = (flags & FLAG_d) ? flags : flags & FLAG_R;
            break;
        case 'r':
            flags |= FLAG_r;
            break;
        case 'S':
            flags |= FLAG_S;
            break;
        case 's':
            flags |= FLAG_s;
            break;
        case 't':
            flags |= FLAG_t;
            break;
        case 'u':
            flags |= FLAG_u;
            flags &= ~FLAG_c; /* if -u is set, turn off -c */ 
            break;
        case 'w':
            flags |= FLAG_w;
            break;
        case '?':
        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;
    
    if (argv[0] == NULL) {
        argv[0] = ".";
    }

    traverse(argv, argc, flags);    
    return 0;
}
