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

/* global pointers which might have to be freed during unexpected exit */
char **dirs, **files; 

void
free_exit(void)
{
    free(dirs);
    free(files);
}

/*
 * traverses the given paths based on the given flags, prints each file name
 * along the traversal.
 */
void
traverse(char *paths[], int flags)
{
    char *file, *path;
    FTS *fts;
    FTSENT *entry;
    int (*compar)(const FTSENT **, const FTSENT **) = ascending;
    int options = FTS_WHITEOUT | FTS_PHYSICAL;
    int info, level, stop_traverse, print_header; 
    int print_dot = flags & (FLAG_A | FLAG_a);
    int num_headers = 0;
    long blk_size = 0;

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
        info = entry->fts_info;
        path = entry->fts_path;
        file = entry->fts_name;
        level = entry->fts_level;
        print_header = !(flags & FLAG_R)
                || ((flags & FLAG_R) && level > 0);

        if (info == FTS_DNR || info == FTS_ERR) {
            (void)fprintf(stderr, "ls: fts_read: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (info == FTS_D) {
            if (!(flags & FLAG_R)) {
                stop_traverse = 1;
            } else if (level > 0 && !print_dot && is_hidden(file)) {
                stop_traverse = 1;
            } else {
                stop_traverse = 0;
            }

            if (flags & FLAG_d) {
                printf("%s\n", path);
            }
            if (flags & FLAG_headers) {
                if (num_headers > 0 && !stop_traverse) {
                    printf("\n");
                }

                if (print_header && !stop_traverse) {
                    printf("%s:\n", path);
                }
                num_headers++;
            }
            if ((flags & FLAG_l) && (level == 0 || (flags & FLAG_R))) {
                blk_size = get_dir_blk_size(path, flags);
                printf("total ");
                if (flags & FLAG_h) {
                    humanize(blk_size);
                    printf("\n");
                } else {
                    printf("%ld\n", blk_size);
                }
            }

            if (stop_traverse || (flags & FLAG_d)) {
                if (fts_set(fts, entry, FTS_SKIP) < 0) { 
                    (void)fprintf(stderr, "ls: fts_set: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }

            if (!(flags & FLAG_d) && ((!stop_traverse) || !(flags & FLAG_R))) {
                traverse_children(fts, flags, print_dot);
            }
        } else if (info != FTS_D && info != FTS_DP && level == 0) {
            print_file(file, entry->fts_statp, flags);
        }
    }

    if (fts_close(fts) < 0) {
        (void)fprintf(stderr, "ls: fts_close: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void
traverse_children(FTS *fts, int flags, int print_hidden)
{
    char *file;
    FTSENT *children = fts_children(fts, 0);
    FTSENT *node = children;

    while (node != NULL) {
        file = node->fts_name;
        if ((print_hidden && is_hidden(file)) || !is_hidden(file)) {
            print_file(file, node->fts_statp, flags);
        }
        node = node->fts_link;
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
    int ch, dirsp = 0, filesp = 0, flags = 0, i;
    struct stat info;
    
    if (atexit(free_exit) != 0) {
        perror("can't register free_exit\n");
		exit(EXIT_FAILURE);
	}

    dirs = malloc(argc * sizeof(char *));
    files = malloc(argc * sizeof(char *));

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
            flags = (flags & FLAG_d) ? flags : flags | FLAG_R;
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
        traverse(files, flags);
    }
    if (dirsp > 0) {
        if (filesp > 0) {
            printf("\n");
            flags |= FLAG_headers;
        }

        if (dirsp > 1 && !(flags & FLAG_d)) {
            flags |= FLAG_headers;
        }

        if (flags & FLAG_R) {
            flags |= FLAG_headers;
        }

        traverse(dirs, flags);
    }

    free (files);
    free(dirs); /* create exit handler for this later! */
    return 0;
}
