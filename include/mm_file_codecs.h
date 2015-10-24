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

#ifndef _MMFILE_CODECS_H_
#define _MMFILE_CODECS_H_

#ifdef __cplusplus
extern "C" {
#endif


#define MMFILE_CODEC_SUCCESS   1
#define MMFILE_CODEC_FAIL      0

#define MMFILE_VIDEO_DECODE    0
#define MMFILE_AUDIO_DECODE    1


typedef struct _mmfileframe {
	unsigned int width;
	unsigned int height;
	unsigned int version;
	unsigned int configLen;
	unsigned int frameDataSize;
	unsigned char *frameData;
	void          *configData;
} MMFileCodecFrame;


typedef struct _mmfilecodecctx MMFileCodecContext;

struct _mmfilecodecctx {
	/* MMFILE_AUDIO_DECODE or MMFILE_VIDEO_DECODE */
	int codecType;
	int codecId;
	int version;

	/* private data */
	void *privateData;

	/* resource free */
	int (*Decode)(MMFileCodecContext *, MMFileCodecFrame *);
	int (*Close)(MMFileCodecContext *);
};

#ifndef __MMFILE_DYN_LOADING__
int mmfile_codec_open(MMFileCodecContext **codecContext, int codecType, int codecId, MMFileCodecFrame *input);
int mmfile_codec_decode(MMFileCodecContext *codecContext, MMFileCodecFrame *output);
int mmfile_codec_close(MMFileCodecContext *codecContext);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MMFILE_CODECS_H_ */

