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

/* Maximum buffer sizes used for formatted string. */
#define TIMEBUF_SZ 64
#define MODESTR_SZ 11 /* e.g. "drwxr-xr-x" + NUL */

void
print_file(const char *path, const struct stat *sb, int flags)
{
    if (sb == NULL) {
        fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
        return;
    }

    if (flags & FLAG_i) {
        printf("%ld ", sb->st_ino);
    }
    
    if (flags & FLAG_l) {
        print_file_long(path, sb, flags);
    } else {
        printf("%s", path);
    }

    if (flags & FLAG_F) {
        print_indicator(path, sb);
    }
    printf("\n");
}

void print_file_long(const char *path, const struct stat *sb, int flags)
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

    printf("%lld %s %s", (long long)sb->st_size, timebuf, path);
}

void
print_indicator(const char *path, const struct stat *sb)
{
    if (S_ISDIR(sb->st_mode)) {
        printf("/");
    }

    if (S_ISLNK(sb->st_mode)) {
        char filename[PATH_MAX];
        if (readlink(path, filename, sizeof(filename)) < 0) {
            fprintf(stderr, "stat: readlink: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        printf("@ -> %s", filename);
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

    if (S_ISREG(sb->st_mode) && (sb->st_mode & (S_IXUSR |S_IXGRP | S_IXOTH))) {
        printf("*");
    }
}
