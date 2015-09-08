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
* @addtogroup	FILE INFO
*/

/**
* @ingroup	FILEINFO
* @addtogroup	UTS_MMF_FILEINFO Unit
*/

/**
* @ingroup	UTS_MMF_FILEINFO Unit
* @addtogroup	UTS_MMF_FILEINFO_COMMON_ 
* @{
*/

/**
* @file uts_mmf_fileinfo_common.h
* @brief This is a suit of unit test cases to test mm-fileinfo API
* @author Haejeong Kim <backto.kim@samsung.com>
* @version Initial Creation Version 0.1
* @date 2008.09.08
* @last update 2011.01.20
*/
#ifndef __UTS_MMF_FILEINFO_COMMON_H_
#define __UTS_MMF_FILEINFO_COMMON_H_

#include <mm_file.h>
#include <mm_message.h>
#include <mm_error.h>
#include <mm_types.h>
#include <string.h>
#include <tet_api.h>
#include <unistd.h>

#define MEDIA_PATH		"/opt/media/Music/Over the horizon.mp3"
#define MEDIA_PATH2	"/opt/media/Videos/Helicopter.mp4"


void startup();
void cleanup();

void (*tet_startup)() = startup;
void (*tet_cleanup)() = cleanup;

void utc_mm_file_create_tag_attrs_func_01();
void utc_mm_file_create_tag_attrs_func_02();

void utc_mm_file_create_content_attrs_func_01();
void utc_mm_file_create_content_attrs_func_02();
	
void utc_mm_file_destroy_content_attrs_func_01();
void utc_mm_file_destroy_content_attrs_func_02();

void utc_mm_file_destroy_tag_attrs_func_01();
void utc_mm_file_destroy_tag_attrs_func_02();

void utc_mm_file_get_stream_info_func_01();
void utc_mm_file_get_stream_info_func_02();
void utc_mm_file_get_stream_info_func_03();
void utc_mm_file_get_stream_info_func_04();

void utc_mm_file_create_content_attrs_simple_func_01();
void utc_mm_file_create_content_attrs_simple_func_02();
void utc_mm_file_create_content_attrs_simple_func_03();

void utc_mm_file_create_content_attrs_from_memory_func_01();
void utc_mm_file_create_content_attrs_from_memory_func_02();
void utc_mm_file_create_content_attrs_from_memory_func_03();

void utc_mm_file_create_tag_attrs_from_memory_func_01();
void utc_mm_file_create_tag_attrs_from_memory_func_02();
void utc_mm_file_create_tag_attrs_from_memory_func_03();

void utc_mm_file_get_attrs_func_01();
void utc_mm_file_get_attrs_func_02();
void utc_mm_file_get_attrs_func_03();
void utc_mm_file_get_attrs_func_04();

#endif //__UTS_MMF_FILEINFO_COMMON_H_


