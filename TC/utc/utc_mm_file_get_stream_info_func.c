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
* @addtogroup	UTS_MMF_FILEINFO_GET_STREAM_INFO_ Uts_Mmf_Fileinfo_Get_Stream_Info_
* @{
*/

/**
* @file uts_mm_fileinfo_get_stream_info.c
* @brief This is a suit of unit test cases to test mm_file_get_stream_info() API function
* @author Haejeong Kim <backto.kim@samsung.com>
* @version Initial Creation Version 0.1
* @date 2008.09.08
* @last update 2011.01.20
*/

#include "utc_mm_fileinfo_common.h"




///////////////////////////////////////////////////////////////////////////////////////////////////
// Definitions of "uts_mmf_fileinfo_get_stream_info"
//-------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////
// Declare the global variables and registers and Internal Funntions
//-------------------------------------------------------------------------------------------------

#define TEST_API "mm_file_get_stream_info"

///////////////////////////////////////////////////////////////////////////////////////////////////
/* Initialize TCM data structures */


struct tet_testlist tet_testlist[] = {
	{utc_mm_file_get_stream_info_func_01, 1},
	{utc_mm_file_get_stream_info_func_02, 2},
	{utc_mm_file_get_stream_info_func_03, 3},
	{utc_mm_file_get_stream_info_func_04, 4},
	{NULL, 0}
};

/* Start up function for each test purpose */
void startup ()
{
}

/* Clean up function for each test purpose */
void cleanup ()
{
}

void utc_mm_file_get_stream_info_func_01()
{
	int err = 0 ;
	int audio_stream_num = 0;
	int video_stream_num = 0;

	err = mm_file_get_stream_info(MEDIA_PATH2,&audio_stream_num,&video_stream_num);
	dts_check_eq (TEST_API, err, MM_ERROR_NONE, "%x", err);
	dts_check_eq (TEST_API, audio_stream_num, 1);
	dts_check_eq (TEST_API, video_stream_num, 1);

	return;
}

void utc_mm_file_get_stream_info_func_02()
{
	int err = 0 ;
	int audio_stream_num = 0;
	int video_stream_num = 0;

	err = mm_file_get_stream_info(MEDIA_PATH,&audio_stream_num,&video_stream_num);
	dts_check_eq (TEST_API, err, MM_ERROR_NONE, "%x", err);
	dts_check_eq (TEST_API, audio_stream_num, 1);
	dts_check_eq (TEST_API, video_stream_num, 0);
}

void utc_mm_file_get_stream_info_func_03()
{
	int err = 0 ;
	int video_stream_num =0;

	err = mm_file_get_stream_info(MEDIA_PATH2, NULL, &video_stream_num);
	dts_check_ne (TEST_API, err, MM_ERROR_NONE, "%x", err);
}

void utc_mm_file_get_stream_info_func_04()
{
	int err = 0 ;

	err = mm_file_get_stream_info(MEDIA_PATH2,NULL,NULL);
	dts_check_ne (TEST_API, err, MM_ERROR_NONE, "%x", err);
}

void utc_mm_file_get_stream_info_func_05()
{
	int err = 0 ;

	err = mm_file_get_stream_info(NULL,NULL,NULL);
	dts_check_ne (TEST_API, err, MM_ERROR_NONE, "%x", err);
}




/** @} */




