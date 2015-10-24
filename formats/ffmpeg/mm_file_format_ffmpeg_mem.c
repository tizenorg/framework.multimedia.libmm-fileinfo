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

#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include "mm_file_debug.h"
#include "mm_file_utils.h"
#include "mm_file_format_ffmpeg_mem.h"


typedef struct {
	unsigned char *ptr;
	long long size;
	long long offset;
	int	state;
} MMFmemIOHandle;

static int mmf_mem_open(URLContext *handle, const char *filename, int flags)
{
	MMFmemIOHandle *memHandle = NULL;
	char **splitedString = NULL;


	if (!handle || !filename || !handle->prot) {
		debug_error("invalid param\n");
		return MMFILE_UTIL_FAIL;
	}

	filename += strlen(handle->prot->name) + 3; /* ://%d:%d means (memory addr:mem size)*/

	splitedString = mmfile_strsplit(filename, ":");
	if (splitedString == NULL) {
		debug_error("invalid param\n");
		return MMFILE_UTIL_FAIL;
	}

	if (!splitedString[0] || !splitedString[1])

	{
		debug_error("invalid param\n");
		goto exception;
	}

	memHandle = mmfile_malloc(sizeof(MMFmemIOHandle));
	if (!memHandle) {
		debug_error("error: mmfile_malloc memHandle\n");
		goto exception;
	}

	memHandle->ptr = (unsigned char *)atoll(splitedString[0]);	/*memory allocation address changed. memHandle->ptr = (unsigned char *) atoi(splitedString[0]); */
	memHandle->size = atoi(splitedString[1]);
	memHandle->offset = 0;
	memHandle->state = 0;

	handle->priv_data = (void *) memHandle;

	/* Imp to reset them otherwise file seek will fail */
	handle->is_streamed = 0; /*FALSE*/
	handle->max_packet_size = 0;

	if (splitedString) {
		mmfile_strfreev(splitedString);
	}

	return MMFILE_UTIL_SUCCESS;

exception:

	if (splitedString) {
		mmfile_strfreev(splitedString);
	}

#if 0	/*dead code */
	if (memHandle) {
		mmfile_free(memHandle);
		handle->priv_data  = NULL;
	}
#endif

	return MMFILE_UTIL_FAIL;
}

static int mmf_mem_read(URLContext *h, unsigned char *buf, int size)
{
	MMFmemIOHandle *memHandle = NULL;
	const unsigned char *c = NULL;
	int len = 0;


	if (!h || !h->priv_data || !buf) {
		debug_error("invalid para\n");
		return MMFILE_UTIL_FAIL;
	}

	memHandle = h->priv_data;

	if (!memHandle->ptr) {
		debug_error("invalid para\n");
		return MMFILE_UTIL_FAIL;
	}

	if (memHandle->offset >= memHandle->size) {
		/* for some file formats last file read */
		debug_error("File Read is beyond the file Size\n");
		return MMFILE_UTIL_FAIL;

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

static int mmf_mem_write(URLContext *h, const unsigned char *buf, int size)
{
	if (!h || !h->priv_data || !buf) {
		debug_error("invalid para\n");
		return MMFILE_UTIL_FAIL;
	}

	debug_error("NOTE PERMITTED\n");
	return MMFILE_UTIL_FAIL;
}


static int64_t mmf_mem_seek(URLContext *h, int64_t pos, int whence)
{
	MMFmemIOHandle *memHandle = NULL;
	long long tmp_offset = 0;


	if (!h || !h->priv_data) {
		debug_error("invalid para\n");
		return MMFILE_UTIL_FAIL;
	}

	memHandle = h->priv_data;

	switch (whence) {
		case SEEK_SET:
			tmp_offset = 0 + pos;
			break;
		case SEEK_CUR:
			tmp_offset = memHandle->offset + pos;
			break;
		case SEEK_END:
			tmp_offset = memHandle->size + pos;
			break;
		case AVSEEK_SIZE:	/*FFMPEG specific*/
			return memHandle->size;
		default:
			return MMFILE_UTIL_FAIL;
	}

	/*check validation*/
	if (tmp_offset < 0 && tmp_offset > memHandle->size) {
		debug_error("invalid file offset\n");
		return MMFILE_UTIL_FAIL;
	}

	/*set */
	memHandle->state = (tmp_offset >= memHandle->size) ? EOF : !EOF;
	memHandle->offset = (unsigned int) tmp_offset;

	return tmp_offset;
}

static int mmf_mem_close(URLContext *h)
{
	MMFmemIOHandle *memHandle = NULL;

	if (!h || !h->priv_data) {
		debug_error("invalid para\n");
		return MMFILE_UTIL_FAIL;
	}

	memHandle = h->priv_data;

	if (memHandle) {
		mmfile_free(memHandle);
		h->priv_data = NULL;
	}

	return MMFILE_UTIL_SUCCESS;
}


URLProtocol MMFileMEMProtocol  = {
	.name		= "mem",
	.url_open	= mmf_mem_open,
	.url_read	= mmf_mem_read,
	.url_write	= mmf_mem_write,
	.url_seek	= mmf_mem_seek,
	.url_close	= mmf_mem_close,
};
