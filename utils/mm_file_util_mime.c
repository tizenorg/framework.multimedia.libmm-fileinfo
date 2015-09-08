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

#include <stdlib.h>
#include <string.h>
#include "mm_debug.h"
#include "mm_file_utils.h"


typedef struct _mmfileavmimetype
{
    char    mimetype[MMFILE_MIMETYPE_MAX_LEN];    
    char    ffmpegFormat[MMFILE_FILE_FMT_MAX_LEN];
    char    extension[MMFILE_FILE_EXT_MAX_LEN];
} MMFileAVMimeType;

#define __FFMPEG_MIME_TABLE_SIZE 75
const MMFileAVMimeType MMFILE_FFMPEG_MIME_TABLE [] =
{
    {"audio/mpeg",          "mp3",      "mp3"},
    {"audio/mp3",           "mp3",      "mp3"},
    {"audio/mpg3",          "mp3",      "mp3"},
    {"audio/mpeg3",         "mp3",      "mp3"},
    {"audio/mpg",           "mp3",      "mp3"},
    {"audio/x-mpeg",        "mp3",      "mp3"},
    {"audio/x-mp3",         "mp3",      "mp3"},
    {"audio/x-mpeg3",       "mp3",      "mp3"},
    {"audio/x-mpg",         "mp3",      "mp3"},
    {"audio/x-mpegaudio",   "mp3",      "mp3"},  //10

    {"video/3gpp",          "mov,mp4,m4a,3gp,3g2,mj2",      "3gp"},
    {"video/h263",          "mov,mp4,m4a,3gp,3g2,mj2",      "3gp"},
    {"video/3gp",           "mov,mp4,m4a,3gp,3g2,mj2",      "3gp"},
    {"video/3gpp",          "mov,mp4,m4a,3gp,3g2,mj2",      "3gp"},
    {"video/mp4v-es",       "mov,mp4,m4a,3gp,3g2,mj2",      "mp4"},
    {"video/mpeg",          "mov,mp4,m4a,3gp,3g2,mj2",      "mpeg"},
    {"audio/3gpp",          "mov,mp4,m4a,3gp,3g2,mj2",      "3gp"},  //17

    {"video/mpeg4",         "mov,mp4,m4a,3gp,3g2,mj2",      "mp4"},
    {"video/mp4",           "mov,mp4,m4a,3gp,3g2,mj2",      "mp4"},
    {"video/x-mp4",         "mov,mp4,m4a,3gp,3g2,mj2",      "mp4"},
    {"video/x-pv-mp4",      "mov,mp4,m4a,3gp,3g2,mj2",      "mp4"},
    {"audio/mp4",           "mov,mp4,m4a,3gp,3g2,mj2",      "mp4"},
    {"audio/MP4A-LATM",     "mov,mp4,m4a,3gp,3g2,mj2",      "mp4"},
    {"audio/mpeg4",         "mov,mp4,m4a,3gp,3g2,mj2",      "mp4"},
    {"audio/m4a",           "mov,mp4,m4a,3gp,3g2,mj2",      "mp4"}, //25

    {"video/avi",           "avi",      "avi"},
    {"video/divx",          "divx",     "divx"},

    {"audio/x-ms-asf",      "asf",      "asf"},
    {"video/x-ms-asf",      "asf",      "asf"},
    {"video/x-ms-asf",      "asf",      "asf"},

    {"video/x-ms-wmv",      "asf",      "wmv"},
    {"audio/wma",           "asf",      "wma"},
    {"audio/x-ms-wma",      "asf",      "wma"},

    {"audio/wave",          "wav",      "wav"},     /* Not Sure */
    {"audio/wav",           "wav",      "wav"},     /* Not Sure */
    {"audio/x-wave",        "wav",      "wav"},     /* Not Sure */
    {"audio/x-wav",         "wav",      "wav"},     /* Not Sure */

    {"audio/aac",           "aac",      "aac"},     /* Not Sure */
    {"audio/g72",           "aac",      "aac"},     /* Not Sure */ //39

    {"audio/AMR",           "amr",      "amr"},     /* Not Sure */
    {"audio/amr-wb",        "amr",      "amr"},     /* Not Sure */
    {"audio/x-amr",         "amr",      "amr"},     /* Not Sure */

    {"audio/x-mid",         "mid",      "mid"},     /* Not Sure */
    {"audio/x-midi",        "mid",      "mid"},     /* Not Sure */
    {"audio/mid",           "mid",      "mid"},     /* Not Sure */
    {"audio/midi",          "mid",      "mid"},     /* Not Sure */
    {"audio/mid",           "mid",      "mid"},     /* Not Sure */  //47

    {"audio/iMelody",       "imy",      "imy"},     /* Not Sure */
    {"audio/imelody",       "imy",      "imy"},     /* Not Sure */
    {"audio/melody",        "imy",      "imy"},     /* Not Sure */
    {"audio/imy",           "imy",      "imy"},     /* Not Sure */
    {"audio/x-iMelody",     "imy",      "imy"},     /* Not Sure */

    {"audio/basic",         "snd",      ""},     /* Not Sure */
    {"audio/pmd",           "pmd",      ""},     /* Not Sure */
    {"audio/sp-midi",       "smp",      ""},     /* Not Sure */ //55

    {"audio/mmf",           "mmf",      "mmf"},     /* Not Sure */
    {"audio/smaf",          "mmf",      "mmf"},     /* Not Sure */
    {"audio/x-mmf",         "mmf",      "mmf"},     /* Not Sure */
    {"audio/x-smaf",        "mmf",      "mmf"},     /* Not Sure */

    {"audio/xmf",           "xmf",      "xmf"},     /* Not Sure */
    {"audio/mobile-xmf",    "xmf",      "xmf"},     /* Not Sure */
    {"audio/x-xmf",         "xmf",      "xmf"},     /* Not Sure */ //62

    {"audio/vnd.rn-realaudio",              "rm",   ""},  /* Not Sure */
    {"audio/x-pn-multirate-realaudio",      "rm",   ""},  /* Not Sure */
    {"audio/x-pn-multirate-realaudio-live", "rm",   ""},  /* Not Sure */
    {"video/vnd.rn-realvideo",              "rm",   ""},  /* Not Sure */
    {"video/vnd.rn-realmedia",              "rm",   ""},  /* Not Sure */
    {"video/x-pn-multirate-realvideo",      "rm",   ""},  /* Not Sure */ //68

    {"video/ogg",           "ogg",      "ogg"},
    {"video/theora",        "ogg",      "ogg"},
    {"audio/ogg",           "ogg",      "ogg"},
    {"audio/x-ogg",         "ogg",      "ogg"},
    {"audio/vorbis",        "ogg",      "ogg"},  //73

	{"audio/x-flac",        "flac",      "flac"},  //74
	{"video/x-flv",        "flv",      "flv"},  //75
};


EXPORT_API
int mmfile_util_get_ffmpeg_format (const char *mime, char *ffmpegFormat)
{
    int i = 0;

    if ( NULL == mime || NULL == ffmpegFormat)
    {
        debug_error ("error: mmfile_util_get_format\n");
        return MMFILE_UTIL_FAIL;
    }

    for (i = 0; i < __FFMPEG_MIME_TABLE_SIZE; i++)
    {
        if (!strcasecmp (MMFILE_FFMPEG_MIME_TABLE[i].mimetype, mime))
        {
            break;
        }
    }

    if (i == __FFMPEG_MIME_TABLE_SIZE)
    {
        debug_error ("error: not found[%s]\n", mime);
        return MMFILE_UTIL_FAIL;    
    }

    memcpy (ffmpegFormat, MMFILE_FFMPEG_MIME_TABLE[i].ffmpegFormat, strlen(MMFILE_FFMPEG_MIME_TABLE[i].ffmpegFormat));

    return MMFILE_UTIL_SUCCESS;
}


EXPORT_API
int mmfile_util_get_file_ext (const char *mime, char *ext)
{
    int i = 0;

    if ( NULL == mime || NULL == ext)
    {
        debug_error ("error: mmfile_util_get_file_ext\n");
        return MMFILE_UTIL_FAIL;
    }

    for (i = 0; i < __FFMPEG_MIME_TABLE_SIZE; i++)
    {
        if (!strcasecmp (MMFILE_FFMPEG_MIME_TABLE[i].mimetype, mime))
        {
            break;
        }
    }

    if (i == __FFMPEG_MIME_TABLE_SIZE)
    {
        debug_error ("error: not found[%s]\n", mime);
        return MMFILE_UTIL_FAIL;    
    }

    memcpy (ext, MMFILE_FFMPEG_MIME_TABLE[i].extension, strlen(MMFILE_FFMPEG_MIME_TABLE[i].extension));

    return MMFILE_UTIL_SUCCESS;
}


