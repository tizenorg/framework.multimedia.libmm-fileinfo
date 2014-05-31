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

#include <string.h>
#include <stdlib.h>
#include "mm_debug.h"
#include "mm_file_utils.h"
#include "mm_file_format_tag_id3.h"
#include "mm_file_format_tags.h"

#define MMFILE_TAGS_NUMBER_OF_ELEMENTS(X) (sizeof X / sizeof X[0])

typedef int (*MMFileFindTagFunc)(MMFileIOHandle *fp, unsigned char bAppended, unsigned int startOffset, unsigned int endOffset, tMMFileTags *out);


typedef struct mmfiletagsdata
{
    MMFileIOHandle *fp;
    unsigned int filesize;
    unsigned int startOffset;
    unsigned int endOffset;
    MMFileList   tagList;
} tMMFileTagsData;


MMFileFindTagFunc gFindPreprendTagFuncs[] =
{
    MMFileID3V2TagFind,
};

MMFileFindTagFunc gFindAppendTagFuncs[] =
{
    MMFileID3V1TagFind,
    MMFileID3V2TagFind,    
};


int _MMFileFindTags        (tMMFileTagsData *privateData, MMFileFindTagFunc FindTag, unsigned char bAppended);
int _MMFileFindPrependTags (tMMFileTagsData *privateData);
int _MMFileFindAppendTags  (tMMFileTagsData *privateData);



EXPORT_API
int MMFileOpenTags (MMFileTagsHandle *tagsHandle, const char *uriName)
{
    tMMFileTagsData *privateData = NULL;
    int ret = 0;

    privateData = mmfile_malloc (sizeof(tMMFileTagsData));
    if (!privateData)
    {
        debug_error ("mmfile_malloc: tMMFileTagsData\n");
        return MMFILE_TAGS_FAIL;
    }

    *tagsHandle = privateData;

    ret = mmfile_open (&privateData->fp, uriName, MMFILE_RDONLY);
    if (MMFILE_UTIL_FAIL == ret)
    {
        debug_error ("mmfile_open\n");
        ret = MMFILE_TAGS_FAIL;
        goto exception;
    }

    ret = mmfile_seek (privateData->fp, 0, MMFILE_SEEK_END);
    if (MMFILE_UTIL_FAIL == ret)
    {
        debug_error ("mmfile_seek\n");
        ret = MMFILE_TAGS_FAIL;
        goto exception;
    }

    privateData->filesize = mmfile_tell (privateData->fp);

    mmfile_seek (privateData->fp, 0, MMFILE_SEEK_SET);
    
    privateData->startOffset = 0;
    privateData->endOffset = privateData->filesize;

    /* Find all of tags */
    while (MMFILE_TAGS_SUCCESS == _MMFileFindPrependTags (privateData)) ;
    while (MMFILE_TAGS_SUCCESS == _MMFileFindAppendTags  (privateData)) ;

    if (!privateData->tagList)
    {
        debug_error ("there is no tags\n");
        ret = MMFILE_TAGS_FAIL;
        goto exception;
    }

    return MMFILE_TAGS_SUCCESS;

exception:
    if (*tagsHandle)
    {
        MMFileTagsClose (*tagsHandle);
        *tagsHandle = NULL;
    }
    
    return ret;
}

EXPORT_API
int MMFileTagsClose (MMFileTagsHandle tagsHandle)
{
    tMMFileTagsData *privateData = tagsHandle;

    if (privateData)
    {
        if (privateData->fp)
        {
            mmfile_close (privateData->fp);
        }

        if (privateData->tagList)
        {
            mmfile_free (privateData->tagList);
        }

        mmfile_free (privateData);
    }

    return MMFILE_TAGS_SUCCESS;
}



EXPORT_API
int MMFileGetFirstTag (MMFileTagsHandle tagsHandle, tMMFileTags *out)
{
    
}

EXPORT_API
int MMFileGetNextTag (MMFileTagsHandle tagsHandle, tMMFileTags *out)
{
}

int _MMFileFindPrependTags (tMMFileTagsData *privateData)
{
    int i = 0;
    int res = 0;

    if (!privateData || !privateData->fp)
    {
        debug_error ("Invalid argument\n");
        return MMFILE_TAGS_FAIL;
    }

    for (i = 0; i < MMFILE_TAGS_NUMBER_OF_ELEMENTS(gFindPreprendTagFuncs); i++)
    {
        if (gFindPreprendTagFuncs[i])
        {
            if (MMFILE_TAGS_SUCCESS == _MMFileFindTags (privateData, gFindPreprendTagFuncs[i], 0))
            {
                return MMFILE_TAGS_SUCCESS;
            }
        }
    }

    return MMFILE_TAGS_FAIL;
}

int _MMFileFindAppendTags (tMMFileTagsData *privateData)
{
    int i = 0;
    int res = 0;

    if (!privateData || !privateData->fp)
    {
        debug_error ("Invalid argument\n");
        return MMFILE_TAGS_FAIL;
    }

    for (i = 0; i < MMFILE_TAGS_NUMBER_OF_ELEMENTS(gFindAppendTagFuncs); i++)
    {
        if (gFindAppendTagFuncs[i])
        {
            if (MMFILE_TAGS_SUCCESS == _MMFileFindTags (privateData, gFindAppendTagFuncs[i], 1))
            {
                return MMFILE_TAGS_SUCCESS;
            }
        }
    }

    return MMFILE_TAGS_FAIL;
}

int _MMFileFindTags (tMMFileTagsData *privateData, MMFileFindTagFunc FindTag, unsigned char bAppended)
{
    tMMFileTags *tagData = NULL;
    int ret = 0;

    if (!privateData || !privateData->fp)
    {
        debug_error ("Invalid argument\n");
        return MMFILE_TAGS_FAIL;
    }

    tagData = mmfile_malloc (sizeof(tMMFileTags));
    if (!tagData)
    {
        debug_error ("mmfile_malloc tagData\n");
        return MMFILE_TAGS_FAIL;
    }

    ret = FindTag (privateData->fp, bAppended, privateData->startOffset, privateData->endOffset, tagData);
    if (ret)
    {
        if (bAppended)
        {
            privateData->endOffset = tagData->startOffset;
        }
        else
        {
            privateData->startOffset = tagData->endOffset;
        }

        /* add tagData into privateData->tagList */
        
        //privateData->tagList = mmfile_list_append (privateData->tagList, tagData);

        ret = MMFILE_TAGS_SUCCESS;
    }

    mmfile_free(tagData);
    
    return ret;
}

