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
#include <ctype.h>
#include "mm_debug.h"
#include "mm_file_utils.h"

static unsigned char is_little_endian = 0;

inline static int _is_little_endian (void)
{
	int i = 0x00000001;
	return ((char *)&i)[0];
}

EXPORT_API
inline unsigned int mmfile_io_be_uint32 (unsigned int value)
{
	return (is_little_endian == 0) ? value : ((unsigned int)((((value)&0xFF000000)>>24) | (((value)&0x00FF0000)>>8) | (((value)&0x0000FF00)<<8) | (((value)&0x000000FF)<<24)));
}

EXPORT_API
inline unsigned int mmfile_io_le_uint32 (unsigned int value)
{
	return (is_little_endian == 1) ? value : ((unsigned int)((((value)&0xFF000000)>>24) | (((value)&0x00FF0000)>>8) | (((value)&0x0000FF00)<<8) | (((value)&0x000000FF)<<24)));
}

EXPORT_API
inline int mmfile_io_be_int32 (unsigned int value)
{
	return (is_little_endian == 0) ? value : ((int)((((value)&0xFF000000)>>24) | (((value)&0x00FF0000)>>8) | (((value)&0x0000FF00)<<8) | (((value)&0x000000FF)<<24)));
}

EXPORT_API
inline int mmfile_io_le_int32 (unsigned int value)
{
	return (is_little_endian == 1) ? value : ((int)((((value)&0xFF000000)>>24) | (((value)&0x00FF0000)>>8) | (((value)&0x0000FF00)<<8) | (((value)&0x000000FF)<<24)));
}

EXPORT_API
inline unsigned short mmfile_io_be_uint16 (unsigned short value)
{
	return (is_little_endian == 0) ? value : ((unsigned short)((((value)&0xFF00)>>8) | (((value)&0x00FF)<<8)));
}

EXPORT_API
inline unsigned short mmfile_io_le_uint16 (unsigned short value)
{
	return (is_little_endian == 1) ? value : ((unsigned short)((((value)&0xFF00)>>8) | (((value)&0x00FF)<<8)));
}

EXPORT_API
inline short mmfile_io_be_int16 (unsigned short value)
{
	return (is_little_endian == 0) ? value : ((short)((((value)&0xFF00)>>8) | (((value)&0x00FF)<<8)));
}

EXPORT_API
inline short mmfile_io_le_int16 (unsigned short value)
{
	return (is_little_endian == 1) ? value : ((short)((((value)&0xFF00)>>8) | (((value)&0x00FF)<<8)));
}




MMFileIOFunc *first_io_func = NULL;

static int _mmfile_open(MMFileIOHandle **handle, struct MMFileIOFunc *Func, const char *filename, int flags)
{
	MMFileIOHandle *pHandle = NULL;
	int err = 0;
	int fileNameLen = 0;

	if (!handle || !Func || !filename || !Func->mmfile_open) {
		debug_error ("invalid param\n");
		err = MMFILE_IO_FAILED;
		goto fail;
	}

	pHandle = mmfile_malloc (sizeof(MMFileIOHandle));
	if (!pHandle) {
		debug_error ("mmfile_malloc: pHandle\n");
		err = MMFILE_IO_FAILED;
		goto fail;
	}

	*handle = pHandle;

	pHandle->iofunc = Func;
	pHandle->flags = flags;
	pHandle->privateData = NULL;
	fileNameLen = strlen(filename);
	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("[%d, %s]\n", fileNameLen, filename);
	#endif
	pHandle->fileName = mmfile_malloc (fileNameLen + 1);
	if (!pHandle->fileName) {
		debug_error ("mmfile_malloc: pHandle->fileName\n");
		err = MMFILE_IO_FAILED;
		goto fail;
	}

	memcpy(pHandle->fileName, filename, fileNameLen);

	err = Func->mmfile_open (pHandle, filename, flags);
	if (err < 0) {
		debug_error ("mmfile_open: pHandle->fileName\n");
		err = MMFILE_IO_FAILED;
		goto fail;
	}
  
	return MMFILE_IO_SUCCESS;

fail:
	if (handle && *handle) // fix for prevent
	{	
		mmfile_close(*handle);
		*handle = NULL;
	}
	
	return err;
}

EXPORT_API
int mmfile_open(MMFileIOHandle **handle, const char *filename, int flags)
{
	MMFileIOFunc   *pFuc = NULL;
	const char  *pFile = NULL;
	char        handle_str[256] = {0,};
	char        *pHandleName = NULL;

	if (!handle || !filename) {
		debug_error ("invalid param\n");
		return MMFILE_IO_FAILED;
	}

	memset (handle_str, 0x00, sizeof(handle_str));

	pFile = filename;
	pHandleName = handle_str;

	/* scan to find ':' */
	while (*pFile != '\0' && *pFile != ':') {
		if (!isalpha(*pFile)) {
			goto file_handle;
		}

		if ((pHandleName - handle_str) < sizeof(handle_str) - 1) {
			*pHandleName++ = *pFile;
		}
		pFile++;
	}

	if (*pFile == '\0') {
file_handle:
		strncpy(handle_str, "file", strlen("file"));
	} else {
		*pHandleName = '\0';
	}

	pFuc = first_io_func;

	while (pFuc != NULL) {
		if (!strcmp(handle_str, pFuc->handleName)) {
			return _mmfile_open (handle, pFuc, filename, flags);
		}
		pFuc = pFuc->next;
	}

	*handle = NULL;

	return MMFILE_IO_FAILED;
}

EXPORT_API
int mmfile_read(MMFileIOHandle *handle, unsigned char *buf, int size)
{
	int ret = 0;
	if (!handle || (handle->flags & MMFILE_WRONLY)) {
		return MMFILE_IO_FAILED;
	}

	if (!handle->iofunc || !handle->iofunc->mmfile_read) {
		return MMFILE_IO_FAILED;
	}

	ret = handle->iofunc->mmfile_read (handle, buf, size);
	return ret;
}

EXPORT_API
int mmfile_write(MMFileIOHandle *handle, unsigned char *buf, int size)
{
	int ret = 0;
	if (!handle || !(handle->flags & (MMFILE_WRONLY | MMFILE_RDWR))) {
		return MMFILE_IO_FAILED;
	}

	if (!handle->iofunc || !handle->iofunc->mmfile_write) {
		return MMFILE_IO_FAILED;
	}

	ret = handle->iofunc->mmfile_write (handle, buf, size);
	return ret;
}

EXPORT_API
long long mmfile_seek(MMFileIOHandle *handle, long long pos, int whence)
{
	long long ret = 0;
	if (!handle || !handle->iofunc || !handle->iofunc->mmfile_seek) {
		return MMFILE_IO_FAILED;
	}

	ret = handle->iofunc->mmfile_seek(handle, pos, whence);
	return ret;
}

EXPORT_API
long long mmfile_tell(MMFileIOHandle *handle)
{
	long long ret = 0;
	if (!handle || !handle->iofunc || !handle->iofunc->mmfile_tell) {
		return MMFILE_IO_FAILED;
	}

	ret = handle->iofunc->mmfile_tell(handle);
	return ret;
}

EXPORT_API
int mmfile_close(MMFileIOHandle *handle)
{
	int ret = 0;

	if (!handle || !handle->iofunc || !handle->iofunc->mmfile_close) {
		return MMFILE_IO_FAILED;
	}

	ret = handle->iofunc->mmfile_close(handle);

	if (handle->fileName) {
		mmfile_free(handle->fileName);
	}

	if (handle) mmfile_free(handle);

	return ret;
}

EXPORT_API
int mmfile_register_io_func (MMFileIOFunc *iofunc)
{
	MMFileIOFunc **ptr = NULL;

	if (!iofunc) {
		return MMFILE_IO_FAILED;
	}

	ptr = &first_io_func;
	while (*ptr != NULL) {
		ptr = &(*ptr)->next;
	}
	*ptr = iofunc;
	iofunc->next = NULL;

	return MMFILE_IO_SUCCESS;
}

EXPORT_API
int mmfile_register_io_all ()
{
	static int initialized = 0;

	if (initialized) {
		return MMFILE_IO_FAILED;
	}

	is_little_endian = _is_little_endian ();
	initialized = 1;

	extern MMFileIOFunc mmfile_file_io_handler;
	extern MMFileIOFunc mmfile_mem_io_handler;
	extern MMFileIOFunc mmfile_mmap_io_handler;

	mmfile_register_io_func (&mmfile_file_io_handler);
	mmfile_register_io_func (&mmfile_mem_io_handler);    
	mmfile_register_io_func (&mmfile_mmap_io_handler);

	return MMFILE_IO_SUCCESS;
}

