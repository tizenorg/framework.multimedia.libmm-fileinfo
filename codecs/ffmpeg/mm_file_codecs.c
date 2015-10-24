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

#include <string.h>
#include <stdlib.h>

#include "mm_file_debug.h"
#include "mm_file_utils.h"
#include "mm_file_codec_private.h"

int (*OpenVideoCodecFunc[MM_VIDEO_CODEC_NUM]) (MMFileCodecContext *codecContext, MMFileCodecFrame *input) = {
	mmfile_codec_open_dummy,         /* NONE */
	mmfile_codec_open_dummy,         /* WMV */
	mmfile_codec_open_dummy,         /* H261 */
	mmfile_codec_open_dummy,         /* H262 */
	mmfile_codec_open_dummy,         /* H263 */
	mmfile_codec_open_dummy,         /* H263V2 */
	mmfile_codec_open_dummy,         /* H263V3 */
	mmfile_codec_open_dummy,         /* H264 */
	mmfile_codec_open_dummy,         /* H26L */
	mmfile_codec_open_dummy,         /* MJPEG */
	mmfile_codec_open_dummy,         /* MPEG1 */
	mmfile_codec_open_dummy,         /* MPEG2 */
	mmfile_codec_open_dummy,         /* MPEG4 */
	mmfile_codec_open_dummy,         /* MPEG4_SIMPLE */
	mmfile_codec_open_dummy,         /* MPEG4_ADV_SIMPE */
	mmfile_codec_open_dummy,         /* MPEG4_MAIN */
	mmfile_codec_open_dummy,         /* MPEG4_CORE */
	mmfile_codec_open_dummy,         /* MPEG4_ACE */
	mmfile_codec_open_dummy,         /* MPEG4_ARTS */
	mmfile_codec_open_dummy,         /* MPEG4_AVC */
	mmfile_codec_open_dummy,         /* REAL */
	mmfile_codec_open_dummy,         /* VC1 */
	mmfile_codec_open_dummy,         /* AVS */
	mmfile_codec_open_dummy,         /* CINEPAK */
	mmfile_codec_open_dummy,         /* INDEO */
	mmfile_codec_open_dummy,         /* THEORA */
	mmfile_codec_open_dummy,         /* DIVX */
	mmfile_codec_open_dummy,         /* XVID */
};

EXPORT_API
int mmfile_codec_open(MMFileCodecContext **codecContext, int codecType, int codecId, MMFileCodecFrame *input)
{
	MMFileCodecContext *codecObject = NULL;
	int ret = 0;

	if (codecId <= MM_VIDEO_CODEC_NONE || codecId >= MM_VIDEO_CODEC_NUM || MMFILE_VIDEO_DECODE != codecType || NULL == input) {
		debug_error("error: invalid params\n");
		return MMFILE_CODEC_FAIL;
	}

	if (NULL == OpenVideoCodecFunc[codecId]) {
		debug_error("error: Not implemented \n");
		return MMFILE_CODEC_FAIL;
	}

	codecObject = mmfile_malloc(sizeof(MMFileCodecContext));
	if (NULL == codecObject) {
		debug_error("error: mmfile_malloc fail for codecObject\n");
		return MMFILE_CODEC_FAIL;
	}

	*codecContext = codecObject;

	ret = OpenVideoCodecFunc[codecId](codecObject, input);
	if (MMFILE_CODEC_FAIL == ret) {
		debug_error("error: init fail about video codec\n");
		ret = MMFILE_CODEC_FAIL;
		goto exception;
	}

	return MMFILE_CODEC_SUCCESS;

exception:
	if (codecObject)    mmfile_free(codecObject);

	return ret;
}

EXPORT_API
int mmfile_codec_decode(MMFileCodecContext *codecContext, MMFileCodecFrame *output)
{
	if (NULL == codecContext || NULL == codecContext->Decode) {
		debug_error("error: invalid params\n");
		return MMFILE_CODEC_FAIL;
	}

	return codecContext->Decode(codecContext, output);
}

EXPORT_API
int mmfile_codec_close(MMFileCodecContext *codecContext)
{
	if (NULL == codecContext || NULL == codecContext->Close) {
		debug_error("error: invalid params\n");
		return MMFILE_CODEC_FAIL;
	}

	codecContext->Close(codecContext);

	if (codecContext->Decode)       codecContext->Decode = NULL;
	if (codecContext->Close)        codecContext->Close = NULL;
	if (codecContext->privateData)  mmfile_free(codecContext->privateData);
	mmfile_free(codecContext);

	return MMFILE_CODEC_SUCCESS;
}

