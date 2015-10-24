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

#ifndef __MM_FILE_FORMAT_MIDI_H__
#define __MM_FILE_FORMAT_MIDI_H__

#include "mm_file_formats.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int duration;
	int track_num;
	int is_xmf;

	char *title;
	char *copyright;
	char *comment;
} MIDI_INFO_SIMPLE;

MIDI_INFO_SIMPLE *mmfile_format_get_midi_infomation(char *uri);
void mmfile_format_free_midi_infomation(MIDI_INFO_SIMPLE *info);

#ifdef __cplusplus
}
#endif

#endif /*__MM_FILE_FORMAT_MIDI_H__*/

