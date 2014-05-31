/*
 * libmm-fileinfo
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Haejeong Kim <backto.kim@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mm_file_utils.h"

#ifdef __MMFILE_MEM_TRACE__
EXPORT_API
void *mmfile_malloc_debug (unsigned int size, const char *func, unsigned int line)
{
	void *tmp = malloc (size);

	if (tmp) {
		memset (tmp, 0x00, size);
		#ifdef __MMFILE_TEST_MODE__
		debug_msg("## DEBUG ## %p = malloc (%d) by %s() %d\n", tmp, size, func, line);
		#endif
	}
	return tmp;
}

EXPORT_API
void *mmfile_calloc_debug (unsigned int nmemb, unsigned int size, const char *func, unsigned int line)
{
	void *tmp = calloc (nmemb, size);

	if (tmp) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg("## DEBUG ## %p = calloc (%d, %d) by %s() %d\n", tmp, nmemb, size, func, line);
		#endif
	}
	return tmp;
}

EXPORT_API
void mmfile_free_debug (void *ptr, const char *func, unsigned int line)
{
	if (ptr) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg("## DEBUG ## free (%p) by %s() %d\n", ptr, func, line);
		#endif
		free (ptr);
	}
}


EXPORT_API
void *mmfile_realloc_debug (void *ptr, unsigned int size, const char *func, unsigned int line)
{
	void *tmp = realloc (ptr, size);

	if (tmp) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg("## DEBUG ## %p = realloc (%p, %d) by %s() %d\n", tmp, ptr, size, func, line);
		#endif
	}
	return tmp;
}

EXPORT_API
void *mmfile_memset_debug (void *s, int c, unsigned int n, const char *func, unsigned int line)
{
	#ifdef __MMFILE_TEST_MODE__
	debug_msg("## DEBUG ## memset (%p, %d, %d) by %s() %d\n", s, c, n, func, line);
	#endif
	return memset (s, c, n);
}

EXPORT_API
void *mmfile_memcpy_debug (void *dest, const void *src, unsigned int n, const char *func, unsigned int line)
{
	#ifdef __MMFILE_TEST_MODE__
	debug_msg("## DEBUG ## memcpy (%p, %p, %d) by %s() %d\n", dest, src, n, func, line);
	#endif
	return memcpy (dest, src, n);
}

#else   /* __MMFILE_MEM_TRACE__ : ------------------------------------------------------------------*/

EXPORT_API
void *mmfile_malloc (unsigned int size)
{
    void *tmp = malloc (size);
    if (tmp)
    {
        memset (tmp, 0x00, size);
    }
    return tmp;
}

EXPORT_API
void *mmfile_calloc (unsigned int nmemb, unsigned int size)
{
    void *tmp = calloc (nmemb, size);
    return tmp;
}

EXPORT_API
void mmfile_free_r (void *ptr)
{
    if (ptr) free (ptr);
}

EXPORT_API
void *mmfile_realloc (void *ptr, unsigned int size)
{
    return realloc (ptr, size);
}

EXPORT_API
void *mmfile_memset (void *s, int c, unsigned int n)
{
    return memset (s, c, n);
}

EXPORT_API
void *mmfile_memcpy (void *dest, const void *src, unsigned int n)
{
    return memcpy (dest, src, n);
}
#endif

