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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm_file_debug.h"
#include "mm_file_utils.h"

typedef struct mmfileiodata {
	int fd;
	long long offset;
} tMMFORMAT_FILEIO_DATA;


static int file_open(MMFileIOHandle *handle, const char *filename, int flags)
{
	tMMFORMAT_FILEIO_DATA *privateData = NULL;
	int access = 0;
	int fd = 0;

	if (!handle || !filename) {
		debug_error("invalid param\n");
		return MMFILE_IO_FAILED;
	}

	filename += strlen(handle->iofunc->handleName) + 3; /* :// */

	if (flags & MMFILE_RDWR) {
		access = O_CREAT | O_TRUNC | O_RDWR;
	} else if (flags & MMFILE_WRONLY) {
		access = O_CREAT | O_TRUNC | O_WRONLY;
	} else {
		access = O_RDONLY;
	}

#ifdef O_BINARY
	access |= O_BINARY;
#endif

	fd = open(filename, access, 0666);
	if (fd < 0) {
		debug_error("open error\n");
		return MMFILE_IO_FAILED;
	}

	privateData = mmfile_malloc(sizeof(tMMFORMAT_FILEIO_DATA));
	if (!privateData) {
		close(fd);
		debug_error("calloc privateData\n");
		return MMFILE_IO_FAILED;
	}

	privateData->fd = fd;
	privateData->offset = 0;

	handle->privateData = (void *)privateData;
	return MMFILE_IO_SUCCESS;
}

static int file_read(MMFileIOHandle *handle, unsigned char *buf, int size)
{
	tMMFORMAT_FILEIO_DATA *privateData = handle->privateData;
	int readSize = 0;

	readSize = read(privateData->fd, buf, size);
	if (readSize < 0) {
		debug_error("read\n");
		return MMFILE_IO_FAILED;
	}

	privateData->offset += readSize;

	return readSize;
}

static int file_write(MMFileIOHandle *handle, unsigned char *buf, int size)
{
	tMMFORMAT_FILEIO_DATA *privateData = handle->privateData;
	int writtenSize = 0;

	writtenSize = write(privateData->fd, buf, size);
	if (writtenSize < 0) {
		debug_error("write\n");
		return MMFILE_IO_FAILED;
	}

	privateData->offset += writtenSize;

	return writtenSize;
}

static int64_t file_seek(MMFileIOHandle *handle, int64_t pos, int whence)
{
	tMMFORMAT_FILEIO_DATA *privateData = handle->privateData;
	privateData->offset = lseek(privateData->fd, pos, whence);
	return privateData->offset;
}

static long long file_tell(MMFileIOHandle *handle)
{
	tMMFORMAT_FILEIO_DATA *privateData = handle->privateData;

	return privateData->offset;
}


static int file_close(MMFileIOHandle *handle)
{
	tMMFORMAT_FILEIO_DATA *privateData = handle->privateData;
	/*int ret = 0;*/

	if (privateData) {
		/*ret = */close(privateData->fd);
		mmfile_free(privateData);
		handle->privateData = NULL;
		return MMFILE_IO_SUCCESS;
	}

	return MMFILE_IO_FAILED;
}


MMFileIOFunc mmfile_file_io_handler = {
	"file",
	file_open,
	file_read,
	file_write,
	file_seek,
	file_tell,
	file_close,
	NULL
};

