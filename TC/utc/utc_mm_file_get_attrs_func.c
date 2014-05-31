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

/**
* @ingroup	MMF_FILEINFO_API
* @addtogroup	FILEINFO
*/

/**
* @ingroup	FILEINFO
* @addtogroup	UTS_MMF_FILEINFO Unit
*/

/**
* @ingroup	UTS_MMF_FILEINFO Unit
* @addtogroup	UTS_MMF_FILEINFO_FREE_ATTRS_ Uts_Mmf_Fileinfo_Free_Tag_Attrs_
* @{
*/

/**
* @file uts_mm_fileinfo_free_tag_attrs.c
* @brief This is a suit of unit test cases to test mm_file_free_tag_attr() API function
* @author Haejeong Kim <backto.kim@samsung.com>
* @version Initial Creation Version 0.1
* @date 2008.09.08
* @last update 2011.01.20
*/


#include "utc_mm_fileinfo_common.h"




///////////////////////////////////////////////////////////////////////////////////////////////////
// Definitions of "uts_mmf_fileinfo_free_tag_attrs"
//-------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////
// Declare the global variables and registers and Internal Funntions
//-------------------------------------------------------------------------------------------------
#define TEST_API "mm_file_get_attrs"

///////////////////////////////////////////////////////////////////////////////////////////////////
/* Initialize TCM data structures */


struct tet_testlist tet_testlist[] = {
	{utc_mm_file_get_attrs_func_01, 1},
	{utc_mm_file_get_attrs_func_02, 2},
	{utc_mm_file_get_attrs_func_03, 3},
	{utc_mm_file_get_attrs_func_04, 4},
	{NULL, 0}
};

typedef struct _ContentContext {
	int duration;
	int video_codec;
	int video_bitrate;
	int video_fps;
	int video_w;
	int video_h;
	int video_track_id;
	int video_track_num;
	int audio_codec;
	int audio_bitrate;
	int audio_channel;
	int audio_samplerate;
	int audio_track_id;
	int audio_track_num;
}ContentContext_t;

typedef struct _TagContext {
	char artist[100];
	int artist_len;
	char title[100];
	int title_len;
	char album[100];
	int album_len;
	char album_artist[100];
	int album_artist_len;
	char genre[100];
	int genre_len;
	char author[100];
	int author_len;
	char date[100]; 			//int
	int date_len;
}TagContext_t;

/* Start up function for each test purpose */
void startup ()
{
}

/* Clean up function for each test purpose */
void cleanup ()
{
}

void utc_mm_file_get_attrs_func_01()
{
	int err = 0 ;
	int ret1 = 0;
	MMHandleType attr;
	ContentContext_t ccontent;
	char *err_attr_name = NULL;

	ret1 = mm_file_create_content_attrs(&attr, MEDIA_PATH);
	dts_check_eq ("mm_file_create_content_attrs", ret1, MM_ERROR_NONE, "%x", ret1);

	memset (&ccontent, 0, sizeof (ContentContext_t));
	err = mm_file_get_attrs(attr, &err_attr_name,
										MM_FILE_CONTENT_AUDIO_CODEC, &ccontent.audio_codec,
										MM_FILE_CONTENT_AUDIO_SAMPLERATE, &ccontent.audio_samplerate,
										MM_FILE_CONTENT_AUDIO_BITRATE, &ccontent.audio_bitrate,
										MM_FILE_CONTENT_AUDIO_CHANNELS, &ccontent.audio_channel,
										MM_FILE_CONTENT_AUDIO_TRACK_INDEX, &ccontent.audio_track_id,
										MM_FILE_CONTENT_AUDIO_TRACK_COUNT, &ccontent.audio_track_num,
										NULL);

	dts_check_eq (TEST_API, err, MM_ERROR_NONE, "%x", err);
	dts_check_eq (TEST_API, ccontent.audio_codec, 2);
	dts_check_eq (TEST_API, ccontent.audio_samplerate, 44100);
	dts_check_eq (TEST_API, ccontent.audio_bitrate, 128000);
	dts_check_eq (TEST_API, ccontent.audio_channel, 2);
	dts_check_eq (TEST_API, ccontent.audio_track_id, 0);
	dts_check_eq (TEST_API, ccontent.audio_track_num, 1);

	mm_file_destroy_content_attrs(attr);

	if (err_attr_name)
		free (err_attr_name);

	return;
}

void utc_mm_file_get_attrs_func_02()
{
	int err = 0 ;
	int ret1 = 0;
	MMHandleType attr;
	ContentContext_t ccontent;

	char *err_attr_name = NULL;

	ret1 = mm_file_create_content_attrs(&attr, MEDIA_PATH);
	dts_check_eq ("mm_file_create_conten_attrs", ret1, MM_ERROR_NONE, "%x", ret1);

	memset (&ccontent, 0, sizeof (ContentContext_t));
	err = mm_file_get_attrs(attr, &err_attr_name, "content-audio-codecs"	, &ccontent.audio_codec, NULL);

	dts_check_ne (TEST_API, err, MM_ERROR_NONE, "%x, %s", err, err_attr_name);

	mm_file_destroy_content_attrs(attr);

	if (err_attr_name)
		free (err_attr_name);

	return;
}

void utc_mm_file_get_attrs_func_03()
{
	int err = 0 ;
	int ret1 = 0;
	MMHandleType attr;
	TagContext_t ctag;
	char *err_attr_name = NULL;

	ret1 = mm_file_create_tag_attrs(&attr, MEDIA_PATH);
	dts_check_eq ("mm_file_create_tag_attrs", ret1, MM_ERROR_NONE, "%x", ret1);

	memset (&ctag, 0, sizeof (TagContext_t));
	err =  mm_file_get_attrs( attr,
			&err_attr_name,
			MM_FILE_TAG_ARTIST, &ctag.artist, &ctag.artist_len,
			MM_FILE_TAG_ALBUM, &ctag.album, &ctag.album_len,
			MM_FILE_TAG_ALBUM_ARTIST, &ctag.album_artist, &ctag.album_artist_len,
			MM_FILE_TAG_TITLE, &ctag.title, &ctag.title_len,
			MM_FILE_TAG_GENRE, &ctag.genre, &ctag.genre_len,
			MM_FILE_TAG_AUTHOR, &ctag.author, &ctag.author_len,
			MM_FILE_TAG_DATE, &ctag.date, &ctag.date_len,
			NULL);

	dts_check_eq (TEST_API, err, MM_ERROR_NONE, "%x", err);
	dts_check_str_eq (TEST_API, ctag.artist, "Samsung");
	dts_check_str_eq (TEST_API, ctag.album, "Samsung");
	dts_check_str_eq (TEST_API, ctag.title, "Over the horizon");
	dts_check_str_eq (TEST_API, ctag.genre, "Rock");
	dts_check_str_eq (TEST_API, ctag.date, "2011");

	mm_file_destroy_tag_attrs(attr);

	if (err_attr_name)
		free (err_attr_name);

	return;
}

void utc_mm_file_get_attrs_func_04()
{
	int err = 0 ;
	int ret1 = 0;
	MMHandleType attr;
	TagContext_t ctag;
	char *err_attr_name = NULL;

	ret1 = mm_file_create_tag_attrs(&attr, MEDIA_PATH);
	dts_check_eq ("mm_file_create_tag_attrs", ret1, MM_ERROR_NONE, "%x", ret1);

	memset (&ctag, 0, sizeof (TagContext_t));
	err =  mm_file_get_attrs( attr, &err_attr_name, "tag-artists"	, &ctag.artist, &ctag.artist_len, NULL);
	dts_check_ne (TEST_API, err, MM_ERROR_NONE, "%x, %s", err, err_attr_name);

	mm_file_destroy_tag_attrs(attr);

	if (err_attr_name)
		free (err_attr_name);

	return;
}






/** @} */




