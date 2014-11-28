/*
 *  This file is part of Joe's Own Editor for Windows.
 *  Copyright (c) 2014 John J. Jordan.
 *
 *  Joe's Own Editor for Windows is free software: you can redistribute it 
 *  and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, either version 2 of the 
 *  License, or (at your option) any later version.
 *
 *  Joe's Own Editor for Windows is distributed in the hope that it will
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Joe's Own Editor for Windows.  If not, see
 *  <http://www.gnu.org/licenses/>.
 */

#include "jwwin.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "jwbuiltin.h"
#include "lzf.h"

static HMODULE modules[8];
static int nmodules = 0;

static int iscompressed(const unsigned char* p);
static int decompress(const unsigned char* p, int len, unsigned char **result, int *resultlen);

void jwAddResourceHandle(HMODULE module)
{
	modules[nmodules++] = module;
}

JFILE *jwfopen(wchar_t *name, wchar_t *mode)
{
	if (name[0] == L'*') {
		wchar_t resname[64];
		int i;

		wcscpy(resname, L"F:");
		wcscat(resname, name);

		for (i = 0; i < nmodules; i++) {
			HRSRC res = FindResource(modules[i], resname, RT_RCDATA);
			if (res) {
				HGLOBAL resptr = LoadResource(modules[i], res);
				unsigned char *ptr = (unsigned char *)LockResource(resptr);
				JFILE *j = (JFILE *)malloc(sizeof(JFILE));

				j->f = 0;
				j->sz = SizeofResource(modules[i], res);

				/* Courtesy of liblzf (wrappers below) */
				if (iscompressed((unsigned char *)ptr)) {
					unsigned char *newptr;
					int newlen;

					if (!decompress((unsigned char *)ptr, j->sz, &newptr, &newlen)) {
						/* Success */
						j->dealloc = newptr;
						j->p = newptr;
						j->sz = newlen;
					} else {
						free(j);
						return NULL;
					}
				} else {
					j->p = ptr;
					j->dealloc = 0;
				}

				return j;
			}
		}

		return 0;
	} else {
		FILE *f = _wfopen(name, mode);
		if (f) {
			JFILE *j = (JFILE *)malloc(sizeof(JFILE));
			j->f = f;
			j->p = 0;
			j->dealloc = 0;
			return j;
		} else {
			return 0;
		}
	}
}

unsigned char *jwfgets(unsigned char *buf, int size, JFILE *f)
{
	int x;

	if (f->f) return (unsigned char *)fgets((char*)buf, size, f->f);
	if (!f->sz) {
		buf[0] = 0;
		return NULL;
	}

	for (x = 0; x < f->sz && x < size && f->p[x] != '\n'; x++) {
		buf[x] = f->p[x];
	}
	
	if (x < f->sz && x < size) {
		// Include the newline.
		buf[x] = f->p[x];
		x++;
	}

	buf[x] = 0;

	f->sz -= x;
	f->p += x;

	return buf;
}

static BOOL CALLBACK RecordResource(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, LONG_PTR lParam)
{
	wchar_t *str = (wchar_t*)lParam;

	if (!wcsncmp(L"F:*", lpszName, 3)) {
		wcscat(str, &lpszName[3]);
		wcscat(str, L";");
	}

	return TRUE;
}

/* This operates off of a static string array of the form:
 *    str1\0str2\0str3\0\0
 * which is generated by first making a string like:
 *    str1;str2;str3;\0
 * dup()'ing it and then replacing the semis with nulls.
 *
 * Callers should not write to it and we don't care if (when) it leaks.
 */
const wchar_t *jwnextbuiltin(const wchar_t* prev, const wchar_t* suffix)
{
	static wchar_t *allbuiltins = NULL;
	const wchar_t *next;
	int slen;

	if (!allbuiltins) {
		wchar_t tmp[1024];
		wchar_t *ptr;
		int i;

		wcscpy(tmp, L"");
		
		for (i = 0; i < nmodules; i++) {
			EnumResourceNames(modules[i], RT_RCDATA, RecordResource, (LONG_PTR)(void*)tmp);
		}

		ptr = wcsdup(tmp);
		for (i = 0; ptr[i]; i++) {
			if (ptr[i] == L';') ptr[i] = 0;
		}

		allbuiltins = ptr;
	}

	if (!prev) {
		next = allbuiltins;
	} else {
		next = &prev[wcslen(prev) + 1];
	}

	if (suffix) {
		slen = wcslen(suffix);
	}

	for (; *next; next += wcslen(next) + 1) {
		if (!suffix) {
			return next;
		} else {
			int nlen = wcslen(next);

			if (slen <= nlen && !wcsicmp(suffix, &next[nlen - slen])) {
				return next;
			}
		}
	}

	return NULL;
}

int jwfclose(JFILE *f)
{
	int rtn = 0;
	if (f->f)
		rtn = fclose(f->f);
	else if (f->dealloc)
		free(f->dealloc);
	free(f);
	return rtn;
}


/**** Wrappers for liblzf ****/

static int iscompressed(const unsigned char* p)
{
	return p[0] == 'Z' && p[1] == 'V' && (p[2] == 0 || p[2] == 1);
}

#define READ_SHORT(x) ((p[i + x] << 8) | p[i + x + 1])

static int decompress(const unsigned char* p, int len, unsigned char **result, int *resultlen)
{
	int i = 0, t = 0;
	size_t sz = 0;
	unsigned char *data;

	/* Extract the length */
	while (i < len) {
		if (p[i] == 'Z' && p[i + 1] == 'V') {
			if (p[i + 2] == 0) {
				/* Uncompressed block */
				int blocksz = READ_SHORT(3);

				i += blocksz + 5;
				sz += blocksz;
			} else if (p[i + 2] == 1) {
				/* Compressed block */
				sz += READ_SHORT(5);
				i += READ_SHORT(3) + 7;
			} else {
				assert(0);
				return 1;
			}
		} else {
			/* Out of sync. error */
			assert(0);
			return 1;
		}
	}

	data = (unsigned char *)malloc(sz);
	
	/* Process each block */
	for (i = 0, t = 0; i < len;) {
		if (p[i + 2] == 0) {
			/* Uncompressed block */
			int blocksz = READ_SHORT(3);

			memcpy(&data[t], &p[i + 5], blocksz);
			i += blocksz + 5;
			t += blocksz;
		} else if (p[i + 2] == 1) {
			/* Compressed block */
			int cmpsz = READ_SHORT(3);
			int ucmpsz = READ_SHORT(5);

			if (!lzf_decompress(&p[i + 7], cmpsz, &data[t], ucmpsz)) {
				/* Error */
				assert(0);
				free(data);
				return 1;
			}

			i += cmpsz + 7;
			t += ucmpsz;
		}
	}

	*result = data;
	*resultlen = (int)sz;
	return 0;
}

