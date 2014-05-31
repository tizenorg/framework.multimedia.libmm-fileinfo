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

#ifndef _MMFILE_FORMAT_PRIVATE_H_
#define _MMFILE_FORMAT_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <mm_types.h>
#include "mm_file_formats.h"

/* open functions list: the order of list depends on mm-types.h */
int mmfile_format_open_dummy (MMFileFormatContext *fileContext);
int mmfile_format_open_ffmpg (MMFileFormatContext *fileContext);
int mmfile_format_open_mp3   (MMFileFormatContext *fileContext);
//int mmfile_format_open_3gp   (MMFileFormatContext *fileContext);
//int mmfile_format_open_avi   (MMFileFormatContext *fileContext);
//int mmfile_format_open_asf   (MMFileFormatContext *fileContext);
int mmfile_format_open_mmf   (MMFileFormatContext *fileContext);
int mmfile_format_open_amr   (MMFileFormatContext *fileContext);
int mmfile_format_open_aac   (MMFileFormatContext *fileContext);
int mmfile_format_open_wav   (MMFileFormatContext *fileContext);
int mmfile_format_open_mid   (MMFileFormatContext *fileContext);
int mmfile_format_open_imy   (MMFileFormatContext *fileContext);


#ifdef __cplusplus
}
#endif

#endif /* _MMFILE_FORMAT_PRIVATE_H_ */

