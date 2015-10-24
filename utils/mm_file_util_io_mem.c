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
#include <stdlib.h>
#include <string.h>

#include "mm_file_debug.h"
#include "mm_file_utils.h"


typedef struct {
	unsigned char *ptr;
	long long size;
	long long offset;
	int	state;
} MMFmemIOHandle;

static int mmf_mem_open(MMFileIOHandle *handle, const char *filename, int flags)
{
	MMFmemIOHandle *memHandle = NULL;
	char **splitedString = NULL;

	if (!handle || !filename || !handle->iofunc || !handle->iofunc->handleName) {
		debug_error("invalid param\n");
		return MMFILE_IO_FAILED;
	}

	filename += strlen(handle->iofunc->handleName) + 3; /* ://%d:%d means (memory addr:mem size)*/

	splitedString = mmfile_strsplit(filename, ":");
	if (splitedString == NULL) {
		debug_error("invalid param\n");
		return MMFILE_IO_FAILED;
	}

	if (!splitedString[0] || !splitedString[1]) {
		debug_error("invalid param\n");
		goto exception;
	}

	memHandle = mmfile_malloc(sizeof(MMFmemIOHandle));
	if (!memHandle) {
		debug_error("error: mmfile_malloc memHandle\n");
		goto exception;
	}

	memHandle->ptr = (unsigned char *)atoll(splitedString[0]);	/*memory allocation address changed. memHandle->ptr = (unsigned char*)atoi(splitedString[0]); */
	memHandle->size = atoi(splitedString[1]);
	memHandle->offset = 0;
	memHandle->state = 0;

	handle->privateData = (void *) memHandle;

	if (splitedString) {
		mmfile_strfreev(splitedString);
	}

	return MMFILE_IO_SUCCESS;

exception:

	if (splitedString) {
		mmfile_strfreev(splitedString);
	}

#if 0	/*dead code */
	if (memHandle) {
		mmfile_free(memHandle);
		handle->privateData  = NULL;
	}
#endif

	return MMFILE_IO_FAILED;
}

static int mmf_mem_read(MMFileIOHandle *h, unsigned char *buf, int size)
{
	MMFmemIOHandle *memHandle = NULL;
	const unsigned char *c = NULL;
	int len = 0;

	if (!h || !h->privateData || !buf) {
		debug_error("invalid para\n");
		return MMFILE_IO_FAILED;
	}

	memHandle = h->privateData;

	if (!memHandle->ptr) {
		debug_error("invalid para\n");
		return MMFILE_IO_FAILED;
	}

	c = memHandle->ptr + memHandle->offset;

	if (memHandle->state != EOF) {
		len = size;
		if (len + memHandle->offset > memHandle->size) {
			len = memHandle->size - memHandle->offset;
		}
	}

	memcpy(buf, c, len);

	memHandle->offset += len;

	if (memHandle->offset == memHandle->size) {
		memHandle->state = EOF;
	}

	return len;
}

static int mmf_mem_write(MMFileIOHandle *h, unsigned char *buf, int size)
{
	MMFmemIOHandle *memHandle = NULL;
	unsigned char *c = NULL;
	int len = 0;

	if (!h || !h->privateData || !buf) {
		debug_error("invalid para\n");
		return MMFILE_IO_FAILED;
	}

	memHandle = h->privateData;

	c = memHandle->ptr + memHandle->offset;

	if (memHandle->state != EOF) {
		len = size;
		if (len + memHandle->offset > memHandle->size) {
			len = memHandle->size - memHandle->offset;
		}
	}

	memcpy(c, buf, len);

	memHandle->offset += len;

	if (memHandle->offset == memHandle->size) {
		memHandle->state = EOF;
	}

	return len;
}


static int64_t mmf_mem_seek(MMFileIOHandle *h, int64_t pos, int whence)
{
	MMFmemIOHandle *memHandle = NULL;
	long tmp_offset = 0;

	if (!h || !h->privateData) {
		debug_error("invalid para\n");
		return MMFILE_IO_FAILED;
	}

	memHandle = h->privateData;

	switch (whence) {
		case MMFILE_SEEK_SET:
			tmp_offset = 0 + pos;
			break;
		case MMFILE_SEEK_CUR:
			tmp_offset = memHandle->offset + pos;
			break;
		case MMFILE_SEEK_END:
			tmp_offset = memHandle->size + pos;
			break;
		default:
			return MMFILE_IO_FAILED;
	}

	/*check validation*/
	if (tmp_offset < 0) {
		debug_error("invalid file offset\n");
		return MMFILE_IO_FAILED;
	}

	/*set */
	memHandle->state = (tmp_offset >= memHandle->size) ? EOF : !EOF;
	memHandle->offset = tmp_offset;

	return memHandle->offset;
}


static long long mmf_mem_tell(MMFileIOHandle *h)
{
	MMFmemIOHandle *memHandle = NULL;

	if (!h || !h->privateData) {
		debug_error("invalid para\n");
		return MMFILE_IO_FAILED;
	}

	memHandle = h->privateData;

	return memHandle->offset;
}

static int mmf_mem_close(MMFileIOHandle *h)
{
	MMFmemIOHandle *memHandle = NULL;

	if (!h || !h->privateData) {
		debug_error("invalid para\n");
		return MMFILE_IO_FAILED;
	}

	memHandle = h->privateData;

	if (memHandle) {
		mmfile_free(memHandle);
		h->privateData = NULL;
	}

	return MMFILE_IO_SUCCESS;
}


MMFileIOFunc mmfile_mem_io_handler = {
	"mem",
	mmf_mem_open,
	mmf_mem_read,
	mmf_mem_write,
	mmf_mem_seek,
	mmf_mem_tell,
	mmf_mem_close,
	NULL
};
