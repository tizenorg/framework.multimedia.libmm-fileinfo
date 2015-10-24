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

#include "mm_file_codec_private.h"
#include "mm_file_codec_dummy.h"


/* internal functions */


/* plugin manadatory API */
int mmfile_codec_decode_dummy(MMFileCodecContext *codecContext, MMFileCodecFrame *output);
int mmfile_codec_close_dummy(MMFileCodecContext *codecContext);


EXPORT_API
int mmfile_codec_open_dummy(MMFileCodecContext *codecContext, MMFileCodecFrame *input)
{
	debug_warning("called mmfile_codec_open_dummy\n");

	codecContext->Decode = mmfile_codec_decode_dummy;
	codecContext->Close  = mmfile_codec_close_dummy;

	return MMFILE_CODEC_SUCCESS;
}

EXPORT_API
int mmfile_codec_decode_dummy(MMFileCodecContext *codecContext, MMFileCodecFrame *output)
{
	debug_warning("called mmfile_codec_decode_dummy\n");
	return MMFILE_CODEC_SUCCESS;
}

EXPORT_API
int mmfile_codec_close_dummy(MMFileCodecContext *codecContext)
{
	debug_warning("called mmfile_codec_close_dummy\n");

	codecContext->Decode   = NULL;
	codecContext->Close    = NULL;

	return MMFILE_CODEC_SUCCESS;
}

