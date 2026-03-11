#include "mio.h"

static int min_i(int a, int b) { return a < b ? a : b; }

MIO* myopen(const char* name, const int mode) {
	MIO* m = (MIO*)malloc(sizeof(MIO));
	if (!m) return NULL;
	int flags = 0;
	if (mode == MODE_R) flags = O_RDONLY;
	else if (mode == MODE_WA) flags = O_WRONLY | O_CREAT | O_APPEND;
	else if (mode == MODE_WT) flags = O_WRONLY | O_CREAT | O_TRUNC;
	else { free(m); return NULL; }
	int fd = open(name, flags, 0644);
	if (fd < 0) { free(m); return NULL; }
	m->fd = fd;
	m->rw = mode;
	m->rb = NULL; m->wb = NULL;
	m->rsize = 0; m->wsize = 0;
	m->rs = 0; m->re = 0; m->ws = 0; m->we = 0;
	if (mode == MODE_R) {
		m->rsize = MBSIZE;
		m->rb = (char*)malloc(m->rsize);
		if (!m->rb) { close(fd); free(m); return NULL; }
	}
	else {
		m->wsize = MBSIZE;
		m->wb = (char*)malloc(m->wsize);
		if (!m->wb) { close(fd); free(m); return NULL; }
	}
	return m;
}

int myclose(MIO* m) {
	if (!m) return -1;
	int r = 0;
	if (M_ISMW(m->rw) && m->we > m->ws) {
		if (myflush(m) < 0) r = -1;
	}
	if (close(m->fd) < 0) r = -1;
	if (m->rb) free(m->rb);
	if (m->wb) free(m->wb);
	free(m);
	return r == 0 ? 0 : -1;
}

static ssize_t fill_readbuf(MIO* m) {
	ssize_t n = read(m->fd, m->rb, m->rsize);
	if (n > 0) { m->rs = 0; m->re = (int)n; }
	return n;
}

int myread(MIO* m, const char* b, const int size) {
	if (!m || m->rw != MODE_R || !b || size <= 0) return -1;
	int copied = 0;
	char* dst = (char*)b;
	while (copied < size) {
		if (m->rs >= m->re) {
			ssize_t n = fill_readbuf(m);
			if (n <= 0) return copied > 0 ? copied : -1;
		}
		int avail = m->re - m->rs;
		int need = size - copied;
		int take = min_i(avail, need);
		memcpy(dst + copied, m->rb + m->rs, (size_t)take);
		m->rs += take;
		copied += take;
	}
	return copied;
}

int mygetc(MIO* m, char* c) {
	if (!m || m->rw != MODE_R || !c) return -1;
	if (m->rs >= m->re) {
		ssize_t n = fill_readbuf(m);
		if (n <= 0) return -1;
	}
	*c = m->rb[m->rs++];
	return 1;
}

char* mygets(MIO* m, int* len) {
	if (!m || m->rw != MODE_R) return NULL;
	int cap = MBSIZE;
	char* out = (char*)malloc(cap);
	if (!out) return NULL;
	int n = 0;
	char ch;
	int s;
	do {
		s = mygetc(m, &ch);
		if (s < 0) { free(out); return NULL; }
	} while (M_ISWS(ch));
	out[n++] = ch;
	for (;;) {
		s = mygetc(m, &ch);
		if (s < 0 || M_ISWS(ch)) break;
		if (n >= cap) {
			int ncap = cap * 2;
			char* tmp = (char*)realloc(out, ncap);
			if (!tmp) { free(out); return NULL; }
			out = tmp; cap = ncap;
		}
		out[n++] = ch;
	}
	if (len) *len = n;
	char* res = (char*)malloc(n + 1);
	if (!res) { free(out); return NULL; }
	memcpy(res, out, (size_t)n);
	res[n] = '\0';
	free(out);
	return res;
}

int myflush(MIO* m) {
	if (!m || !M_ISMW(m->rw)) return -1;
	int total = 0;
	while (m->ws < m->we) {
		ssize_t n = write(m->fd, m->wb + m->ws, (size_t)(m->we - m->ws));
		if (n <= 0) return -1;
		m->ws += (int)n;
		total += (int)n;
	}
	m->ws = 0; m->we = 0;
	return total;
}

int mywrite(MIO* m, const char* b, const int size) {
	if (!m || !M_ISMW(m->rw) || !b || size <= 0) return -1;
	int written = 0;
	while (written < size) {
		int space = m->wsize - m->we;
		if (space == 0) {
			if (myflush(m) < 0) return -1;
			space = m->wsize - m->we;
		}
		int need = size - written;
		int take = min_i(space, need);
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
	if (!m || !M_ISMW(m->rw)) return -1;
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
	if (!m || !M_ISMW(m->rw) || !str || len < 0) return -1;
	if (len == 0) return 0;
	return mywrite(m, str, len);
}






