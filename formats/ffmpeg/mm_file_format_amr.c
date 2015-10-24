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

#include <stdio.h>
#include <string.h>	/*memcmp*/
#include <stdlib.h>	/*malloc*/


#include <mm_error.h>
#include "mm_file_debug.h"

#include "mm_file_utils.h"
#include "mm_file_format_private.h"
#include "mm_file_format_amr.h"


/* Media specific definations */
#define NUM_AMR_NB_MODES         8
#define NUM_AMR_WB_MODES         9

#define MMFILE_AMR_SINGLE_CH_HEADER_SIZE     6
#define MMFILE_AMR_SINGLE_CH_HEADER          "#!AMR\n"
#define MMFILE_AMR_WB_SINGLE_CH_HEADER_SIZE  9
#define MMFILE_AMR_WB_SINGLE_CH_HEADER       "#!AMR-WB\n"
#define MMFILE_AMR_MULTI_CH_HEADER_SIZE      12
#define MMFILE_AMR_MULTI_CH_HEADER           "#!AMR_MC1.0\n"
#define MMFILE_AMR_WB_MULTI_CH_HEADER_SIZE   15
#define MMFILE_AMR_WB_MULTI_CH_HEADER        "#!AMR-WB_MC1.0\n"

#define MMFILE_AMR_MAX_HEADER_SIZE  MMFILE_AMR_WB_MULTI_CH_HEADER_SIZE
#define MMFILE_AMR_MIN_HEADER_SIZE  MMFILE_AMR_SINGLE_CH_HEADER_SIZE

#define MMFILE_AMR_FRAME_DUR     20
#define AMR_NB_SAMPLES_PER_SEC   8000
#define AMR_WB_SAMPLES_PER_SEC   16000

#define AMR_MAX_READ_BUF_SZ	4096

#define AMR_GET_MODE(firstByte)  (((firstByte) >> 3) & 0x0F)


typedef enum _mmfile_amr_format_types {
	AMR_FORMAT_NB,
	AMR_FORMAT_WB,
	AMR_FORMAT_UNKNOWN
} eAmrFormatType;

typedef enum _mmfile_amr_channel_type {
	AMR_CHANNEL_TYPE_SINGLE,
	AMR_CHANNEL_TYPE_MULTIPLE,
	AMR_CHANNEL_TYPE_UNKNOWN
} eAmrChannelType;

typedef struct _mmfile_amr_handle {
	MMFileIOHandle *hFile;
	long long       duration;
	long long       fileSize;
	unsigned int    streamOffset;
	unsigned int    bitRate;
	unsigned int    samplingRate;
	unsigned int    frameRate;
	unsigned int    numAudioChannels;
	long long       numFrames;
	unsigned int    numTracks;
	int             amrMode;
	eAmrFormatType  amrFormat;
	eAmrChannelType amrChannelType;
} tMMFILE_AMR_HANDLE;


typedef struct _mmfile_amr_mode_config {
	unsigned int  bitRate;
	unsigned int  frameSize;
} tAmrModeConfig;

/*RTP format only supported*/
/*mode vs bitRate-frameSize lookup table; [0]->AMR-NB  [1]->AMR-WB */
const tAmrModeConfig AmrModeConfigTable[2][16] = {
	{
		{4750, 13}, {5150, 14}, {5900, 16}, {6700, 18},
		{7400, 20}, {7950, 21}, {10200,27}, {12200,32},
		{0,     6}, {0,     1}, {0,     1}, {0,     1},
		{0,     1}, {0,     1}, {0,     1}, {0,     1},
	},
	{
		{6600, 18}, {8850, 24}, {12650,33}, {14250,37},
		{15850,41}, {18250,47}, {19850,51}, {23050,59},
		{23850,61}, {0,     6}, {0,     6}, {0,     1},
		{0,     1}, {0,     1}, {0,     1}, {0,     1},
	}
};

/* internal APIs */

void _amr_init_handle(tMMFILE_AMR_HANDLE *pData)
{
	pData->hFile = NULL;
	pData->duration = 0;
	pData->fileSize = 0L;
	pData->streamOffset = 0;
	pData->bitRate = 0;
	pData->samplingRate = 0;
	pData->frameRate = 0;
	pData->numAudioChannels = 1;
	pData->numTracks = 1;
	pData->numFrames = 0;
	pData->amrChannelType = AMR_CHANNEL_TYPE_SINGLE;
}

int _parse_amr_header(tMMFILE_AMR_HANDLE *pData)
{

	unsigned char header[MMFILE_AMR_MAX_HEADER_SIZE];
	int ret = MMFILE_AMR_PARSER_SUCCESS;

	ret = mmfile_read(pData->hFile, header, MMFILE_AMR_MAX_HEADER_SIZE);
	if (ret != MMFILE_AMR_MAX_HEADER_SIZE) {
		return MMFILE_AMR_PARSER_FAIL;
	}

#ifdef __MMFILE_TEST_MODE__
	debug_msg("\nAMR HEADER: [%2x] [%2x] [%2x] [%2x] [%2x]\n   \
         [%2x] [%2x] [%2x] [%2x] [%2x]\n   \
         [%2x] [%2x] [%2x] [%2x] [%2x]\n",
	          header[0], header[1], header[2], header[3], header[4],
	          header[5], header[6], header[7], header[8], header[9],
	          header[10], header[11], header[12], header[13], header[14]);
#endif

	if (!(memcmp(header, MMFILE_AMR_SINGLE_CH_HEADER, MMFILE_AMR_SINGLE_CH_HEADER_SIZE))) {
		pData->amrFormat = AMR_FORMAT_NB;
		pData->amrChannelType = AMR_CHANNEL_TYPE_SINGLE;
		pData->streamOffset = MMFILE_AMR_SINGLE_CH_HEADER_SIZE;
	}

	else if (!(memcmp(header, MMFILE_AMR_WB_SINGLE_CH_HEADER, MMFILE_AMR_WB_SINGLE_CH_HEADER_SIZE))) {
		pData->amrFormat = AMR_FORMAT_WB;
		pData->amrChannelType = AMR_CHANNEL_TYPE_SINGLE;
		pData->streamOffset = MMFILE_AMR_WB_SINGLE_CH_HEADER_SIZE;
	}

	else if (!(memcmp(header, MMFILE_AMR_MULTI_CH_HEADER, MMFILE_AMR_MULTI_CH_HEADER_SIZE))) {
		pData->amrFormat = AMR_FORMAT_NB;
		pData->amrChannelType = AMR_CHANNEL_TYPE_MULTIPLE;
		pData->streamOffset = MMFILE_AMR_MULTI_CH_HEADER_SIZE;
	}

	else if (!(memcmp(header, MMFILE_AMR_WB_MULTI_CH_HEADER, MMFILE_AMR_WB_MULTI_CH_HEADER_SIZE))) {
		pData->amrFormat = AMR_FORMAT_WB;
		pData->amrChannelType = AMR_CHANNEL_TYPE_MULTIPLE;
		pData->streamOffset = MMFILE_AMR_WB_MULTI_CH_HEADER_SIZE;
	}

	else {
		pData->amrFormat = AMR_FORMAT_UNKNOWN;
		pData->amrChannelType = AMR_CHANNEL_TYPE_UNKNOWN;
		ret = MMFILE_AMR_PARSER_FAIL;
	}

	return ret;
}


int _parse_amr_stream(tMMFILE_AMR_HANDLE *pData)
{
	int frameLen = 0;
	unsigned char amrMode = 0;
	int ret = MMFILE_AMR_PARSER_SUCCESS;
	unsigned char *p;
	unsigned char *buf;
	int readed;
	int pos;
	long long sum_bitrate = 0;
	long long frames_bitrate = 0;

	buf = mmfile_malloc(AMR_MAX_READ_BUF_SZ);
	if (!buf) {
		debug_error("failed to memory allocaion.\n");
		return MMFILE_AMR_PARSER_FAIL;
	}

	for (readed = 0;;) {
		readed = mmfile_read(pData->hFile, buf, AMR_MAX_READ_BUF_SZ);
		if (readed <= 0) break;

		for (p = buf, pos = 0;;) {
			amrMode = AMR_GET_MODE((*(char *)p));
			frameLen = AmrModeConfigTable[pData->amrFormat][amrMode].frameSize;
			sum_bitrate += AmrModeConfigTable[pData->amrFormat][amrMode].bitRate;
			pData->numFrames++;
			frames_bitrate += (AmrModeConfigTable[pData->amrFormat][amrMode].bitRate == 0 ? 0 : 1);

			p += frameLen;
			pos += frameLen;
			if (pos == readed) {
				break;
			} else  if (pos > readed) {
				mmfile_seek(pData->hFile, (pos - readed), MMFILE_SEEK_CUR);
				break;
			}
		}
	}

	mmfile_free(buf);

	pData->duration = pData->numFrames * MMFILE_AMR_FRAME_DUR;
	pData->frameRate = 1000 / MMFILE_AMR_FRAME_DUR;

	if (frames_bitrate) {
		pData->bitRate = sum_bitrate / frames_bitrate;
	}

	return ret;
}


int mmfile_amrparser_open(MMFileAMRHandle *handle, const char *filenamec)
{
	tMMFILE_AMR_HANDLE *privateData = NULL;
	int ret = 0;

	if (NULL == filenamec || NULL == handle) {
		debug_error("file source is NULL\n");
		return MMFILE_AMR_PARSER_FAIL;
	}

	privateData = mmfile_malloc(sizeof(tMMFILE_AMR_HANDLE));
	if (NULL == privateData) {
		debug_error("file source is NULL\n");
		return MMFILE_AMR_PARSER_FAIL;
	}

	/* Initialize the members of handle */
	_amr_init_handle(privateData);

	ret = mmfile_open(&privateData->hFile, filenamec, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error("error: mmfile_open\n");
		goto exception;
	}

	mmfile_seek(privateData->hFile, 0, MMFILE_SEEK_END);
	privateData->fileSize = mmfile_tell(privateData->hFile);
	mmfile_seek(privateData->hFile, 0, MMFILE_SEEK_SET);

	if (privateData->fileSize < MMFILE_AMR_MIN_HEADER_SIZE) {
		debug_error("Too small file to parse!!\n");
		goto exception;
	}

	ret = _parse_amr_header(privateData);
	if (ret == MMFILE_AMR_PARSER_FAIL) {
		debug_error("Invalid AMR header\n");
		goto exception;
	}

	if (privateData->amrChannelType != AMR_CHANNEL_TYPE_SINGLE) {
		debug_error("Unsupported channel mode\n"); /*Need to study AMR_Format.txt, Pg:36*/
		goto exception;
	}

#ifdef __MMFILE_TEST_MODE__
	debug_msg("AMR Format Type: %s\n", \
	          privateData->amrFormat == AMR_FORMAT_NB ? "AMR-NB" : "AMR-WB");
#endif

	*handle = privateData;

	return MMFILE_AMR_PARSER_SUCCESS;

exception:
	if (privateData) {
		mmfile_close(privateData->hFile);
		mmfile_free(privateData);
		*handle = NULL;
	}
	return MMFILE_AMR_PARSER_FAIL;

}


int mmfile_amrparser_get_stream_info(MMFileAMRHandle handle, tMMFILE_AMR_STREAM_INFO *amrinfo)
{
	tMMFILE_AMR_HANDLE *privateData = NULL;
	int ret;

	if (NULL == handle || NULL == amrinfo) {
		debug_error("handle is NULL\n");
		return MMFILE_AMR_PARSER_FAIL;
	}

	privateData = (tMMFILE_AMR_HANDLE *) handle;

	mmfile_seek(privateData->hFile, privateData->streamOffset, MMFILE_SEEK_SET);

	ret = _parse_amr_stream(privateData);
	if (ret == MMFILE_AMR_PARSER_FAIL) {
		debug_error("Error in parsing the stream\n");
		return ret;
	}

	amrinfo->duration = privateData->duration;
	amrinfo->fileSize = privateData->fileSize;
	amrinfo->bitRate = privateData->bitRate;
	amrinfo->frameRate = privateData->frameRate;
	amrinfo->numAudioChannels = 1;
	amrinfo->numTracks = 1;

	if (privateData->amrFormat == AMR_FORMAT_NB) {
		amrinfo->samplingRate = AMR_NB_SAMPLES_PER_SEC;
	} else {
		amrinfo->samplingRate = AMR_WB_SAMPLES_PER_SEC;
	}

	return MMFILE_AMR_PARSER_SUCCESS;
}


int mmfile_amrparser_close(MMFileAMRHandle handle)
{
	tMMFILE_AMR_HANDLE *privateData = NULL;

	if (NULL == handle) {
		debug_error("handle is NULL\n");
		return MMFILE_AMR_PARSER_FAIL;
	}

	privateData = (tMMFILE_AMR_HANDLE *) handle;

	mmfile_close(privateData->hFile);

	return MMFILE_AMR_PARSER_SUCCESS;
}



/* mm plugin interface */
int mmfile_format_read_stream_amr(MMFileFormatContext *formatContext);
int mmfile_format_read_frame_amr(MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int mmfile_format_read_tag_amr(MMFileFormatContext *formatContext);
int mmfile_format_close_amr(MMFileFormatContext *formatContext);


EXPORT_API
int mmfile_format_open_amr(MMFileFormatContext *formatContext)
{
	MMFileAMRHandle handle = NULL;
	int res = MMFILE_FORMAT_FAIL;

	if (NULL == formatContext || NULL == formatContext->uriFileName) {
		debug_error("error: mmfile_format_open_amr\n");
		return MMFILE_FORMAT_FAIL;
	}

	formatContext->ReadStream   = mmfile_format_read_stream_amr;
	formatContext->ReadFrame    = mmfile_format_read_frame_amr;
	formatContext->ReadTag      = mmfile_format_read_tag_amr;
	formatContext->Close        = mmfile_format_close_amr;

	formatContext->videoTotalTrackNum = 0;
	formatContext->audioTotalTrackNum = 1;

	res = mmfile_amrparser_open(&handle, formatContext->uriFileName);
	if (MMFILE_AMR_PARSER_FAIL == res) {
		debug_error("mmfile_amrparser_open\n");
		return MMFILE_FORMAT_FAIL;
	}

	formatContext->privateFormatData = handle;

	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_read_stream_amr(MMFileFormatContext *formatContext)
{
	MMFileAMRHandle     handle = NULL;
	tMMFILE_AMR_STREAM_INFO  amrinfo = {0, };
	MMFileFormatStream  *audioStream = NULL;

	int ret = MMFILE_FORMAT_FAIL;

	if (NULL == formatContext) {
		debug_error("error: invalid params\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	handle = formatContext->privateFormatData;

	ret = mmfile_amrparser_get_stream_info(handle, &amrinfo);
	if (MMFILE_FORMAT_SUCCESS != ret) {
		debug_error("error: mmfile_amrparser_get_stream_info\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	formatContext->duration = amrinfo.duration;
	formatContext->videoStreamId = -1;
	formatContext->videoTotalTrackNum = 0;
	formatContext->audioTotalTrackNum = amrinfo.numTracks;
	formatContext->nbStreams = 1;

	audioStream = mmfile_malloc(sizeof(MMFileFormatStream));
	if (NULL == audioStream) {
		debug_error("error: calloc_audiostream\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	audioStream->streamType = MMFILE_AUDIO_STREAM;
	audioStream->codecId = MM_AUDIO_CODEC_AMR;
	audioStream->bitRate = amrinfo.bitRate;
	audioStream->framePerSec = amrinfo.frameRate;
	audioStream->width = 0;
	audioStream->height = 0;
	audioStream->nbChannel = amrinfo.numAudioChannels;
	audioStream->samplePerSec = amrinfo.samplingRate;
	formatContext->streams[MMFILE_AUDIO_STREAM] = audioStream;

#ifdef  __MMFILE_TEST_MODE__
	mmfile_format_print_contents(formatContext);
#endif

	return MMFILE_FORMAT_SUCCESS;

exception:
	return ret;
}

EXPORT_API
int mmfile_format_read_tag_amr(MMFileFormatContext *formatContext)
{
	return MMFILE_FORMAT_SUCCESS;
}


EXPORT_API
int mmfile_format_read_frame_amr(MMFileFormatContext *formatContext,
                                 unsigned int timestamp, MMFileFormatFrame *frame)
{
	debug_error("error: mmfile_format_read_frame_amr, no handling\n");

	return MMFILE_FORMAT_FAIL;
}


EXPORT_API
int mmfile_format_close_amr(MMFileFormatContext *formatContext)
{
	MMFileAMRHandle  handle = NULL;
	int ret = MMFILE_FORMAT_FAIL;

	if (NULL == formatContext) {
		debug_error("error: invalid params\n");
		return MMFILE_FORMAT_FAIL;
	}

	handle = formatContext->privateFormatData;

	if (NULL != handle) {
		ret = mmfile_amrparser_close(handle);
		if (ret == MMFILE_AMR_PARSER_FAIL) {
			debug_error("error: mmfile_format_close_amr\n");
		}
	}

	if (formatContext->streams[MMFILE_AUDIO_STREAM]) {
		mmfile_free(formatContext->streams[MMFILE_AUDIO_STREAM]);
		formatContext->streams[MMFILE_AUDIO_STREAM] = NULL;
	}

	formatContext->ReadStream   = NULL;
	formatContext->ReadFrame    = NULL;
	formatContext->ReadTag      = NULL;
	formatContext->Close        = NULL;

	return MMFILE_FORMAT_SUCCESS;
}


