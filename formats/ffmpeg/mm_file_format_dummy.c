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

#include <string.h>	/*memcmp*/
#include <stdlib.h>	/*malloc*/

#include <mm_error.h>

#include "mm_file_debug.h"

#include "mm_file_format_private.h"
#include "mm_file_format_dummy.h"


/* internal functions */


/* plugin manadatory API */
int mmfile_format_read_stream_dummy(MMFileFormatContext *formatContext);
int mmfile_format_read_frame_dummy(MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int mmfile_format_read_tag_dummy(MMFileFormatContext *formatContext);
int mmfile_format_close_dummy(MMFileFormatContext *formatContext);


EXPORT_API
int mmfile_format_open_dummy(MMFileFormatContext *formatContext)
{
	debug_warning("called mmfile_format_open_dummy\n");

	formatContext->ReadStream   = mmfile_format_read_stream_dummy;
	formatContext->ReadFrame    = mmfile_format_read_frame_dummy;
	formatContext->ReadTag      = mmfile_format_read_tag_dummy;
	formatContext->Close        = mmfile_format_close_dummy;

	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_read_stream_dummy(MMFileFormatContext *formatContext)
{
	debug_warning("called mmfile_format_read_stream_dummy\n");
	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_read_frame_dummy(MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame)
{
	debug_warning("called mmfile_format_read_frame_dummy\n");
	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_read_tag_dummy(MMFileFormatContext *formatContext)
{
	debug_warning("called mmfile_format_read_tag_dummy\n");
	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_close_dummy(MMFileFormatContext *formatContext)
{
	debug_warning("called mmfile_format_close_dummy\n");
	if (formatContext) {
		formatContext->ReadStream   = NULL;
		formatContext->ReadFrame    = NULL;
		formatContext->ReadTag      = NULL;
		formatContext->Close        = NULL;
	}

	return MMFILE_FORMAT_SUCCESS;
}

