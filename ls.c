#include <sys/types.h>
#include <sys/stat.h>

#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cmp.h"
#include "flags.h"
#include "ls.h"

/*
 * checks whether a file is hidden or not.
 * return values:
 *  - 1: file is hidden
 *  - 0: file is not hidden
 */
int
is_hidden(const char* file_name) {
    if (file_name[0] == '.') {
        return 1;
    }
    return 0;
}

/*
 * traverses the given paths based on the given flags, prints each file name
 * along the traversal.
 */
void
print_traverse(char *paths[], int flags)
{
    char *file_name;
    FTS *fts;
    FTSENT *entry;
    
    if (flags & FLAG_f) {
        fts = fts_open(paths, FTS_SEEDOT, NULL);
    } else {
        fts = fts_open(paths, FTS_SEEDOT, ascending);
    }

    while ((entry = fts_read(fts))) {
        file_name = entry->fts_name; 
        if (entry->fts_info != FTS_DP && entry->fts_level == 1) {
            if (flags == 0 && is_hidden(file_name)) {
                continue;
            }
            if ((flags & FLAG_A) && (strcmp(file_name, ".") == 0 ||
                strcmp(file_name, "..") == 0)) {
                    continue;
            }
            printf("%s\n", file_name);
        }
    }

    if (fts_close(fts) < 0) {
        fprintf(stderr, "ls: fts_close: failed to close fts");
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
    int flags;

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

    print_traverse(argv, flags);    
    return 0;
}
