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

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mm_debug.h"
#include "mm_file_utils.h"


typedef struct {
	int fd;
	unsigned char *ptr;
	long long size;
	long long offset;
	int	state;
} MMFMMapIOHandle;

static int mmf_mmap_open (MMFileIOHandle *handle, const char *filename, int flags)
{
    MMFMMapIOHandle *mmapHandle = NULL;
    struct stat finfo = {0, };
    int access = 0;
    
    if (!handle || !filename || !handle->iofunc || !handle->iofunc->handleName)
    {
        debug_error ("invalid param\n");
        return MMFILE_IO_FAILED;        
    }

    filename += strlen(handle->iofunc->handleName) + 3; /* :// */

    memset (&finfo, 0x00, sizeof (struct stat));

    mmapHandle = mmfile_malloc (sizeof(MMFMMapIOHandle));
    if (!mmapHandle)
    {
        debug_error ("error: mmfile_malloc mmapHandle\n");
        return MMFILE_IO_FAILED;        
    }

    if (flags & MMFILE_RDWR)
    {
        access = O_CREAT | O_TRUNC | O_RDWR;
    } 
    else if (flags & MMFILE_WRONLY) 
    {
        access = O_CREAT | O_TRUNC | O_WRONLY;
    } 
    else 
    {
        access = O_RDONLY;
    }
    
#ifdef O_BINARY
    access |= O_BINARY;
#endif

    mmapHandle->fd = open (filename, access, 0666);
    if (mmapHandle->fd < 0)
    {
        debug_error ("error: open error: %s\n", filename);
        goto exception;
    }

    if (fstat (mmapHandle->fd, &finfo) == -1) 
    {
        debug_error ("error: fstat\n");
        goto exception;
    }

    if (!S_ISREG(finfo.st_mode))
    {
        debug_error ("error: it is not regular file\n");
        goto exception;
    }

    mmapHandle->size = finfo.st_size;
    mmapHandle->offset = 0;
    mmapHandle->state = 0;

    if (flags & MMFILE_RDWR)
    {
        //mmapHandle->ptr = mmap64 (0, mmapHandle->size, PROT_WRITE | PROT_READ, MAP_SHARED, mmapHandle->fd, 0);
        mmapHandle->ptr = mmap (0, mmapHandle->size, PROT_WRITE | PROT_READ, MAP_SHARED, mmapHandle->fd, 0);
    } 
    else if (flags & MMFILE_WRONLY) 
    {
        //mmapHandle->ptr = mmap64 (0, mmapHandle->size, PROT_WRITE, MAP_SHARED, mmapHandle->fd, 0);
        mmapHandle->ptr = mmap (0, mmapHandle->size, PROT_WRITE, MAP_SHARED, mmapHandle->fd, 0);
    } 
    else 
    {
        //mmapHandle->ptr = mmap64 (0, mmapHandle->size, PROT_READ, MAP_SHARED, mmapHandle->fd, 0);
        mmapHandle->ptr = mmap (0, mmapHandle->size, PROT_READ, MAP_SHARED, mmapHandle->fd, 0);
    }
    
    if (mmapHandle->ptr == (void*)-1)
    {
        debug_error ("error: mmap\n");
        mmapHandle->ptr = NULL;
        goto exception;
    }

    handle->privateData = (void*) mmapHandle;

    return MMFILE_IO_SUCCESS;

exception:
    if (mmapHandle)
    {
        if (mmapHandle->ptr)
        {
            munmap (mmapHandle->ptr, mmapHandle->size);
        }

        if (mmapHandle->fd > 2)
        {
            close (mmapHandle->fd);
        }

        mmfile_free (mmapHandle);
        handle->privateData = NULL;
    }
    
    return MMFILE_IO_FAILED;
}

static int mmf_mmap_read (MMFileIOHandle *h, unsigned char *buf, int size)
{
    MMFMMapIOHandle *mmapHandle = NULL;
    const unsigned char *c = NULL;
    int len = 0;

    if (!h || !h->privateData || !buf)
    {
        debug_error ("invalid para\n");
        return MMFILE_IO_FAILED;
    }

    mmapHandle = h->privateData;

    c = mmapHandle->ptr + mmapHandle->offset;

    if (mmapHandle->state != EOF)
    {
        len = size;
        if (len + mmapHandle->offset > mmapHandle->size) 
        {
            len = mmapHandle->size - mmapHandle->offset;
        }
    }
    else
    {
        return 0;
    }

    memcpy (buf, c, len);
    
    mmapHandle->offset += len;

    if ( mmapHandle->offset == mmapHandle->size) 
    {
        mmapHandle->state = EOF;
    }

    return len;
}

static int mmf_mmap_write (MMFileIOHandle *h, unsigned char *buf, int size)
{
    MMFMMapIOHandle *mmapHandle = NULL;
    unsigned char *c = NULL;
    int len = 0;

    if (!h || !h->privateData || !buf)
    {
        debug_error ("invalid para\n");
        return MMFILE_IO_FAILED;
    }

    mmapHandle = h->privateData;
    
    c = mmapHandle->ptr + mmapHandle->offset;

    if (mmapHandle->state != EOF)
    {
        len = size;
        if (len + mmapHandle->offset > mmapHandle->size)
        {
            len = mmapHandle->size - mmapHandle->offset;
        }
    }
    else
    {
        return 0;
    }

    memcpy (c, buf, len);

    mmapHandle->offset += len;

    if ( mmapHandle->offset == mmapHandle->size) 
    {
        mmapHandle->state = EOF;
    }

    return len;
}


static long long mmf_mmap_seek (MMFileIOHandle *h, long long pos, int whence)
{
    MMFMMapIOHandle *mmapHandle = NULL;
    long tmp_offset = 0;

    if (!h || !h->privateData)
    {
        debug_error ("invalid para\n");
        return MMFILE_IO_FAILED;
    }

    mmapHandle = h->privateData;

    switch (whence) 
    {
        case SEEK_SET:
            tmp_offset = 0 + pos;
            break;
        case SEEK_CUR:
            tmp_offset = mmapHandle->offset + pos;
            break;
        case SEEK_END:
            tmp_offset = mmapHandle->size + pos;
            break;
        default:
            return MMFILE_IO_FAILED;
    }

    /*check validation*/
    if (tmp_offset < 0)
    {
        debug_error ("invalid file offset\n");
        return MMFILE_IO_FAILED;
    }

    /*set */
    mmapHandle->state = (tmp_offset >= mmapHandle->size) ? EOF : !EOF;
    mmapHandle->offset = tmp_offset;

    return mmapHandle->offset;
}


static long long mmf_mmap_tell (MMFileIOHandle *h)
{
    MMFMMapIOHandle *mmapHandle = NULL;

    if (!h || !h->privateData)
    {
        debug_error ("invalid para\n");
        return MMFILE_IO_FAILED;
    }

    mmapHandle = h->privateData;

    return mmapHandle->offset;
}

static int mmf_mmap_close (MMFileIOHandle *h)
{
    MMFMMapIOHandle *mmapHandle = NULL;

    if (!h || !h->privateData)
    {
        debug_error ("invalid para\n");
        return MMFILE_IO_FAILED;
    }

    mmapHandle = h->privateData;

    if (mmapHandle)
    {
        if (mmapHandle->ptr)
        {
            munmap (mmapHandle->ptr, mmapHandle->size);
        }

        if (mmapHandle->fd > 2)
        {
    close (mmapHandle->fd);
        }

    mmfile_free (mmapHandle);
    }
    
    h->privateData = NULL;

    return MMFILE_IO_SUCCESS;
}


MMFileIOFunc mmfile_mmap_io_handler = {
	"mmap",
	mmf_mmap_open,
	mmf_mmap_read,
	mmf_mmap_write,
	mmf_mmap_seek,
	mmf_mmap_tell,
	mmf_mmap_close
};
