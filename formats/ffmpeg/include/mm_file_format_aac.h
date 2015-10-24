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

#ifndef __MM_FILE_FORMAT_AAC_H__
#define __MM_FILE_FORMAT_AAC_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "mm_file_formats.h"

#define MMFILE_AAC_PARSER_SUCCESS   1
#define MMFILE_AAC_PARSER_FAIL      0

typedef enum _mmfile_aac_profile_type {
	AAC_PROFILE_MAIN,
	AAC_PROFILE_LC,
	AAC_PROFILE_SSR,
	AAC_PROFILE_LTP,
	AAC_PROFILE_UNKNOWN
}TAacProfileType;

typedef void* MMFileAACHandle;

typedef struct _mmfileaacstreaminfo {
	unsigned int    iseekable;
	long long       duration;
	long long       fileSize;
	unsigned int    bitRate;
	unsigned int    samplingRate;
	unsigned int    frameRate;
	unsigned int    numAudioChannels;
	unsigned int    numTracks;
	TAacProfileType profileType;
} tMMFILE_AAC_STREAM_INFO;


typedef struct _mmfileaactaginfo {
	char *title;
	char *author;
	char *artist;
	char *album;
	char *album_artist;
	char *year;
	char *copyright;
	char *comment;
	char *genre;
	char *tracknum;
	char *composer;
	char *classification;
	char *rating;
	char *recordDate;
	char *conductor;
	char *artworkMime;
	char *artwork;
	unsigned int artworkSize;
} tMMFILE_AAC_TAG_INFO;


int mmfile_aacparser_open(MMFileAACHandle *handle, const char *src);
int mmfile_aacparser_get_stream_info(MMFileAACHandle handle, tMMFILE_AAC_STREAM_INFO *aacinfo);
int mmfile_aacparser_get_tag_info(MMFileAACHandle handle, tMMFILE_AAC_TAG_INFO *info);
int mmfile_aacparser_get_next_frame(MMFileAACHandle handle, tMMFILE_AAC_STREAM_INFO *aacinfo);
int mmfile_aacparser_close(MMFileAACHandle handle);

#ifdef __cplusplus
}
#endif

#endif /*__MM_FILE_FORMAT_AAC_H__*/

