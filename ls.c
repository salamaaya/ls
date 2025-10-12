#include <sys/types.h>
#include <sys/stat.h>

#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cmp.h"
#include "ls.h"

void
print_traverse(char *paths[])
{
    FTS *fts = fts_open(paths, FTS_SEEDOT, ascending);
    FTSENT *entry;

    while ((entry = fts_read(fts))) {
        if (entry->fts_level == 1 && entry->fts_name[0] != '.') {
            printf("%s\n", entry->fts_name);
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
    int Aflag = 0, aflag = 0, cflag = 0, dflag = 0, Fflag = 0, fflag = 0,
        hflag = 0, iflag = 0, kflag = 0, lflag = 0, nflag = 0, qflag = 0,
        Rflag = 0, rflag = 0, Sflag = 0, sflag = 0, tflag = 0, uflag = 0,
        wflag = 0;

    while ((ch = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
        case 'A':
            Aflag = 1;
            break;
        case 'a':
            aflag = 1;
            break;
        case 'c':
            cflag = 1;
            break;
        case 'd':
            dflag = 1;
            break;
        case 'F':
            Fflag = 1;
            break;
        case 'f':
            fflag = 1;
            break;
        case 'h':
            hflag = 1;
            break;
        case 'i':
            iflag = 1;
            break;
        case 'k':
            kflag = 1;
            break;
        case 'l':
            lflag = 1;
            break;
        case 'n':
            nflag = 1;
            break;
        case 'q':
            qflag = 1;
            break;
        case 'R':
            Rflag = 1;
            break;
        case 'r':
            rflag = 1;
            break;
        case 'S':
            Sflag = 1;
            break;
        case 's':
            sflag = 1;
            break;
        case 't':
            tflag = 1;
            break;
        case 'u':
            uflag = 1;
            break;
        case 'w':
            wflag = 1;
            break;
        case '?':
        default:
            usage();
        }
    }
    argc -= optind;
    argv += optind;
    
    if (Aflag) {
        printf("-A not implemented yet!\n");
    }
    if (aflag) {
        printf("-a not implemented yet!\n");
    }
    if (cflag) {
        printf("-c not implemented yet!\n");
    }
    if (dflag) {
        printf("-d not implemented yet!\n");
    }
    if (Fflag) {
        printf("-F not implemented yet!\n");
    }
    if (fflag) {
        printf("-f not implemented yet!\n");
    }
    if (hflag) {
        printf("-h not implemented yet!\n");
    }
    if (iflag) {
        printf("-i not implemented yet!\n");
    }
    if (kflag) {
        printf("-k not implemented yet!\n");
    }
    if (lflag) {
        printf("-l not implemented yet!\n");
    }
    if (nflag) {
        printf("-n not implemented yet!\n");
    }
    if (qflag) {
        printf("-q not implemented yet!\n");
    }
    if (Rflag) {
        printf("-R not implemented yet!\n");
    }
    if (rflag) {
        printf("-r not implemented yet!\n");
    }
    if (Sflag) {
        printf("-S not implemented yet!\n");
    }
    if (sflag) {
        printf("-s not implemented yet!\n");
    }
    if (tflag) {
        printf("-t not implemented yet!\n");
    }
    if (uflag) {
        printf("-u not implemented yet!\n");
    }
    if (wflag) {
        printf("-w not implemented yet!\n");
    }

    if (argv[0] == NULL) {
        argv[0] = ".";
    }

    print_traverse(argv);    
    return 0;
}
