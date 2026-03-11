#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#ifdef DEBUG
#define PDEBUG 1
#else
#define PDEBUG 0
#endif
#define dprint(fmt, ...) \
    do { if (PDEBUG) fprintf(stderr, "DEBUG--> %s:%d:%s(): " fmt, \
         __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (0)

struct dentry {
    char* dname;
    int  dlen;
    struct stat st;
    char owner[32];
    char group[32];
    char mtime[64];
    char type_char;
};

static int is_hidden(const char* name) { return name[0] == '.'; }

static void join_path(const char* dir, const char* name, char* out, size_t n) {
    size_t ld = strlen(dir);
    if (ld > 0 && dir[ld - 1] == '/')
        snprintf(out, n, "%s%s", dir, name);
    else
        snprintf(out, n, "%s/%s", dir, name);
}

static void fill_owner_group(struct dentry* e) {
    struct passwd* pw = getpwuid(e->st.st_uid);
    struct group* gr = getgrgid(e->st.st_gid);
    if (pw && pw->pw_name) snprintf(e->owner, sizeof e->owner, "%s", pw->pw_name);
    else snprintf(e->owner, sizeof e->owner, "%u", (unsigned)e->st.st_uid);
    if (gr && gr->gr_name) snprintf(e->group, sizeof e->group, "%s", gr->gr_name);
    else snprintf(e->group, sizeof e->group, "%u", (unsigned)e->st.st_gid);
}

static void fill_mtime(struct dentry* e) {
    struct tm* lt = localtime(&e->st.st_mtime);
    strftime(e->mtime, sizeof e->mtime, "%Y-%m-%d %H:%M", lt);
}

static char type_suffix(mode_t m) {
    if (S_ISDIR(m))  return '/';
    if (S_ISLNK(m))  return '@';
    if (S_ISFIFO(m)) return '|';
    if (m & (S_IXUSR | S_IXGRP | S_IXOTH)) return '*';
    return '\0';
}

static int cmp_dentry_by_name(const void* a, const void* b) {
    const struct dentry* ea = (const struct dentry*)a;
    const struct dentry* eb = (const struct dentry*)b;
    return strcmp(ea->dname, eb->dname);
}

struct dentry* readDirectory(const char* path, int* num, int show_hidden, int want_type)
{
    DIR* dp;
    struct dirent* de;
    int dcount = 0;
    struct dentry* data = NULL;
    int rcount = 0;

    dp = opendir(path);
    if (dp == NULL) {
        dprint("%s is not a valid path (%s)\n", path, strerror(errno));
        return NULL;
    }

    while ((de = readdir(dp)) != NULL) {
        if (!show_hidden && is_hidden(de->d_name)) continue;
        dcount++;
    }
    rewinddir(dp);

    data = (struct dentry*)malloc(dcount * sizeof(struct dentry));
    if (!data) { closedir(dp); return NULL; }

    while ((de = readdir(dp)) != NULL) {
        if (!show_hidden && is_hidden(de->d_name)) continue;

        dprint("found entry:[%s]\n", de->d_name);
        data[rcount].dname = strdup(de->d_name);
        data[rcount].dlen = (int)strlen(de->d_name);

        char full[Path_Max];
        join_path(path, de->d_name, full, sizeof full);
        if (lstat(full, &data[rcount].st) < 0) {
            memset(&data[rcount].st, 0, sizeof(struct stat));
            snprintf(data[rcount].owner, sizeof data[rcount].owner, "%s", "?");
            snprintf(data[rcount].group, sizeof data[rcount].group, "%s", "?");
            snprintf(data[rcount].mtime, sizeof data[rcount].mtime, "%s", "?");
            data[rcount].type_char = '\0';
        }
        else {
            fill_owner_group(&data[rcount]);
            fill_mtime(&data[rcount]);
            data[rcount].type_char = want_type ? type_suffix(data[rcount].st.st_mode) : '\0';
        }

        rcount++;
    }
    closedir(dp);

    *num = rcount;
    return data;
}

static void usage(const char* prog) {
    fprintf(stderr, "USAGE: %s [-aFs] <directory_path>\n", prog);
}

int main(int argc, char* argv[]) {
    int i, size;
    int opt_a = 0, opt_F = 0, opt_s = 0;
    int opt;
    while ((opt = getopt(argc, argv, "aFs")) != -1) {
        switch (opt) {
        case 'a': opt_a = 1; break;
        case 'F': opt_F = 1; break;
        case 's': opt_s = 1; break;
        default: usage(argv[0]); return -1;
        }
    }
    if (optind >= argc) { usage(argv[0]); return -1; }
    const char* path = argv[optind];

    dprint("Input Directory:%s\n", path);

    struct dentry* data = readDirectory(path, &size, opt_a, opt_F);
    if (!data) { perror("opendir/lstat"); return -1; }

    if (opt_s && size > 1) {
        qsort(data, size, sizeof(struct dentry), cmp_dentry_by_name);
    }

    printf("Directory listing for [%s]:\n", path);

    for (i = 0; i < size; i++) {
        char code = data[i].type_char ? data[i].type_char : ' ';
        long long bytes = (long long)data[i].st.st_size;

        printf("%-20s %c %10lld %-8s %-8s %s\n",
            data[i].dname,
            code,
            bytes,
            data[i].owner,
            data[i].group,
            data[i].mtime);
    }

    for (i = 0; i < size; i++) free(data[i].dname);
    free(data);

    dprint("Program done\n");
    return 0;
}







