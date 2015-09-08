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
#include <stdbool.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include <mm_types.h>
#include "mm_debug.h"
#include "mm_file_formats.h"
#include "mm_file_utils.h"
#include "mm_file_format_ffmpeg_mem.h"
#include "mm_file_format_frame.h"

#define MILLION 1000000
#ifdef MMFILE_FORMAT_DEBUG_DUMP
static void __save_frame(AVFrame *pFrame, int width, int height, int iFrame);

void __save_frame(AVFrame *pFrame, int width, int height, int iFrame) {
	FILE *pFile;
	char szFilename[32];
	int y;
	// Open file
	sprintf(szFilename, "frame%d.ppm", iFrame);
	pFile=fopen(szFilename, "wb");
	if(pFile==NULL)
		return;

	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);
	// Write pixel data
	for(y=0; y<height; y++)
		fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

	// Close file
	fclose(pFile);
}
#endif

static int __getMimeType(int formatId, char *mimeType)
{
	int ret = 0;	/*default: success*/

	switch(formatId) {
		case MM_FILE_FORMAT_3GP:
		case MM_FILE_FORMAT_MP4:
			sprintf(mimeType,"video/3gpp");
			break;
		case MM_FILE_FORMAT_ASF:
		case MM_FILE_FORMAT_WMA:
		case MM_FILE_FORMAT_WMV:
			sprintf(mimeType,"video/x-ms-asf");
			break;
		case  MM_FILE_FORMAT_AVI:
			sprintf(mimeType,"video/avi");
			break;
		case MM_FILE_FORMAT_OGG:
			sprintf(mimeType,"video/ogg");
			break;
		case MM_FILE_FORMAT_REAL:
			sprintf(mimeType,"video/vnd.rn-realvideo");
			break;
		case MM_FILE_FORMAT_AMR:
			sprintf(mimeType,"audio/AMR");
			break;
		case MM_FILE_FORMAT_AAC:
			sprintf(mimeType,"audio/aac");
			break;
		case MM_FILE_FORMAT_MP3:
			sprintf(mimeType,"audio/mp3");
			break;
		case MM_FILE_FORMAT_AIFF:
		case MM_FILE_FORMAT_WAV:
			sprintf(mimeType,"audio/wave");
			break;
		case MM_FILE_FORMAT_MID:
			sprintf(mimeType,"audio/midi");
			break;
		case MM_FILE_FORMAT_MMF:
			sprintf(mimeType,"audio/mmf");
			break;
		case MM_FILE_FORMAT_DIVX:
			sprintf(mimeType,"video/divx");
			break;
		case MM_FILE_FORMAT_IMELODY:
			sprintf(mimeType,"audio/iMelody");
			break;
		case MM_FILE_FORMAT_JPG:
			sprintf(mimeType,"image/jpeg");
			break;
		case MM_FILE_FORMAT_AU:
			sprintf(mimeType,"audio/basic");
			break;
		case MM_FILE_FORMAT_VOB:
			sprintf(mimeType,"video/mpeg");
			break;
		case MM_FILE_FORMAT_FLV:
			sprintf(mimeType,"video/x-flv");
			break;
		case MM_FILE_FORMAT_QT:
			sprintf(mimeType,"video/quicktime");
			break;
		case MM_FILE_FORMAT_MATROSKA:
			sprintf(mimeType,"video/x-matroska");
			break;
		case MM_FILE_FORMAT_FLAC:
			sprintf(mimeType,"audio/x-flac");
			break;
		default:
			ret = -1;
	}

#ifdef __MMFILE_TEST_MODE__
	debug_msg ("id: %d, mimetype: %s\n", formatId, mimeType);
#endif

	return ret;
}
static int __get_fileformat(const char *urifilename, int *format)
{
	int index;

	debug_error("%s\n", urifilename);

	for (index = 0; index < MM_FILE_FORMAT_NUM; index++) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("search index = [%d]\n", index);
		#endif
		switch (index) {
			case MM_FILE_FORMAT_QT:
			case MM_FILE_FORMAT_3GP:
			case MM_FILE_FORMAT_MP4: {
				if (MMFileFormatIsValidMP4 (urifilename)) {
					*format = MM_FILE_FORMAT_3GP;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_ASF:
			case MM_FILE_FORMAT_WMA:
			case MM_FILE_FORMAT_WMV: {
				if (MMFileFormatIsValidASF (urifilename)) {
					*format = MM_FILE_FORMAT_ASF;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_DIVX:
			case MM_FILE_FORMAT_AVI: {
				if (MMFileFormatIsValidAVI(urifilename)) {
					*format = MM_FILE_FORMAT_AVI;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_MATROSKA: {
				if (MMFileFormatIsValidMatroska (urifilename)) {
					*format = MM_FILE_FORMAT_MATROSKA;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_FLV: {
				if (MMFileFormatIsValidFLV (urifilename)) {
					*format = MM_FILE_FORMAT_FLV;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			/* this is not video file format */
			case MM_FILE_FORMAT_OGG:
			case MM_FILE_FORMAT_AMR:
			case MM_FILE_FORMAT_AAC:
			case MM_FILE_FORMAT_MP3:
			case MM_FILE_FORMAT_WAV:
			case MM_FILE_FORMAT_MID:
			case MM_FILE_FORMAT_MMF:
			case MM_FILE_FORMAT_IMELODY:
			case MM_FILE_FORMAT_FLAC:
				break;
			/* not supported file */
			case MM_FILE_FORMAT_NUT:
			case MM_FILE_FORMAT_REAL:
			case MM_FILE_FORMAT_AIFF:
			case MM_FILE_FORMAT_AU:
			case MM_FILE_FORMAT_VOB:
			case MM_FILE_FORMAT_JPG:
				break;
			default: {
				debug_error ("error: invaild format enum[%d]\n", index);
				break;
			}
		}
	}

	if (index == MM_FILE_FORMAT_NUM) {
		debug_error("Can't probe file type\n");
	}

	*format = -1;
	return MMFILE_FORMAT_FAIL;
}

static int __mmfile_get_frame(AVFormatContext *pFormatCtx, double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height)
{

	int i = 0;
	int len = 0;
	int ret = MMFILE_FORMAT_SUCCESS;
	int videoStream = -1;
	int key_detected = 0;
	int got_picture = 0;
	double pos = timestamp;
	bool find = false;
	bool first_seek = true;
	int64_t pts = 0;
	AVCodecContext *pVideoCodecCtx = NULL;
	AVCodec *pVideoCodec = NULL;
	AVFrame *pFrame = NULL, *pFrameRGB = NULL;
	AVPacket packet;

	/* Retrieve stream information */
#ifdef __MMFILE_FFMPEG_V100__
	if(avformat_find_stream_info(pFormatCtx, NULL) < 0) {
#else
	if(av_find_stream_info(pFormatCtx) < 0) {
#endif
		debug_error("error : av_find_stream_info failed");
		ret = MMFILE_FORMAT_FAIL;
		goto exception; /* Couldn't find stream information */
	}

	/* Find the first video stream */
	for(i = 0; i < pFormatCtx->nb_streams; i++) {
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}
	}

	if(videoStream == -1) {
		debug_error("error : videoStream == -1");
		ret = MMFILE_FORMAT_FAIL;
		goto exception; /* Didn't find a video stream */
	}

	/* Get a pointer to the codec context for the video stream */
	pVideoCodecCtx=pFormatCtx->streams[videoStream]->codec;
	if (pVideoCodecCtx == NULL) {
		debug_error ("invalid param\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	/* Find the decoder for the video stream */
	pVideoCodec = avcodec_find_decoder(pVideoCodecCtx->codec_id);
	if(pVideoCodec == NULL) {
		debug_error("error : Unsupported codec");
		ret = MMFILE_FORMAT_FAIL;
		goto exception; /* Codec not found */
	}

	/* Open codec */
#ifdef __MMFILE_FFMPEG_V100__
	pVideoCodecCtx->thread_type = 0;
	pVideoCodecCtx->thread_count = 0;
	if(avcodec_open2(pVideoCodecCtx, pVideoCodec, NULL) < 0) {
#else
	if(avcodec_open(pVideoCodecCtx, pVideoCodec) < 0) {
#endif
		debug_error("error : avcodec_open failed");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;; /*Could not open codec */
	}

	/* Storing Data  */
	/* Allocate video frame */
	pFrame = avcodec_alloc_frame();
	if(pFrame == NULL) {
		debug_error ("error: pFrame is NULL\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	/* Allocate an AVFrame structure */
	pFrameRGB = avcodec_alloc_frame();
	if(pFrameRGB == NULL) {
		debug_error ("error: pFrameRGB is NULL\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	/* Seeking */
	AVStream *pStream = pFormatCtx->streams[videoStream];
	double duration = (double) pFormatCtx->duration / AV_TIME_BASE;
#if 0
	if (duration <= 0) {
		double tmpDuration = 0.0;

		if (pStream->codec->bit_rate > 0 && pFormatCtx->file_size > 0) {
			if (pStream->codec->bit_rate >= 8)
				tmpDuration = 0.9 * pFormatCtx->file_size / (pStream->codec->bit_rate / 8);

			if (tmpDuration > 0)
				duration = tmpDuration;
		}
	}
#endif
	duration = duration * MILLION;
	if ((duration <= 0) ||(duration <= pos)) {
		debug_error("duration error duration[%f] pos[%f]", duration, pos);
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	if (is_accurate)
		av_seek_frame(pFormatCtx, -1, pos, AVSEEK_FLAG_ANY);
	else
		av_seek_frame(pFormatCtx, -1, pos, AVSEEK_FLAG_BACKWARD);

	/* Reading Data */
	int64_t tmpPts = 0;
#ifdef __MMFILE_TEST_MODE__
	int idx = 0;
#endif

	av_init_packet(&packet);

	while(av_read_frame(pFormatCtx, &packet) >= 0) {
		got_picture = 0;

		// Is this a packet from the video stream?
		if(packet.stream_index == videoStream) {
#ifdef __MMFILE_TEST_MODE__
			debug_msg("find Video Stream+++++++[%2d]", idx++);
#endif
			/* Decode video frame*/
			len = avcodec_decode_video2(pVideoCodecCtx, pFrame, &got_picture, &packet);
			if (len < 0) {
					debug_warning ("Error while decoding frame");
			} else if ((packet.flags & AV_PKT_FLAG_KEY) || (key_detected == 1)) {

				key_detected = 0;

				if (first_seek || !is_accurate) {
					/* This is first seeking or not accurate mode.
					Sometimes flag is AV_PKT_FLAG_KEY but got_picture is NULL.
					first_seek is used when accurate mode and when time stamp's frame is not key frame.
					Go back to previousto Key frame and decode frame until time stamp's frame*/

					if (got_picture) {
						if(pFrame->key_frame) {
							#ifdef __MMFILE_TEST_MODE__
							debug_msg("find Video Stream+++++++Find key frame");
							#endif
						} else {
						#ifdef __MMFILE_TEST_MODE__
							debug_msg("find Video Stream+++++++ not key frame");
						#endif
						}

						/*eventhough decoded pFrame is not key frame, if packet.flags is AV_PKT_FLAG_KEY then can extract frame*/
						find = true;

					} else {
						#ifdef __MMFILE_TEST_MODE__
						debug_msg("find Video Stream+++++++Find key but no frame");
						#endif
						key_detected = 1;
					}
				}
			} else {
				if(is_accurate) {
					if (first_seek) {
						pts = (packet.pts == AV_NOPTS_VALUE) ? (packet.dts * av_q2d(pStream->time_base)) : packet.pts;
						first_seek = false;

						av_seek_frame(pFormatCtx, -1, pos, AVSEEK_FLAG_BACKWARD);
					} else {
						tmpPts = (packet.pts == AV_NOPTS_VALUE) ? (packet.dts * av_q2d(pStream->time_base)) : packet.pts;
						if (pts == tmpPts)
							find = true;
					}
				}
			}

			if(find && got_picture) {
				break;
			}
		}

		/* Free the packet that was allocated by av_read_frame*/
		av_free_packet(&packet);
		av_init_packet(&packet);
	}

	/*free pkt after loop breaking*/
	av_free_packet (&packet);

	/* Did we get a video frame?*/
	if(got_picture && find) {

#ifdef __MMFILE_TEST_MODE__
		debug_msg("Find Frame");
#endif
		/* return frame infromations*/
		if((pVideoCodecCtx->width == 0) || (pVideoCodecCtx->height == 0)) {
			*width = pVideoCodecCtx->coded_width;
			*height = pVideoCodecCtx->coded_height;
		} else {
			*width = pVideoCodecCtx->width;
			*height = pVideoCodecCtx->height;
		}

		*size = avpicture_get_size(PIX_FMT_RGB24, *width, *height);
		*frame = mmfile_malloc (*size);
		if (NULL == *frame) {
			debug_error ("error: avpicture_get_size. [%d]\n", size);
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}

#ifdef __MMFILE_TEST_MODE__
		debug_msg("size : %d", *size);
		debug_msg("width : %d", *width);
		debug_msg("height : %d", *height);
		debug_msg("frame : %x", *frame);
#endif
		ret = avpicture_fill ((AVPicture *)pFrameRGB, *frame, PIX_FMT_RGB24, *width, *height);
		if (ret < 0) {
			debug_error ("error: avpicture_fill fail. errcode = 0x%08X\n", ret);
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}

#ifdef __MMFILE_FFMPEG_V085__
		struct SwsContext *img_convert_ctx = NULL;

		img_convert_ctx = sws_getContext (*width, *height, pVideoCodecCtx->pix_fmt,
		                          *width, *height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

		if (NULL == img_convert_ctx) {
			debug_error ("failed to get img convet ctx\n");
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}

		ret = sws_scale (img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize,
		     0, *height, pFrameRGB->data, pFrameRGB->linesize);
		if ( ret < 0 ) {
			debug_error ("failed to convet image\n");
			sws_freeContext(img_convert_ctx);
			img_convert_ctx = NULL;
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}

		sws_freeContext(img_convert_ctx);
		img_convert_ctx = NULL;
#else
		ret = img_convert ((AVPicture *)pFrameRGB, PIX_FMT_RGB24, (AVPicture*)pFrame, pVideoCodecCtx->pix_fmt, *width, *height);
		if ( ret < 0 ) {
			debug_error ("failed to convet image\n");
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}
#endif

#ifdef MMFILE_FORMAT_DEBUG_DUMP
		__save_frame(pFrameRGB, pVideoCodecCtx->width, pVideoCodecCtx->height,  (int)(pos/1000));
#endif
	}
	else
	{
		debug_error("Not Found Proper Frame[%d][%d]", got_picture, find);
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	if (pFrame)			av_free (pFrame);
	if (pFrameRGB)		av_free (pFrameRGB);
	if (pVideoCodecCtx)	avcodec_close(pVideoCodecCtx);

	return MMFILE_FORMAT_SUCCESS;

exception:
	if (*frame)			{ mmfile_free (*frame); *frame = NULL; }
	if (pFrame)			av_free (pFrame);
	if (pFrameRGB)		av_free (pFrameRGB);
	if (pVideoCodecCtx) 	avcodec_close (pVideoCodecCtx);

	return ret;
}

int mmfile_format_get_frame(const char* path, double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height)
{
	int ret = MMFILE_FORMAT_SUCCESS;
	AVFormatContext *pFormatCtx = NULL;

	if (!size || !width || !height) {
		return MMFILE_FORMAT_FAIL;
	}

	av_register_all();

	/* Open video file */
	if(avformat_open_input(&pFormatCtx, path, NULL, NULL) != 0) {
		debug_error("error : avformat_open_input failed");
		return MMFILE_FORMAT_FAIL; /* Couldn't open file */
	}

	if (!pFormatCtx) {
		debug_warning ("failed to find av stream. maybe corrupted data.\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	ret = __mmfile_get_frame(pFormatCtx, timestamp, is_accurate, frame, size, width, height);

exception:
	/* Close video file */
	if (pFormatCtx) 		avformat_close_input(&pFormatCtx);

	return ret;
  }

int mmfile_format_get_frame_from_memory(const void *data, unsigned int datasize, double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height)
{
	int ret = MMFILE_FORMAT_SUCCESS;
	int format = -1;
	char mimeType[MMFILE_MIMETYPE_MAX_LEN] = {0,};
	char ffmpegFormatName[MMFILE_FILE_FMT_MAX_LEN] = {0,};
	char tempURIBuffer[MMFILE_URI_MAX_LEN] = {0,};
	char *urifilename = NULL;
	AVFormatContext *pFormatCtx = NULL;
	AVInputFormat *grab_iformat = NULL;

	if (!size || !width || !height) {
		return MMFILE_FORMAT_FAIL;
	}

	av_register_all();

	sprintf (tempURIBuffer, "%s%u:%u", MMFILE_MEM_URI, (unsigned int)data, datasize);
	urifilename = mmfile_strdup (tempURIBuffer);
	if (!urifilename) {
		debug_error ("error: uri is NULL\n");
		return MMFILE_FORMAT_FAIL;
	}

	mmfile_register_io_all();

	ret = __get_fileformat(urifilename, &format);
	if (ret != MMFILE_FORMAT_SUCCESS) {
		debug_error ("error: file format is invalid\n");
		return MMFILE_FORMAT_FAIL;
	}

#ifdef __MMFILE_FFMPEG_V085__
	ffurl_register_protocol(&MMFileMEMProtocol, sizeof (URLProtocol));
#else
	register_protocol (&MMFileMEMProtocol);
#endif

	if(__getMimeType(format,mimeType)< 0) {
		debug_error ("error: Error in MIME Type finding\n");
		return MMFILE_FORMAT_FAIL;
	}

	memset (ffmpegFormatName, 0x00, MMFILE_FILE_FMT_MAX_LEN);

	ret = mmfile_util_get_ffmpeg_format (mimeType,ffmpegFormatName);

	if (MMFILE_UTIL_SUCCESS != ret) {
		debug_error ("error: mmfile_util_get_ffmpeg_format\n");
		return MMFILE_FORMAT_FAIL;
	}

	grab_iformat = av_find_input_format (ffmpegFormatName);

	if (NULL == grab_iformat) {
		debug_error ("error: cannot find format\n");
		goto exception;
	}

#ifdef __MMFILE_FFMPEG_V085__
	ret = avformat_open_input (&pFormatCtx, urifilename, grab_iformat, NULL);
#else
	ret = av_open_input_file (&pFormatCtx, urifilename, grab_iformat, 0, NULL);
#endif
	if (ret < 0) {
		debug_error("error: cannot open %s %d\n", urifilename, ret);
		goto exception;
	}

	if (!pFormatCtx) {
		debug_warning ("failed to find av stream. maybe corrupted data.\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	ret = __mmfile_get_frame(pFormatCtx, timestamp, is_accurate, frame, size, width, height);

exception:
	/* Close video file */
	if (pFormatCtx) 		avformat_close_input(&pFormatCtx);

	return ret;
  }
