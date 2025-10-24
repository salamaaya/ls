#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
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
is_hidden(const char* filename) {
    if (filename[0] == '.') {
        return 1;
    }
    return 0;
}

/*
 * determines whether a file should be printed based on the current flags
 * set and the entry's FTSENT struct info.
 * return values:
 *  0: file info should not be printed
 *  1: file info should be printed
 */
int
should_print(int flags, FTSENT *entry) {
    ushort info = entry->fts_info;

    /* only print directory with -d */
    if (info == FTS_DP && (flags & FLAG_d) && entry->fts_level < 1) {
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
traverse(char *paths[], int flags)
{
    FTS *fts;
    FTSENT *entry;
    int (*compar)(const FTSENT **, const FTSENT **) = ascending;
    int options = FTS_WHITEOUT | FTS_PHYSICAL;

    if (flags & FLAG_f) {
        flags |= FLAG_a; /* like NetBSD, -f implies -a */
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
        fprintf(stderr, "ls: fts_open: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while ((entry = fts_read(fts))) {
        if (should_print(flags, entry)) {
            if ((flags & FLAG_l) || (flags & FLAG_n)) {
                print_file_long(entry->fts_name, entry->fts_statp, flags);
            } else {
                print_file(entry->fts_name, entry->fts_statp, flags);
            }
        }
    }

    if (fts_close(fts) < 0) {
        fprintf(stderr, "ls: fts_close: %s\n", strerror(errno));
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
    int ch;
    int flags = 0;

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
            break;
        case 'd':
            flags |= FLAG_d;
            break;
        case 'F':
            flags |= FLAG_F;
            break;
        case 'f':
            flags |= FLAG_f;
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
            break;
        case 'q':
            flags |= FLAG_q;
            break;
        case 'R':
            flags |= FLAG_R;
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

    traverse(argv, flags);    
    return 0;
}
