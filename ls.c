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
#include "utils.h"

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
    int level = entry->fts_level;

    /* only print directory itself when -d is used */
    if ((flags & FLAG_d) && info == FTS_DP && level == 0) {
        return 1;
    }

    /* single file argument to ls */
    if (level == 0 && info == FTS_F) {
        return 1;
    }

    /* regular listing files inside directory */
    if (!(flags & FLAG_d) && level == 1) {
        /* check if entry is being visited in post-order, a regular file,
         * ".", or ".." */
        if (info == FTS_DP || info == FTS_F || info == FTS_DOT) {
            /* hidden files should only be printed in the case of -a and -A */
            if (!(flags & (FLAG_A | FLAG_a)) && is_hidden(entry->fts_name)) {
                    return 0;
            }
            return 1;
        }
    }

    /* recursive listing */
    if (flags & FLAG_R) {
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
    long blk_size;

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
                blk_size = get_dir_blk_size(entry->fts_path, flags);
                printf("total ");
                if (flags & FLAG_h) {
                    humanize(blk_size);
                    printf("\n");
                } else {
                    printf("%ld\n", blk_size);
                }
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
    char **dirs = malloc(argc * sizeof(char *));
    char **files = malloc(argc * sizeof(char *));
    int ch, dirsp = 0, filesp = 0, flags = 0, i;
    struct stat info;

    if (dirs == NULL) {
        (void)fprintf(stderr, "ls: malloc: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (files == NULL) {
        (void)fprintf(stderr, "ls: malloc: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

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
            flags &= ~FLAG_k; /* if -h is set, turn off -k */ 
            break;
        case 'i':
            flags |= FLAG_i;
            break;
        case 'k':
            flags |= FLAG_k;
            flags &= ~FLAG_h; /* if -k is set, turn off -h */ 
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

    /* here, find which arguments are directories and which are files,
     * that way, we can traverse the files first and then directories since
     * fts_open does not do that */
    for (i = 0; i < argc; i++) {
        if (stat(argv[i], &info) < 0) {
            (void)fprintf(stderr, "ls: stat: %s\n", strerror(errno));
            continue;
        }

        if (S_ISDIR(info.st_mode)) {
            dirs[dirsp++] = argv[i];
        } else {
            files[filesp++] = argv[i];
        }
    }

    /* handle no arguments to ls */
    if (filesp == 0 && dirsp == 0) {
        dirs[dirsp++] = ".";
    }

    if (filesp > 0) {
        traverse(files, filesp, flags);
    }
    if (dirsp > 0) {
        if (filesp > 0) {
            printf("\n");
            /* if there are files, then also print the labels on dirs */
            traverse(dirs, dirsp+1, flags);
        } else {
            traverse(dirs, dirsp, flags);
        }
    }

    free (files);
    free(dirs); /* create exit handler for this later! */
    return 0;
}
