// This file is part of the casycom project
//
// Copyright (c) 2015 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "util.h"
#include <stdarg.h>
#if HAVE_EXECINFO_H
    #include <execinfo.h>
#endif

//{{{ Memory management ------------------------------------------------

void* xalloc (size_t sz)
{
    void* p = xrealloc (NULL, sz);
    memset (p, 0, sz);
    return p;
}

void* xrealloc (void* p, size_t sz)
{
    p = realloc (p, sz);
    if (!p) {
	casycom_log (LOG_ERR, "out of memory");
	exit (EXIT_FAILURE);
    }
    return p;
}

//}}}-------------------------------------------------------------------
//{{{ Debugging

#ifndef NDEBUG

void casycom_log (int type, const char* fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    if (isatty (STDOUT_FILENO))
	vprintf (fmt, args);
    else
	vsyslog (type, fmt, args);
    va_end (args);
}

void casycom_backtrace (void)
{
#if HAVE_EXECINFO_H
    void* frames [64];
    int nFrames = backtrace (ArrayBlock(frames));
    if (nFrames <= 1)
	return;	// Can happen if there is no debugging information
    char** syms = backtrace_symbols (frames, nFrames);
    if (!syms)
	return;
    for (int i = 1; i < nFrames; ++i)
	casycom_log (LOG_ERR, "\t%s\n", syms[i]);
    free (syms);
#endif
}

#endif

#ifndef UC_VERSION
static inline char _num_to_digit (uint8_t b)
{
    char d = (b & 0xF) + '0';
    return d <= '9' ? d : d+('A'-'0'-10);
}
static inline bool _printable (char c)
{
    return c >= 32 && c < 127;
}
void hexdump (const void* vp, size_t n)
{
    const uint8_t* p = vp;
    char line[65]; line[64] = 0;
    for (size_t i = 0; i < n; i += 16) {
	memset (line, ' ', sizeof(line)-1);
	for (size_t h = 0; h < 16; ++h) {
	    if (i+h < n) {
		uint8_t b = p[i+h];
		line[h*3] = _num_to_digit(b>>4);
		line[h*3+1] = _num_to_digit(b);
		line[h+3*16] = _printable(b) ? b : '.';
	    }
	}
	puts (line);
    }
}
#endif

//}}}-------------------------------------------------------------------
//{{{ vector

void vector_reserve (void* vv, size_t sz)
{
    CharVector* v = vv;
    if (v->allocated >= sz)
	return;
    size_t nsz = v->allocated + !v->allocated;
    while (nsz < sz)
	nsz *= 2;
    v->d = xrealloc (v->d, nsz * v->elsize);
    memset (v->d + v->allocated * v->elsize, 0, (nsz - v->allocated) * v->elsize);
    v->allocated = nsz;
}

void vector_deallocate (void* vv)
{
    CharVector* v = vv;
    xfree (v->d);
    v->size = 0;
    v->allocated = 0;
}

void* vector_emplace (void* vv, size_t ip)
{
    CharVector* v = vv;
    assert (ip <= v->size && "out of bounds insert");
    vector_reserve (vv, v->size+1);
    char* ii = v->d + ip * v->elsize;
    memmove (ii + v->elsize, ii, (v->size-ip)*v->elsize);
    memset (ii, 0, v->elsize);
    ++v->size;
    return ii;
}

void* vector_emplace_n (void* vv, size_t ip, size_t n)
{
    CharVector* v = vv;
    assert (ip <= v->size && "out of bounds insert");
    vector_reserve (vv, v->size+n);
    char* ii = v->d + ip * v->elsize;
    memmove (ii + n*v->elsize, ii, (v->size-ip)*v->elsize);
    memset (ii, 0, n*v->elsize);
    v->size += n;
    return ii;
}

void vector_insert (void* vv, size_t ip, const void* e)
{
    CharVector* v = vv;
    void* h = vector_emplace (v, ip);
    memcpy (h, e, v->elsize);
}

void vector_insert_n (void* vv, size_t ip, const void* e, size_t esz)
{
    CharVector* v = vv;
    void* h = vector_emplace_n (v, ip, esz);
    memcpy (h, e, esz*v->elsize);
}

void vector_erase_n (void* vv, size_t ep, size_t n)
{
    CharVector* v = vv;
    char *ei = v->d + ep*v->elsize, *eei = v->d + (ep+n)*v->elsize;
    assert (ep <= v->size && ep+n <= v->size && "out of bounds erase");
    memmove (ei, eei, (v->size - (ep+n)) * v->elsize);
    v->size -= n;
}

void vector_swap (void* vv1, void* vv2)
{
    CharVector *v1 = vv1, *v2 = vv2;
    assert (v1->elsize == v2->elsize && "can only swap identical vectors");
    CharVector t;
    memcpy (&t, v1, sizeof(t));
    memcpy (v1, v2, sizeof(*v1));
    memcpy (v2, &t, sizeof(*v2));
}

void vector_copy (void* vv1, const void* vv2)
{
    CharVector* v1 = vv1;
    const CharVector* v2 = vv2;
    vector_resize (v1, v2->size);
    memcpy (v1->d, v2->d, v2->size);
}

static size_t _vector_bound (const void* vv, vector_compare_fn_t cmp, const int cmpv, const void* e)
{
    const CharVector* v = vv;
    size_t f = 0, l = v->size;
    while (f < l) {
	size_t m = (f+l)/2;
	if (cmpv > cmp (v->d + m*v->elsize, e))
	    f = m + 1;
	else
	    l = m;
    }
    return f;
}
size_t vector_lower_bound (const void* vv, vector_compare_fn_t cmp, const void* e)
    { return _vector_bound (vv, cmp, 0, e); }
size_t vector_upper_bound (const void* vv, vector_compare_fn_t cmp, const void* e)
    { return _vector_bound (vv, cmp, 1, e); }

//}}}-------------------------------------------------------------------
//{{{ Miscellaneous

unsigned sd_listen_fds (void)
{
    const char* e = getenv("LISTEN_PID");
    if (!e || getpid() != (pid_t) strtoul(e, NULL, 10))
	return 0;
    e = getenv("LISTEN_FDS");
    return e ? strtoul (e, NULL, 10) : 0;
}

const char* find_exe_in_path (const char* exe, char* exepath, size_t exepathlen)
{
    const char* pathenv = getenv("PATH");
    if (!pathenv)
	pathenv = "/usr/bin";
    char path [PATH_MAX];
    char c, *pend = path;
    do {
	*pend++ = ((c = *pathenv++) == ':' ? 0 : c);
    } while (c);
    for (const char *p = path; p < pend; p = strnext(p)) {
	snprintf (exepath, exepathlen, "%s/%s", p, exe);
	if (access (exepath, X_OK) == 0)
	    break;
	exepath[0] = 0;
    }
    return exepath[0] ? exepath : NULL;
}

//}}}-------------------------------------------------------------------
