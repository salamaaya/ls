#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "flags.h"
#include "print.h"
#include "utils.h"

/* Maximum buffer sizes used for formatted string. */
#define TIMEBUF_SZ 64
#define MODESTR_SZ 11 /* e.g. "drwxr-xr-x" + NUL */

void
humanize(off_t bytes)
{
    /* buf needs to be small enough to ensure humanize_number autoscales */
    char buf[5];

    if (humanize_number(buf, sizeof(buf), bytes, "", HN_AUTOSCALE,
        HN_B | HN_DECIMAL | HN_NOSPACE) < 0) {
            fprintf(stderr, "ls: humanize_number: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
    }

    printf("%s", buf);
}

void
print_file(char *file, char *path, const struct stat *sb, int flags)
{
    long blks;
    size_t i;

    if (sb == NULL) {
        fprintf(stderr, "ls: %s: %s\n", file, strerror(errno));
        return;
    }

    if (flags & FLAG_i) {
        printf("%ld ", sb->st_ino);
    }

    if (flags & FLAG_s) {
        blks = get_file_blk_size(sb);
        if (flags & FLAG_h) {
            humanize(sb->st_size);
            printf(" ");
        } else {
            if (flags & FLAG_k) {
                /* st_blocks are in units of 512 bytes, which is half a KB */
                blks = sb->st_blocks / 2;
            } 
            printf("%ld ", blks);
        }
    }

    if (!(flags & FLAG_w)) {
        /* remove non-printable characters */
        for (i = 0; i < strlen(file); i++) {
            if (!isprint((int)file[i])) {
                file[i] = '?';
            }
        }
    }

    if (flags & FLAG_l) {
        print_file_long(file, path, sb, flags);
    } else {
        printf("%s", file);
        if (flags & FLAG_F) {
            print_indicator(sb);
        }
    }

    printf("\n");
}

void
print_file_long(char *file, char *path, const struct stat *sb, int flags)
{
    char modes[MODESTR_SZ];
    char timebuf[TIMEBUF_SZ];
    const char *group, *owner;
    long nlink = (long)sb->st_nlink;
    struct passwd *pw;
    struct group *gr;
    struct tm tm;
    time_t time = sb->st_mtime;

    strmode(sb->st_mode, modes);

    pw = getpwuid(sb->st_uid);
    owner = pw ? pw->pw_name : NULL;

    gr = getgrgid(sb->st_gid);
    group = gr ? gr->gr_name : NULL;

    if (flags & FLAG_c) {
        time = sb->st_ctime;
    } else if (flags & FLAG_u) {
        time = sb->st_atime;
    }

    if (localtime_r(&time, &tm) == NULL) {
        memset(&tm, 0, sizeof(tm));
    }

    if (strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", &tm) == 0) {
        size_t n = strlcpy(timebuf, "???", sizeof(timebuf));
        if (n >= sizeof(timebuf)) {
            (void)fprintf(stderr, "stat: strlcpy: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (owner == NULL || (flags & FLAG_n)) {
        printf("%s %ld %u ", modes, nlink, (unsigned)sb->st_uid);
    } else {
        printf("%s %ld %s ", modes, nlink, owner);
    }

    if (group == NULL || (flags & FLAG_n)) {
        printf("%u ", (unsigned)sb->st_gid);
    } else {
        printf("%s ", group);
    }

    if (S_ISCHR(sb->st_mode)) {
        printf("%u, %u %s %s", major(sb->st_rdev), minor(sb->st_rdev), 
            timebuf, file);
    } else if (flags & FLAG_h) {
        humanize(sb->st_size);
        printf(" %s %s", timebuf, file);
    } else {
        printf("%lld %s %s", (long long)sb->st_size, timebuf, file);
    }

    if (flags & FLAG_F) {
        print_indicator(sb);
    }

    if (S_ISLNK(sb->st_mode)) {
        char filename[PATH_MAX], fullpath[PATH_MAX];
        ssize_t len;
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, file);
        if ((len = readlink(fullpath, filename, sizeof(filename))) < 0) {
            (void)fprintf(stderr, "ls: readlink: %s: %s\n", fullpath, strerror(errno));
            /*exit(EXIT_FAILURE);*/
            return;
        } else {
            filename[len] = '\0';
        }
        printf(" -> %s", filename);
    }
}

void
print_indicator(const struct stat *sb)
{
    if (S_ISDIR(sb->st_mode)) {
        printf("/");
    }

    if (S_ISWHT(sb->st_mode)) {
        printf("%c", '%');
    }

    if (S_ISSOCK(sb->st_mode)) {
        printf("=");
    }

    if (S_ISFIFO(sb->st_mode)) {
        printf("|");
    }

    if (S_ISLNK(sb->st_mode)) {
        printf("@");
    }

    if (S_ISREG(sb->st_mode) && (sb->st_mode & (S_IXUSR |S_IXGRP | S_IXOTH))) {
        printf("*");
    }
}
