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
* @addtogroup	UTS_MMF_FILEINFO_GET_ATTRS_ Uts_Mmf_Fileinfo_Get_Content_Attrs_
* @{
*/

/**
* @file uts_mm_fileinfo_get_content_attrs.c
* @brief This is a suit of unit test cases to test mm_file_get_content_attr() API function
* @author Haejeong Kim <backto.kim@samsung.com>
* @version Initial Creation Version 0.1
* @date 2008.09.08
* @last update 2011.01.20
*/

#include "utc_mm_fileinfo_common.h"




///////////////////////////////////////////////////////////////////////////////////////////////////
// Definitions of "uts_mmf_fileinfo_get_tag_attrs"
//-------------------------------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////////////////////////
// Declare the global variables and registers and Internal Funntions
//-------------------------------------------------------------------------------------------------
#define TEST_API "mm_file_create_content_attrs"

///////////////////////////////////////////////////////////////////////////////////////////////////
/* Initialize TCM data structures */


struct tet_testlist tet_testlist[] = {
	{utc_mm_file_create_content_attrs_func_01, 1},
	{utc_mm_file_create_content_attrs_func_02, 2},
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

void utc_mm_file_create_content_attrs_func_01()
{
	int err = 0;
	MMHandleType content_attrs;

	/* Get the content & tag attributes */
	err = mm_file_create_content_attrs(&content_attrs, MEDIA_PATH);
	dts_check_eq (TEST_API, err, MM_ERROR_NONE, "%x", err);

	return;
}

void utc_mm_file_create_content_attrs_func_02()
{
	int err = 0;

	err = mm_file_create_content_attrs( NULL, MEDIA_PATH);
	dts_check_ne (TEST_API, err, MM_ERROR_NONE, "%x", err);

	return;
}






/** @} */




