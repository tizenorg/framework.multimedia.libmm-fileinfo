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

#ifndef _MMFILE_ID3_TAG_H_
#define _MMFILE_ID3_TAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mm_file_utils.h"
#include "mm_file_format_tags.h"

#define MMFILE_ID3TAG_FAIL      0
#define MMFILE_ID3TAG_SUCCESS   1

typedef void *MMFileID3TagHandle;

typedef enum mmfileId3TagInfoVersion {
	MMFILE_ID3TAG_V1_0 = 0,
	MMFILE_ID3TAG_V1_1,
	MMFILE_ID3TAG_V2_0,
	MMFILE_ID3TAG_V2_2,
	MMFILE_ID3TAG_V2_3,
	MMFILE_ID3TAG_V2_4,
	MMFILE_ID3TAG_VMAX,
} eMMFileID3TagVersion;

int MMFileID3V1TagFind(MMFileIOHandle *fp, unsigned char bAppended, unsigned int startOffset, unsigned int endOffset, tMMFileTags *out);
int MMFileID3V2TagFind(MMFileIOHandle *fp, unsigned char bAppended, unsigned int startOffset, unsigned int endOffset, tMMFileTags *out);

#ifdef __cplusplus
}
#endif

#endif /*_MMFILE_ID3_TAG_H_ */

