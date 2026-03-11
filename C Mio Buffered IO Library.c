//mio.c code :
#include "mio.h"

MIO* mioin = NULL;
MIO* mioout = NULL;
MIO* mioerr = NULL;

int mio_min_i(int a, int b) { if (a < b) return a; else return b; }

int mio_mode_to_flags(int mode) {
    if (mode == MODE_R) return O_RDONLY;
    if (mode == MODE_WA) return O_WRONLY | O_CREAT | O_APPEND;
    if (mode == MODE_WT) return O_WRONLY | O_CREAT | O_TRUNC;
    return -1;
}

void mio_init_struct(MIO* m, int fd, int mode, int bsize) {
    m->fd = fd;
    m->rw = mode;

    if (mode == MODE_R && bsize > 0) {
        m->rsize = bsize;
        m->rb = (char*)malloc((size_t)m->rsize);
    }
    else {
        m->rsize = 0;
        m->rb = NULL;
    }
    m->rs = 0;
    m->re = 0;

    if (M_ISMW(mode) && bsize > 0) {
        m->wsize = bsize;
        m->wb = (char*)malloc((size_t)m->wsize);
    }
    else {
        m->wsize = 0;
        m->wb = NULL;
    }
    m->ws = 0;
    m->we = 0;
}

ssize_t mio_fill_readbuf(MIO* m) {
    ssize_t n = read(m->fd, m->rb, (size_t)m->rsize);
    if (n > 0) {
        m->rs = 0;
        m->re = (int)n;
    }
    return n;
}

MIO* myopen(const char* name, const int mode, const int bsize) {
    MIO* m = (MIO*)malloc(sizeof(MIO));
    if (!m) return NULL;

    int flags = mio_mode_to_flags(mode);
    if (flags < 0) { free(m); return NULL; }

    int fd = open(name, flags, 0644);
    if (fd < 0) { free(m); return NULL; }

    mio_init_struct(m, fd, mode, bsize);

    if ((mode == MODE_R && bsize > 0 && !m->rb) ||
        (M_ISMW(mode) && bsize > 0 && !m->wb)) {
        close(fd);
        if (m->rb) free(m->rb);
        if (m->wb) free(m->wb);
        free(m);
        return NULL;
    }
    return m;
}

MIO* mydopen(const int fd, const int mode, const int bsize) {
    if (fd < 0) return NULL;
    MIO* m = (MIO*)malloc(sizeof(MIO));
    if (!m) return NULL;

    mio_init_struct(m, fd, mode, bsize);

    if ((mode == MODE_R && bsize > 0 && !m->rb) ||
        (M_ISMW(mode) && bsize > 0 && !m->wb)) {
        if (m->rb) free(m->rb);
        if (m->wb) free(m->wb);
        free(m);
        return NULL;
    }
    return m;
}

int myclose(MIO* m) {
    int r = 0;
    if (!m) return -1;

    if (M_ISMW(m->rw) && m->wsize > 0 && m->we > m->ws) {
        if (myflush(m) < 0) r = -1;
    }
    if (close(m->fd) < 0) r = -1;

    if (m->rb) free(m->rb);
    if (m->wb) free(m->wb);
    free(m);

    if (r == 0) return 0;
    else return -1;
}

int myread(MIO* m, const char* b, const int size) {
    if (!m) return -1;
    if (m->rw != MODE_R) return -1;
    if (!b) return -1;
    if (size <= 0) return -1;

    if (m->rsize == 0) {
        ssize_t n = read(m->fd, (void*)b, (size_t)size);
        if (n == 0) return -1;
        if (n < 0) return -1;
        return (int)n;
    }

    int copied = 0;
    char* dst = (char*)b;
    while (copied < size) {
        if (m->rs >= m->re) {
            ssize_t n = mio_fill_readbuf(m);
            if (n <= 0) {
                if (copied > 0) return copied;
                else return -1;
            }
        }
        int avail = m->re - m->rs;
        int need = size - copied;
        int take = mio_min_i(avail, need);
        memcpy(dst + copied, m->rb + m->rs, (size_t)take);
        m->rs += take;
        copied += take;
    }
    return copied;
}

int mygetc(MIO* m, char* c) {
    if (!m) return -1;
    if (m->rw != MODE_R) return -1;
    if (!c) return -1;

    if (m->rsize == 0) {
        char ch;
        ssize_t n = read(m->fd, &ch, 1);
        if (n <= 0) return -1;
        *c = ch;
        return 1;
    }

    if (m->rs >= m->re) {
        ssize_t n = mio_fill_readbuf(m);
        if (n <= 0) return -1;
    }
    *c = m->rb[m->rs++];
    return 1;
}

char* mygets(MIO* m, int* len) {
    if (!m) return NULL;
    if (m->rw != MODE_R) return NULL;

    int cap = m->rsize > 0 ? m->rsize : MBSIZE;
    char* out = (char*)malloc((size_t)cap);
    if (!out) return NULL;

    int n = 0;
    char ch;
    int s;

    do {
        s = mygetc(m, &ch);
        if (s < 0) { free(out); return NULL; }
    } while (M_ISWS(ch));

    out[n++] = ch;

    while (1) {
        s = mygetc(m, &ch);
        if (s < 0) break;
        if (M_ISWS(ch)) break;
        if (n >= cap) {
            int ncap = cap * 2;
            char* tmp = (char*)realloc(out, (size_t)ncap);
            if (!tmp) { free(out); return NULL; }
            cap = ncap;
            out = tmp;
        }
        out[n++] = ch;
    }

    if (len) *len = n;
    char* res = (char*)malloc((size_t)n + 1);
    if (!res) { free(out); return NULL; }
    memcpy(res, out, (size_t)n);
    res[n] = '\0';
    free(out);
    return res;
}

char* mygetline(MIO* m, int* len) {
    if (!m) return NULL;
    if (m->rw != MODE_R) return NULL;

    int cap = m->rsize > 0 ? m->rsize : MBSIZE;
    char* buf = (char*)malloc((size_t)cap);
    if (!buf) return NULL;

    int n = 0;
    int got_any = 0;
    char ch;

    while (1) {
        int s = mygetc(m, &ch);
        if (s < 0) break;
        got_any = 1;
        if (ch == '\n') break;
        if (n >= cap) {
            int ncap = cap * 2;
            char* tmp = (char*)realloc(buf, (size_t)ncap);
            if (!tmp) { free(buf); return NULL; }
            cap = ncap;
            buf = tmp;
        }
        buf[n++] = ch;
    }

    if (!got_any && n == 0) { free(buf); return NULL; }

    if (n > 0 && buf[n - 1] == '\r') n--;

    char* out = (char*)malloc((size_t)n + 1);
    if (!out) { free(buf); return NULL; }
    memcpy(out, buf, (size_t)n);
    out[n] = '\0';
    free(buf);
    if (len) *len = n;
    return out;
}

int myflush(MIO* m) {
    if (!m) return -1;
    if (!M_ISMW(m->rw)) return -1;

    if (m->wsize == 0) return 0;

    int total = 0;
    while (m->ws < m->we) {
        ssize_t n = write(m->fd, m->wb + m->ws, (size_t)(m->we - m->ws));
        if (n <= 0) return -1;
        m->ws += (int)n;
        total += (int)n;
    }
    m->ws = 0;
    m->we = 0;
    return total;
}

int mywrite(MIO* m, const char* b, const int size) {
    if (!m) return -1;
    if (!M_ISMW(m->rw)) return -1;
    if (!b) return -1;
    if (size <= 0) return -1;

    if (m->wsize == 0) {
        ssize_t n = write(m->fd, b, (size_t)size);
        if (n < 0) return -1;
        return (int)n;
    }

    int written = 0;
    while (written < size) {
        int space = m->wsize - m->we;
        if (space == 0) {
            if (myflush(m) < 0) return -1;
            space = m->wsize - m->we;
        }
        int need = size - written;
        int take = mio_min_i(space, need);
        memcpy(m->wb + m->we, b + written, (size_t)take);
        m->we += take;
        written += take;
        if (m->we == m->wsize) {
            if (myflush(m) < 0) return -1;
        }
    }
    return written;
}

int myputc(MIO* m, const char c) {
    if (!m) return -1;
    if (!M_ISMW(m->rw)) return -1;

    if (m->wsize == 0) {
        ssize_t n = write(m->fd, &c, 1);
        if (n == 1) return 1;
        else return -1;
    }

    if (m->we == m->wsize) {
        if (myflush(m) < 0) return -1;
    }
    m->wb[m->we++] = c;
    if (m->we == m->wsize) {
        if (myflush(m) < 0) return -1;
    }
    return 1;
}

int myputs(MIO* m, const char* str, const int len) {
    if (!m) return -1;
    if (!M_ISMW(m->rw)) return -1;
    if (!str) return -1;
    if (len < 0) return -1;
    if (len == 0) return 0;
    return mywrite(m, str, len);
}

void myinit(void) {
    mioin = mydopen(STDIN_FILENO, MODE_R, MBSIZE);
    mioout = mydopen(STDOUT_FILENO, MODE_WA, 0);
    mioerr = mydopen(STDERR_FILENO, MODE_WA, 0);
}

//mio.h code :
#ifndef MIO_H_
#define MIO_H_

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dprint.h"

#define MBSIZE 10
#define MODE_R 0
#define MODE_WA 1
#define MODE_WT 2
#define MTAB '\t'
#define MNLINE '\n'
#define MCRET '\r'
#define MSPACE ' '

#define M_ISWS(X) ((X)==MTAB || (X)==MNLINE || (X)==MSPACE || (X)==MCRET)
#define M_ISMW(X) ((X)==MODE_WA || (X)==MODE_WT)

typedef struct _mio {
    int fd;
    int rw;
    char* rb, * wb;
    int rsize, wsize;
    int rs, re, ws, we;
} MIO;

extern MIO* mioin;
extern MIO* mioout;
extern MIO* mioerr;

MIO* myopen(const char* name, const int mode, const int bsize);
MIO* mydopen(const int fd, const int mode, const int bsize);
int   myclose(MIO* m);

int   myread(MIO* m, const char* b, const int size);
int   mygetc(MIO* m, char* c);
char* mygets(MIO* m, int* len);
char* mygetline(MIO* m, int* len);

int   mywrite(MIO* m, const char* b, const int size);
int   myflush(MIO* m);
int   myputc(MIO* m, const char c);
int   myputs(MIO* m, const char* str, const int len);

void  myinit(void);

#endif






