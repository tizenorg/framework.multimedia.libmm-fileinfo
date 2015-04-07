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

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/samplefmt.h>
#ifdef __MMFILE_FFMPEG_V085__
#include <libswscale/swscale.h>
#endif
#include <mm_error.h>
#include <mm_types.h>

#include "mm_debug.h"
#include "mm_file_formats.h"
#include "mm_file_utils.h"
#include "mm_file_format_ffmpeg.h"

#include "mm_file_format_ffmpeg_mem.h"
#include <sys/time.h>

#define _SHORT_MEDIA_LIMIT		2000	/* under X seconds duration*/

extern int	img_convert (AVPicture *dst, int dst_pix_fmt, const AVPicture *src, int src_pix_fmt,int src_width, int src_height);

/* internal functions */
static int _is_good_pgm (unsigned char *buf, int wrap, int xsize, int ysize);
#ifdef MMFILE_FORMAT_DEBUG_DUMP
static void _save_pgm (unsigned char *buf, int wrap, int xsize, int ysize, char *filename);
#endif
#ifdef __MMFILE_TEST_MODE__
static void _dump_av_packet (AVPacket *pkt);
#endif

static int	_get_video_fps (int frame_cnt, int duration, AVRational r_frame_rate, int is_roundup);
static int	_get_first_good_video_frame (AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int videoStream, AVFrame **pFrame, int cdis);

static int	ConvertVideoCodecEnum (int AVVideoCodecID);
static int	ConvertAudioCodecEnum (int AVAudioCodecID);

/* plugin manadatory API */
int mmfile_format_read_stream_ffmpg (MMFileFormatContext * formatContext);
int mmfile_format_read_frame_ffmpg  (MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int mmfile_format_read_tag_ffmpg    (MMFileFormatContext *formatContext);
int mmfile_format_close_ffmpg       (MMFileFormatContext *formatContext);
static int getMimeType(int formatId, char *mimeType); 



EXPORT_API
int mmfile_format_open_ffmpg (MMFileFormatContext *formatContext)
{
	AVFormatContext     *pFormatCtx = NULL;
	AVInputFormat       *grab_iformat = NULL;
	int ret = 0;
	int i;
	char ffmpegFormatName[MMFILE_FILE_FMT_MAX_LEN] = {0,};
	char mimeType[MMFILE_MIMETYPE_MAX_LEN] = {0,};

	formatContext->ReadStream   = mmfile_format_read_stream_ffmpg;
	formatContext->ReadFrame    = mmfile_format_read_frame_ffmpg;
	formatContext->ReadTag      = mmfile_format_read_tag_ffmpg;
	formatContext->Close        = mmfile_format_close_ffmpg;

#ifdef __MMFILE_TEST_MODE__
	debug_msg ("ffmpeg version: %d\n", avformat_version ());
	/**
	 * FFMPEG DEBUG LEVEL
	 *  AV_LOG_QUIET -1
	 *  AV_LOG_FATAL 0
	 *  AV_LOG_ERROR 0
	 *  AV_LOG_WARNING 1
	 *  AV_LOG_INFO 1
	 *  AV_LOG_VERBOSE 1
	 *  AV_LOG_DEBUG 2
     */
	av_log_set_level (AV_LOG_DEBUG);
#else
	av_log_set_level (AV_LOG_QUIET);
#endif

	av_register_all();

	if (formatContext->filesrc->type  == MM_FILE_SRC_TYPE_MEMORY) {

#ifdef __MMFILE_FFMPEG_V085__ 
		ffurl_register_protocol(&MMFileMEMProtocol, sizeof (URLProtocol));
#else
		register_protocol (&MMFileMEMProtocol);
#endif	
		if(getMimeType(formatContext->filesrc->memory.format,mimeType)< 0) {
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
		ret = avformat_open_input (&pFormatCtx, formatContext->uriFileName, grab_iformat, NULL);
#else
		ret = av_open_input_file (&pFormatCtx, formatContext->uriFileName, grab_iformat, 0, NULL);
#endif
		if (ret < 0) {
			debug_error("error: cannot open %s %d\n", formatContext->uriFileName, ret);
			goto exception;
		}
		formatContext->privateFormatData = pFormatCtx;
	}
	
	if (formatContext->filesrc->type  == MM_FILE_SRC_TYPE_FILE) {
#ifdef __MMFILE_FFMPEG_V085__
		ret = avformat_open_input(&pFormatCtx, formatContext->filesrc->file.path, NULL, NULL);
#else
		ret = av_open_input_file(&pFormatCtx, formatContext->filesrc->file.path, NULL, 0, NULL);
#endif
		if (ret < 0) {
			debug_error("error: cannot open %s %d\n", formatContext->filesrc->file.path, ret);
			goto exception;
		}
		formatContext->privateFormatData = pFormatCtx;
	}

	if (!pFormatCtx || !(pFormatCtx->nb_streams > 0)) {
		debug_warning ("failed to find av stream. maybe corrupted data.\n");
		goto exception;
	}

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("number of stream: %d\n", pFormatCtx->nb_streams);
	#endif

	formatContext->videoTotalTrackNum = 0;
	formatContext->audioTotalTrackNum = 0;

	for(i = 0; i < pFormatCtx->nb_streams; i++) {
#ifdef __MMFILE_FFMPEG_V085__		
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ("FFMPEG video codec id: 0x%08X\n", pFormatCtx->streams[i]->codec->codec_id);
			#endif

			AVPacket pkt = pFormatCtx->streams[i]->attached_pic;
			if((pkt.data != NULL) && (pkt.size > 0))
				continue;

			//eventhough codec_id is 0, avformat_find_stream_info() can find proper codec_id.
			formatContext->videoTotalTrackNum += 1;
		}
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ("FFMPEG audio codec id: 0x%08X\n", pFormatCtx->streams[i]->codec->codec_id);
			#endif
			formatContext->audioTotalTrackNum += 1;
		}
#else	
		if (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ("FFMPEG video codec id: 0x%08X\n", pFormatCtx->streams[i]->codec->codec_id);
			#endif
			formatContext->videoTotalTrackNum += 1;
		}
		if (pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO) {
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ("FFMPEG audio codec id: 0x%08X\n", pFormatCtx->streams[i]->codec->codec_id);
			#endif
			formatContext->audioTotalTrackNum += 1;
		}
#endif
	}

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("format: %s (%s)\n", pFormatCtx->iformat->name, pFormatCtx->iformat->long_name);
	#ifdef __MMFILE_FFMPEG_V085__
	av_dump_format (pFormatCtx, 0, formatContext->filesrc->file.path, 0);
	#else
	dump_format (pFormatCtx, 0, formatContext->filesrc->file.path, 0);
	#endif
	#endif

	return MMFILE_FORMAT_SUCCESS;

exception: /* fail to get content information */

	mmfile_format_close_ffmpg (formatContext);
	formatContext->privateFormatData = NULL;

	return MMFILE_FORMAT_FAIL;
}

static bool __check_uhqa(int sample_rate,  enum AVSampleFormat sample_fmt_info)
{
	bool ret = FALSE;

#ifdef __MMFILE_TEST_MODE__
	debug_error("[sample rate %d, sample format %d]", sample_rate, sample_fmt_info);
#endif

	if ((sample_rate >= 44100) && (sample_fmt_info >= AV_SAMPLE_FMT_S32)) {
#ifdef __MMFILE_TEST_MODE__
		debug_msg("UHQA CONTENT");
#endif
		ret = TRUE;
	} else {
		ret = FALSE;
	}

	return ret;
}

EXPORT_API
int mmfile_format_read_stream_ffmpg (MMFileFormatContext * formatContext)
{
	AVFormatContext     *pFormatCtx = NULL;
	AVCodecContext      *pAudioCodecCtx = NULL;
	AVCodecContext      *pVideoCodecCtx = NULL;

	MMFileFormatStream  *videoStream = NULL;
	MMFileFormatStream  *audioStream = NULL;
	int ret = 0;

	if (NULL == formatContext || NULL == formatContext->privateFormatData) {
		debug_error ("invalid param\n");
		return MMFILE_FORMAT_FAIL;
	}

	pFormatCtx = formatContext->privateFormatData;
	pFormatCtx->start_time = -1;

	/**
	 *@important if data is corrupted, occur segment fault by av_find_stream_info().
	 *			- fixed 2009-06-25.
	 */
#ifdef __MMFILE_FFMPEG_V100__
	if (formatContext->cdis != 1)
		ret = avformat_find_stream_info (pFormatCtx, NULL);
	else
		ret = 0;
#else
	ret = av_find_stream_info (pFormatCtx);
#endif
	if ( ret < 0 ) {
		debug_warning ("failed to find stream info. errcode = %d\n", ret);
		goto exception;
	}

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("FFMPEG: dur %lld, start %lld\n", pFormatCtx->duration, pFormatCtx->start_time);
	#endif

	/**
	 *@note asf has long duration bug. and Some content's start time is wrong(negative number).
	 */
	if(pFormatCtx->start_time < 0) {
		debug_warning ("Wrong Start time = %lld\n", pFormatCtx->start_time);
		formatContext->duration = (long long)(pFormatCtx->duration) * 1000 / AV_TIME_BASE;
	}
	else {
		formatContext->duration = (long long)(pFormatCtx->duration + pFormatCtx->start_time) * 1000 / AV_TIME_BASE;
	}

	formatContext->videoStreamId = -1;
	formatContext->audioStreamId = -1;
	formatContext->nbStreams = 0;

	int i = 0;
	for ( i = 0; i < pFormatCtx->nb_streams; i++ ) {
#ifdef __MMFILE_FFMPEG_V085__		
		if ( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
#else
		if ( pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
#endif
			if (formatContext->videoStreamId == -1) {
				videoStream = mmfile_malloc (sizeof(MMFileFormatStream));
				if (NULL == videoStream) {
					debug_error ("mmfile_malloc error\n");
					goto exception;
				}

				videoStream->streamType = MMFILE_VIDEO_STREAM;
				formatContext->streams[MMFILE_VIDEO_STREAM] = videoStream;
				formatContext->nbStreams += 1;
				formatContext->videoStreamId = i;

				pVideoCodecCtx = pFormatCtx->streams[i]->codec;
				if (pVideoCodecCtx) {
					videoStream->codecId		= ConvertVideoCodecEnum (pVideoCodecCtx->codec_id);
					if (videoStream->codecId == MM_VIDEO_CODEC_NONE) {
						debug_error("Proper codec is not found in [%d] stream", i);
						formatContext->videoStreamId = -1;
						mmfile_free(videoStream);
						formatContext->streams[MMFILE_VIDEO_STREAM] = NULL;
						videoStream = NULL;
						continue;
					}

					/**
					 * Get FPS
					 * 1. try to get average fps of video stream.
					 * 2. if (1) failed, try to get fps of media container.
					 */
					videoStream->framePerSec	= _get_video_fps (pFormatCtx->streams[i]->nb_frames,
																	pFormatCtx->streams[i]->duration,
																	pFormatCtx->streams[i]->time_base,
																	1);

					if (videoStream->framePerSec == 0)
						videoStream->framePerSec = av_q2d (pFormatCtx->streams[i]->r_frame_rate);

					videoStream->width			= pVideoCodecCtx->width;
					videoStream->height			= pVideoCodecCtx->height;
					videoStream->bitRate		= pVideoCodecCtx->bit_rate;
				}
			}
		} 
#ifdef __MMFILE_FFMPEG_V085__
		else if ( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO ) {
#else
		else if ( pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO ) {
#endif
			if (formatContext->audioStreamId == -1) {
				audioStream = mmfile_malloc (sizeof(MMFileFormatStream));
				if (NULL == audioStream) {
					debug_error ("mmfile_malloc error\n");
					goto exception;
				}

				audioStream->streamType = MMFILE_AUDIO_STREAM;
				formatContext->streams[MMFILE_AUDIO_STREAM] = audioStream;
				formatContext->nbStreams += 1;
				formatContext->audioStreamId = i;

				pAudioCodecCtx = pFormatCtx->streams[i]->codec;
				if (pAudioCodecCtx) {
					audioStream->codecId		= ConvertAudioCodecEnum (pAudioCodecCtx->codec_id);
					audioStream->bitRate		= pAudioCodecCtx->bit_rate;
					audioStream->nbChannel		= pAudioCodecCtx->channels;
					audioStream->samplePerSec	= pAudioCodecCtx->sample_rate;
					audioStream->bitPerSample = pAudioCodecCtx->bits_per_raw_sample;
					audioStream->is_uhqa = __check_uhqa(audioStream->samplePerSec, pFormatCtx->streams[i]->codec->sample_fmt);
				}
			}
		}
	}

	if ( formatContext->nbStreams == 0 ) {
		debug_error("error: there is no audio and video track\n");
		goto exception;
	}

	#ifdef __MMFILE_TEST_MODE__
	mmfile_format_print_contents (formatContext);
	#endif

	return MMFILE_FORMAT_SUCCESS;

exception:
	if (videoStream) {
		mmfile_free (videoStream);
		formatContext->streams[MMFILE_VIDEO_STREAM] = NULL;
	}

	if (audioStream) {
		mmfile_free (audioStream);
		formatContext->streams[MMFILE_AUDIO_STREAM] = NULL;
	}

	if (pFormatCtx) {
#ifdef __MMFILE_FFMPEG_V100__
		avformat_close_input (&pFormatCtx);
#else
		av_close_input_file (pFormatCtx);
#endif
		formatContext->privateFormatData = NULL;
	}

	formatContext->audioTotalTrackNum = 0;
	formatContext->videoTotalTrackNum = 0;
	formatContext->nbStreams = 0;

	return MMFILE_FORMAT_FAIL;
}

#define DATA_LENGTH 4
#define POS_OF_MIME_LEN DATA_LENGTH
#define CONVERT_TO_INT(dest, src) {dest = 0; dest |= (0 |src[0] << 24) | (0 | src[1] << 16) | (0 | src[2] << 8) | (0 | src[3]);}

EXPORT_API
int mmfile_format_read_tag_ffmpg (MMFileFormatContext *formatContext)
{
	AVFormatContext     *pFormatCtx = NULL;

	if (NULL == formatContext || NULL == formatContext->privateFormatData) {
		debug_error ("invalid param\n");
		return MMFILE_FORMAT_FAIL;
	}

	pFormatCtx = formatContext->privateFormatData;

	if (formatContext->formatType == MM_FILE_FORMAT_3GP ||formatContext->formatType == MM_FILE_FORMAT_MP4) {
		MMFileUtilGetMetaDataFromMP4 (formatContext);
	}

#ifdef __MMFILE_FFMPEG_V085__
/*metadata extracted by ffmpeg*/
	int idx = 0;

	if(pFormatCtx != NULL) {
		for(idx = 0; idx < pFormatCtx->nb_streams + 1; idx++) {
			AVDictionary *metainfo = NULL;
			AVStream *st = NULL;

			if(idx < pFormatCtx->nb_streams) {	//Check metadata of normal stream like audio, video, video cover art(cover art saved in new stream). refer to mov_read_covr() in ffmpeg.
				st = pFormatCtx->streams[idx];
					if(st != NULL)
						metainfo = st->metadata;
			} else {	//Check metadata of Content
				if(pFormatCtx->metadata != NULL) {
					metainfo = pFormatCtx->metadata;
				} else {
					continue;
				}
			}

			/*refer to mov_read_covr() in ffmpeg.*/
			if(st != NULL) {
				AVPacket pkt = st->attached_pic;
				int codec_id = st->codec->codec_id;

				if((pkt.data != NULL) && (pkt.size > 0)) {
					/*Set mime type*/
					if (formatContext->artworkMime)	mmfile_free (formatContext->artworkMime);

					if(codec_id == AV_CODEC_ID_MJPEG)
						formatContext->artworkMime = mmfile_strdup("image/jpeg");
					else if(codec_id == AV_CODEC_ID_PNG)
						formatContext->artworkMime = mmfile_strdup("image/png");
					else if(codec_id == AV_CODEC_ID_BMP)
						formatContext->artworkMime = mmfile_strdup("image/bmp");
					else
						debug_error ("Unknown cover type: 0x%x\n", codec_id);

					/*Copy artwork*/
					if (formatContext->artwork)	mmfile_free (formatContext->artwork);

					formatContext->artworkSize = pkt.size;
					formatContext->artwork = mmfile_malloc (pkt.size);
					memcpy (formatContext->artwork, pkt.data, pkt.size);
				}
			}

			if(metainfo != NULL) {
				AVDictionaryEntry *tag = NULL;
				while((tag=av_dict_get(metainfo, "", tag, AV_DICT_IGNORE_SUFFIX))) {
					if(tag->key != NULL) {
						if(!strcasecmp(tag->key, "title")) {
							if (formatContext->title)	free (formatContext->title);
							formatContext->title = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "artist")) {
							if (formatContext->artist)	free (formatContext->artist);
							formatContext->artist = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "composer")) {
							if (formatContext->composer)	free (formatContext->composer);
							formatContext->composer = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "album")) {
							if (formatContext->album)	free (formatContext->album);
							formatContext->album = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "album_artist")) {
							if (formatContext->album_artist)	free (formatContext->album_artist);
							formatContext->album_artist = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "copyright")) {
							if (formatContext->copyright)	free (formatContext->copyright);
							formatContext->copyright = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "comment")) {
							if (formatContext->comment)	free (formatContext->comment);
							formatContext->comment = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "description")) {
							if (formatContext->description)	free (formatContext->description);
							formatContext->description = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "genre")) {
							if (formatContext->genre)	free (formatContext->genre);
							formatContext->genre = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "date")) {
							if (formatContext->year)	free (formatContext->year);
							formatContext->year = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "creation_time")) {
							if (formatContext->recDate)	free (formatContext->recDate);
							formatContext->recDate = mmfile_strdup (tag->value);
						} else if((!strcasecmp(tag->key, "track")) || (!strcasecmp(tag->key, "tracknumber"))) {
							if (formatContext->tagTrackNum)	free (formatContext->tagTrackNum);
							formatContext->tagTrackNum = mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "lyrics")) {
							if (formatContext->unsyncLyrics)	free (formatContext->unsyncLyrics);
							formatContext->unsyncLyrics= mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "rotate")) {	//can be "90", "180", "270"
							if (formatContext->rotate)	free (formatContext->rotate);
							formatContext->rotate= mmfile_strdup (tag->value);
						} else if(!strcasecmp(tag->key, "metadata_block_picture")) {
							gsize len = 0;
							guchar *meta_data = NULL;

							meta_data = g_base64_decode(tag->value, &len);
							if (meta_data != NULL) {
								/* in METADATA_BLOCK_PICTURE,
								the length of mime type and  the length of description are flexible,
								so, we have to get the length of their for getting correct postion of picture data. */
								int mime_len = 0;
								int description_len = 0;
								int data_len = 0;
								int current_pos = 0;
								unsigned char current_data[DATA_LENGTH] = {0};

								/* get length of mime_type */
								memcpy(current_data, meta_data + POS_OF_MIME_LEN, DATA_LENGTH);
								CONVERT_TO_INT(mime_len, current_data);

								/* get length of description */
								current_pos =  mime_len + (DATA_LENGTH * 2); /*current position is length of description */
								memcpy(current_data, meta_data + current_pos, DATA_LENGTH);
								CONVERT_TO_INT(description_len, current_data);

								/* get length of picture data */
								current_pos = mime_len  + description_len + (DATA_LENGTH * 7); /*current position is length of picture data */
								memcpy(current_data, meta_data + current_pos, DATA_LENGTH);
								CONVERT_TO_INT(data_len, current_data);

								/* set the size of art work */
								formatContext->artworkSize = data_len;

								/* set mime type */
								current_pos = POS_OF_MIME_LEN + DATA_LENGTH; /*current position is mime type */
								if (formatContext->artworkMime) mmfile_free (formatContext->artworkMime);
								formatContext->artworkMime = strndup((const char*)meta_data + current_pos, mime_len);

								/* set art work data */
								current_pos = mime_len  + description_len + (DATA_LENGTH * 8); /*current position is picture data */
								if (formatContext->artwork) mmfile_free (formatContext->artwork);

								formatContext->artwork = mmfile_malloc (data_len);
								if(formatContext->artwork != NULL)
									memcpy(formatContext->artwork, meta_data + current_pos, data_len);

								g_free(meta_data);
							}
						} else {
							#ifdef __MMFILE_TEST_MODE__
							debug_log("Not support metadata. [%s:%s]", tag->key, tag->value);
							#endif
						}
					}
				}
			}
#ifdef 	__MMFILE_TEST_MODE__
			mmfile_format_print_tags (formatContext);
#endif
		}
	}
#else
	if (pFormatCtx->title[0])		{
		if (formatContext->title)
			free (formatContext->title);
		formatContext->title = mmfile_strdup (pFormatCtx->title);
	}
	if (pFormatCtx->author[0]){
		if (formatContext->author)
			free (formatContext->author);
		formatContext->author = mmfile_strdup (pFormatCtx->author);
	}
	if (pFormatCtx->copyright[0])	{
		if (formatContext->copyright)
			free (formatContext->copyright);
		formatContext->copyright = mmfile_strdup (pFormatCtx->copyright);
	}
	if (pFormatCtx->comment[0])		{
		if (formatContext->comment)
			free (formatContext->comment);
		formatContext->comment = mmfile_strdup (pFormatCtx->comment);
	}
	if (pFormatCtx->album[0])		{
		if (formatContext->album)
			free (formatContext->album);
		formatContext->album = mmfile_strdup (pFormatCtx->album);
	}
	if (pFormatCtx->genre[0])		{
		if (formatContext->genre)
			free (formatContext->genre);
		formatContext->genre = mmfile_strdup (pFormatCtx->genre);
	}

	if (pFormatCtx->year) {
		char year[10] = {0,};
		snprintf (year, 10, "%d", pFormatCtx->year);
		year[9] = '\0';
		if (formatContext->year)
			free (formatContext->year);
		formatContext->year = mmfile_strdup (year);
	}

	if (pFormatCtx->track) {
		char tracknum[10] = {0,};
		snprintf (tracknum, 10, "%d", pFormatCtx->track);
		tracknum[9] = '\0';
		if (formatContext->tagTrackNum)
			free (formatContext->tagTrackNum);
		formatContext->tagTrackNum = mmfile_strdup (tracknum);
	}
#endif

	return MMFILE_FORMAT_SUCCESS;
}


EXPORT_API
int mmfile_format_read_frame_ffmpg  (MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame)
{
	AVFormatContext	*pFormatCtx = NULL;
	AVCodecContext	*pVideoCodecCtx = NULL;
	AVCodec			*pVideoCodec = NULL;
	AVFrame			*pFrame = NULL;
	AVFrame			*pFrameRGB = NULL;

	int width;
	int height;
	int numBytes = 0;
	int ret = 0;

	if (NULL == formatContext ||
		NULL == frame ||
		NULL == formatContext->privateFormatData ||
		formatContext->videoTotalTrackNum <= 0) {

		debug_error ("invalid param\n");
		return MMFILE_FORMAT_FAIL;
	}

	if (formatContext->isdrm == MM_FILE_DRM_PROTECTED) {
		debug_error ("This is protected drm file\n");
		return MMFILE_FORMAT_FAIL;
	}

	pFormatCtx = formatContext->privateFormatData;

	if (formatContext->videoStreamId != -1) {
		pVideoCodecCtx = pFormatCtx->streams[formatContext->videoStreamId]->codec;
		if (NULL == pVideoCodecCtx) {
			debug_error ("invalid param\n");
			return MMFILE_FORMAT_FAIL;
		}

		pVideoCodec = avcodec_find_decoder (pVideoCodecCtx->codec_id);
		if ( NULL == pVideoCodec ) {
			debug_error ("error: avcodec_find_decoder failed\n");
			return MMFILE_FORMAT_FAIL;
		}

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("flag: 0x%08X\n", pVideoCodec->capabilities);
		// debug_msg ("  DRAW_HORIZ_BAND : %d\n", pVideoCodec->capabilities & CODEC_CAP_DRAW_HORIZ_BAND ? 1 : 0);
		// debug_msg ("  DR1             : %d\n", pVideoCodec->capabilities & CODEC_CAP_DR1 ? 1 : 0);
		// debug_msg ("  PARSE_ONLY      : %d\n", pVideoCodec->capabilities & CODEC_CAP_PARSE_ONLY ? 1 : 0);
		// debug_msg ("  TRUNCATED       : %d\n", pVideoCodec->capabilities & CODEC_CAP_TRUNCATED ? 1 : 0);
		// debug_msg ("  HWACCEL         : %d\n", pVideoCodec->capabilities & CODEC_CAP_HWACCEL ? 1 : 0);
		// debug_msg ("  DELAY           : %d\n", pVideoCodec->capabilities & CODEC_CAP_DELAY ? 1 : 0);
		// debug_msg ("  SMALL_LAST_FRAME: %d\n", pVideoCodec->capabilities & CODEC_CAP_SMALL_LAST_FRAME ? 1 : 0);
		// debug_msg ("  HWACCEL_VDPAU   : %d\n", pVideoCodec->capabilities & CODEC_CAP_HWACCEL_VDPAU ? 1 : 0);
		#endif

		if (pVideoCodec->capabilities & CODEC_CAP_TRUNCATED) {
			pVideoCodecCtx->flags |= CODEC_FLAG_TRUNCATED;
		}

		/*set workaround bug flag*/
		pVideoCodecCtx->workaround_bugs = FF_BUG_AUTODETECT;
#ifdef __MMFILE_FFMPEG_V100__
		/* this is solution for PLM issue P13091703323 */
		/* If using thread when decoding frame, the result of decoding is not always same. 
		    Thumbnail of video content is different with original file when copying file. */
		pVideoCodecCtx->thread_type = 0;
		pVideoCodecCtx->thread_count = 0;
		ret = avcodec_open2 (pVideoCodecCtx, pVideoCodec, NULL);
#else
		ret = avcodec_open (pVideoCodecCtx, pVideoCodec);
#endif
		if (ret < 0) {
			debug_error ("error: avcodec_open fail.\n");
			return MMFILE_FORMAT_FAIL;
		}

		pFrameRGB = avcodec_alloc_frame ();

		if (!pFrameRGB) {
			debug_error ("error: pFrame or pFrameRGB is NULL\n");
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}

		/* search & decode */
		// seek_ts = formatContext->duration > _SHORT_MEDIA_LIMIT ? seek_ts : 0;	/*if short media, seek first key frame*/
		ret = _get_first_good_video_frame (pFormatCtx, pVideoCodecCtx, formatContext->videoStreamId, &pFrame, formatContext->cdis);
		if ( ret != MMFILE_FORMAT_SUCCESS ) {
			debug_error ("error: get key frame\n");
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("Video default resolution = [%dx%d]\n", pVideoCodecCtx->coded_width, pVideoCodecCtx->coded_height);
		debug_msg ("Video coded resolution = [%dx%d]\n", pVideoCodecCtx->width, pVideoCodecCtx->height);
		#endif

		/*sometimes, ffmpeg's width/height is wrong*/
		#if 0	/*coded_width/height sometimes wrong. so use width/height*/
		width = pVideoCodecCtx->coded_width == 0 ? pVideoCodecCtx->width : pVideoCodecCtx->coded_width;
		height = pVideoCodecCtx->coded_height == 0 ? pVideoCodecCtx->height : pVideoCodecCtx->coded_height;
		#endif
		if((pVideoCodecCtx->width == 0) || (pVideoCodecCtx->height == 0)) {
			width = pVideoCodecCtx->coded_width;
			height = pVideoCodecCtx->coded_height;
		} else {
			width = pVideoCodecCtx->width;
			height = pVideoCodecCtx->height;
		}

		numBytes = avpicture_get_size(PIX_FMT_RGB24, width, height);
		if (numBytes < 0) {
			debug_error ("error: avpicture_get_size. [%d x %d]\n", width, height);
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}

		frame->frameData = mmfile_malloc (numBytes);
		if (NULL == frame->frameData) {
			debug_error ("error: avpicture_get_size. [%d]\n", numBytes);
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}

		ret = avpicture_fill ((AVPicture *)pFrameRGB, frame->frameData, PIX_FMT_RGB24, width, height);
		if (ret < 0) {
			debug_error ("error: avpicture_fill fail. errcode = 0x%08X\n", ret);
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}

#ifdef __MMFILE_FFMPEG_V085__
		struct SwsContext *img_convert_ctx = NULL;

		img_convert_ctx = sws_getContext (width, height, pVideoCodecCtx->pix_fmt,
		                          width, height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

		if (NULL == img_convert_ctx) {
			debug_error ("failed to get img convet ctx\n");
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}

		ret = sws_scale (img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize,
		     0, height, pFrameRGB->data, pFrameRGB->linesize);
		if ( ret < 0 ) {
			debug_error ("failed to convet image\n");
			ret = MMFILE_FORMAT_FAIL;
			sws_freeContext(img_convert_ctx);
			img_convert_ctx = NULL;
			goto exception;
		}

		sws_freeContext(img_convert_ctx);
		img_convert_ctx = NULL;
#else
		ret = img_convert ((AVPicture *)pFrameRGB, PIX_FMT_RGB24, (AVPicture*)pFrame, pVideoCodecCtx->pix_fmt, width, height);
		if ( ret < 0 ) {
			debug_error ("failed to convet image\n");
			ret = MMFILE_FORMAT_FAIL;
			goto exception;
		}
#endif
		frame->frameSize = numBytes;
		frame->frameWidth = width;
		frame->frameHeight = height;
		frame->configLenth = 0;
		frame->bCompressed = 0; /* false */

		if (pFrame)			av_free (pFrame);
		if (pFrameRGB)		av_free (pFrameRGB);

		avcodec_close(pVideoCodecCtx);

		return MMFILE_FORMAT_SUCCESS;
	}


exception:
	if (pVideoCodecCtx)		avcodec_close (pVideoCodecCtx);
	if (frame->frameData)	{ mmfile_free (frame->frameData); frame->frameData = NULL; }
	if (pFrame)				av_free (pFrame);
	if (pFrameRGB)			av_free (pFrameRGB);
	return ret;
}


EXPORT_API
int mmfile_format_close_ffmpg(MMFileFormatContext *formatContext)
{
	if (formatContext) {
		AVFormatContext *pFormatCtx = formatContext->privateFormatData;

		if (pFormatCtx) {
#ifdef __MMFILE_FFMPEG_V100__
			avformat_close_input(&pFormatCtx);
#else
			av_close_input_file (pFormatCtx);
#endif
			formatContext->privateFormatData = NULL;
		}
	}

	return MMFILE_FORMAT_SUCCESS;
}

/**
 * return average of difference
 */
static unsigned int _diff_memory (const void *s1, const void *s2, unsigned int n)
{
	char *s = (char *)s1;
	char *d = (char *)s2;
	int i;
	int ret;
	int tmp;

	for (i = 0, ret = 0; i < n; i++) {
		if (*s++ != *d++) {
			tmp = (*s - *d);
			ret += (tmp < 0 ? -tmp : tmp);
		}
	}
	ret /= n;
	return ret;
}

int64_t gettime(void)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}


#define IS_GOOD_OLD_METHOD
#ifdef IS_GOOD_OLD_METHOD
/**
 * compare with center line.
 */
static int _is_good_pgm (unsigned char *buf, int wrap, int xsize, int ysize)
{
#define _MM_CHUNK_NUM			8						/*FIXME*/
#define _MM_CHUNK_LIMIT			(_MM_CHUNK_NUM / 2)
#define _MM_CHUNK_DIFF_LIMIT	(_MM_CHUNK_LIMIT * 2 + 1)	/*FIXME*/

	int i;
	int step;
	int point;
	unsigned char *cnt;		/*center line of image*/
	int is_different;
	unsigned int sum_diff;
	int cnt_offset;

	/*set center line*/
	step = ysize / _MM_CHUNK_NUM;
	cnt_offset = (ysize / 2);
	cnt = buf + cnt_offset * wrap;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("checking frame. %p, %d, %d, %d\n", buf, wrap, xsize, ysize);
	#endif

	/*if too small, always ok return.*/
	if (ysize < _MM_CHUNK_NUM)
		return 1;

	for (point = 0, sum_diff = 0, i = step; i < ysize; i += step) {
		if (i != cnt_offset) {
		
			/*binary compare*/
			is_different = _diff_memory (cnt, buf + i * wrap, xsize);
			point += (is_different == 0 ? 0 : 1);
			sum_diff += is_different;

			#ifdef __MMFILE_TEST_MODE__
			debug_msg ("check %04d line. %s [%d]\n", i, (is_different == 0 ? "same" : "different"), is_different);
			#endif

			if (point >= _MM_CHUNK_LIMIT) {
				if (sum_diff > _MM_CHUNK_DIFF_LIMIT) {
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ("Good :-)\n");
					#endif
					return 1;
				} else {
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ("Bad :-(\n");
					#endif
					return 0;
				}
			}
		}

	}
	return 0;
}
#else // IS_GOOD_OLD_METHOD 
/* ToDo : for enhancement */
#endif // IS_GOOD_OLD_METHOD



static int
_get_video_fps (int frame_cnt, int duration, AVRational r_frame_rate, int is_roundup)
{
	double fps, round;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("frame count: %d, dur: %d, num: %d, den: %d\n", frame_cnt, duration, r_frame_rate.num, r_frame_rate.den)
	#endif

	if (duration <= 0 || r_frame_rate.num <= 0 || r_frame_rate.den <= 0)
		return 0;

	round = (is_roundup != 0 ? 0.50f : 0.00f);

	fps = (double)frame_cnt / ((double)(duration * r_frame_rate.num) / r_frame_rate.den) + round;

	return (int)fps;
}

#ifdef MMFILE_FORMAT_DEBUG_DUMP
static void _save_pgm (unsigned char *buf,int wrap, int xsize,int ysize,char *filename)
{
	FILE *f;
	int i;

	f = fopen(filename,"w");
	if (f) {
		fprintf (f,"P5\n%d %d\n%d\n",xsize,ysize,255);
		for (i = 0; i < ysize; i++)
			fwrite (buf + i * wrap, 1, xsize, f);
		fclose (f);
	}
}
#endif

#ifdef __MMFILE_TEST_MODE__
static void _dump_av_packet (AVPacket *pkt)
{
	debug_msg ("--------- AV Packet -----------\n");
	debug_msg (" pts: %lld\n", pkt->pts);
	debug_msg (" dts: %lld\n", pkt->dts);
	debug_msg (" data: %p\n", pkt->data);
	debug_msg (" size: %d\n", pkt->size);
	debug_msg (" stream_index: %d\n", pkt->stream_index);
#ifdef __MMFILE_FFMPEG_V085__	
	debug_msg (" flags: 0x%08X, %s\n", pkt->flags, (pkt->flags & AV_PKT_FLAG_KEY) ? "Keyframe" : "_");
#else
	debug_msg (" flags: 0x%08X, %s\n", pkt->flags, (pkt->flags & PKT_FLAG_KEY) ? "Keyframe" : "_");
#endif
	debug_msg (" duration: %d\n", pkt->duration);
	debug_msg (" destruct: %p\n", pkt->destruct);
	debug_msg (" priv: %p\n", pkt->priv);
	debug_msg (" pos: %lld\n", pkt->pos);
	debug_msg (" convergence_duration: %lld\n", pkt->convergence_duration);
	debug_msg ("-------------------------------\n");
}
#endif

static int _get_first_good_video_frame (AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, int videoStream, AVFrame **pFrame, int cdis)
{
	// AVStream *st = NULL;
	AVPacket pkt;

	AVFrame *frame = NULL;
	AVFrame *tmp_frame = NULL;
	AVFrame *first_frame = NULL;

	// long long timestamp;
	int stream_id = videoStream;
	int ret;
	int found = 0;
	int i,v, len, got_picture;
	int retry = 0;
	int key_detected;
#ifdef MMFILE_FORMAT_DEBUG_DUMP
	char pgm_name[256] = {0,};
#endif
	int key_search_limit = 0;
	int frame_search_limit = 0;

#define	_RETRY_SEARCH_LIMIT		75
#define	_KEY_SEARCH_LIMIT		(_RETRY_SEARCH_LIMIT*2)		/*2 = 1 read. some frame need to read one more*/
#define	_FRAME_SEARCH_LIMIT		500

#define	_RETRY_SEARCH_LIMIT_CDIS		10
#define	_KEY_SEARCH_LIMIT_CDIS		(_RETRY_SEARCH_LIMIT*2)		/*2 = 1 read. some frame need to read one more*/
#define	_FRAME_SEARCH_LIMIT_CDIS		10

	first_frame = avcodec_alloc_frame ();
	tmp_frame = avcodec_alloc_frame ();

	if (!first_frame || !tmp_frame) {
		debug_error ("failed to alloc frame.\n");
		if (first_frame) av_free (first_frame);
		if (tmp_frame) av_free (tmp_frame);
		return MMFILE_FORMAT_FAIL;
	}

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("frame: 1. %p, 2. %p\n", first_frame, tmp_frame);
	#endif

#ifdef __MMFILE_FFMPEG_V085__
	pCodecCtx->skip_frame = AVDISCARD_BIDIR;
#else
	pCodecCtx->hurry_up = 1;
#endif

	if (cdis == 1) {
		key_search_limit = _KEY_SEARCH_LIMIT_CDIS;
		frame_search_limit = _FRAME_SEARCH_LIMIT_CDIS;
	} else {
		key_search_limit = _KEY_SEARCH_LIMIT;
		frame_search_limit = _FRAME_SEARCH_LIMIT;
	}

	for(i = 0, v = 0, key_detected = 0, frame = first_frame; i < key_search_limit && v < frame_search_limit;) {
		av_init_packet (&pkt);
		got_picture = 0;

		ret = av_read_frame (pFormatCtx, &pkt);
		if (ret < 0) {
			debug_error ("read failed. (maybe EOF or broken)\n");
			break;
		} else {
			if (pkt.stream_index == stream_id) {
				v++;
#ifdef __MMFILE_FFMPEG_V085__				
				if ((pkt.flags & AV_PKT_FLAG_KEY ) || (key_detected == 1)) 
#else
				if ((pkt.flags & PKT_FLAG_KEY ) || (key_detected == 1)) 
#endif
				{
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ("video frame: %d, %d, %d\n", retry, i, v);
					_dump_av_packet (&pkt);
					#endif

					i++;
					key_detected = 0;
#ifdef __MMFILE_FFMPEG_V085__
					len = avcodec_decode_video2 (pCodecCtx, frame, &got_picture, &pkt);
#else
					len = avcodec_decode_video (pCodecCtx, frame, &got_picture, pkt.data, pkt.size);
#endif
					if (len < 0) {
						debug_warning ("Error while decoding frame %dth\n", i);
					} else if (got_picture) {
						if (frame->key_frame) {
							#ifdef __MMFILE_TEST_MODE__
							debug_msg ("key frame!\n");
							#endif
							#ifdef MMFILE_FORMAT_DEBUG_DUMP
							sprintf (pgm_name, "./key_%d_%d_%d.pgm", retry, i, v);
							_save_pgm (frame->data[0], frame->linesize[0], pCodecCtx->width, pCodecCtx->height, pgm_name);
							#endif

							found++;

							#ifdef __MMFILE_TEST_MODE__
							int64_t ti;
							ti = gettime();
							#endif
							ret = _is_good_pgm (frame->data[0], frame->linesize[0], pCodecCtx->width, pCodecCtx->height);
							#ifdef __MMFILE_TEST_MODE__
							ti = gettime() - ti;
							debug_msg ("Elapsed time = %lld\n", ti);
							#endif
							if (ret != 0) {
								#ifdef __MMFILE_TEST_MODE__
								debug_msg ("is good frame.\n");
								#endif
								break;
							} else {
								/*reset video frame count & retry searching*/
								#ifdef __MMFILE_TEST_MODE__
								debug_warning ("not good fame. retry scanning.\n");
								#endif
								i = 0;
								v = 0;
								retry++;
							}

							/*set buffer frame*/
							frame = tmp_frame;

							/*limit of retry.*/
							if (retry > _RETRY_SEARCH_LIMIT)	break;

						} else {
							#ifdef __MMFILE_TEST_MODE__
							debug_msg ("skip (not key frame).\n");
							#endif
							#ifdef MMFILE_FORMAT_DEBUG_DUMP
							sprintf (pgm_name, "./not_key_%d_%d_%d.pgm", retry, i, v);
							_save_pgm (frame->data[0], frame->linesize[0], pCodecCtx->width, pCodecCtx->height, pgm_name);
							#endif
						}
					} else {
						#ifdef __MMFILE_TEST_MODE__
						debug_msg ("decode not completed.\n");
						#endif
						key_detected = 1;
					}
				}
			}
		}
		av_free_packet (&pkt);
	}

	/*free pkt after loop breaking*/
	if (pkt.data) av_free_packet (&pkt);

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("found: %d, retry: %d\n", found, retry);
	#endif

	/*set decode frame to output*/
	if (found > 0) {
		ret = MMFILE_FORMAT_SUCCESS;
		if (retry == 0 || found == retry) {
			*pFrame = first_frame;
			if (tmp_frame) av_free (tmp_frame);
		} else {
			*pFrame = tmp_frame;
			if (first_frame) av_free (first_frame);
		}
	} else {
		ret = MMFILE_FORMAT_FAIL;
		if (first_frame) av_free (first_frame);
		if (tmp_frame) av_free (tmp_frame);
	}

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("out frame: %p\n", *pFrame);
	#endif

#ifdef __MMFILE_FFMPEG_V085__
	pCodecCtx->skip_frame = AVDISCARD_NONE;
#else
	pCodecCtx->hurry_up = 0;
#endif

	return ret;
}

static int ConvertVideoCodecEnum (int AVVideoCodecID)
{
	int ret_codecid = 0;

	switch (AVVideoCodecID)
	{
		case AV_CODEC_ID_NONE:
			ret_codecid = MM_VIDEO_CODEC_NONE;
			break;
		case AV_CODEC_ID_MPEG1VIDEO:
			ret_codecid = MM_VIDEO_CODEC_MPEG1;
			break;
		case AV_CODEC_ID_MPEG2VIDEO:  ///< preferred ID for MPEG-1/2 video decoding
			ret_codecid = MM_VIDEO_CODEC_MPEG2;
			break;
		case AV_CODEC_ID_MPEG2VIDEO_XVMC:
			ret_codecid = MM_VIDEO_CODEC_MPEG2;
			break;
		case AV_CODEC_ID_H261:
			ret_codecid = MM_VIDEO_CODEC_H261;
			break;
		case AV_CODEC_ID_H263:
			ret_codecid = MM_VIDEO_CODEC_H263;
			break;
		case AV_CODEC_ID_MPEG4:
			ret_codecid = MM_VIDEO_CODEC_MPEG4;
			break;
		case AV_CODEC_ID_MSMPEG4V1:
			ret_codecid = MM_VIDEO_CODEC_MPEG4;
			break;
		case AV_CODEC_ID_MSMPEG4V2:
			ret_codecid = MM_VIDEO_CODEC_MPEG4;
			break;
		case AV_CODEC_ID_MSMPEG4V3:
			ret_codecid = MM_VIDEO_CODEC_MPEG4;
			break;
		case AV_CODEC_ID_WMV1:
			ret_codecid = MM_VIDEO_CODEC_WMV;
			break;
		case AV_CODEC_ID_WMV2:
			ret_codecid = MM_VIDEO_CODEC_WMV;
			break;
		case AV_CODEC_ID_H263P:
			ret_codecid = MM_VIDEO_CODEC_H263;
			break;
		case AV_CODEC_ID_H263I:
			ret_codecid = MM_VIDEO_CODEC_H263;
			break;
		case AV_CODEC_ID_FLV1:
			ret_codecid = MM_VIDEO_CODEC_FLV;
			break;
		case AV_CODEC_ID_H264:
			ret_codecid = MM_VIDEO_CODEC_H264;
			break;
		case AV_CODEC_ID_INDEO2:
		case AV_CODEC_ID_INDEO3:
		case AV_CODEC_ID_INDEO4:
		case AV_CODEC_ID_INDEO5:
			ret_codecid = MM_VIDEO_CODEC_INDEO;
			break;
		case AV_CODEC_ID_THEORA:
			ret_codecid = MM_VIDEO_CODEC_THEORA;
			break;
		case AV_CODEC_ID_CINEPAK:
			ret_codecid = MM_VIDEO_CODEC_CINEPAK;
			break;
#ifndef __MMFILE_FFMPEG_V085__
		case CODEC_ID_XVID:
			ret_codecid = MM_VIDEO_CODEC_XVID;
			break;
#endif
		case AV_CODEC_ID_VC1:
			ret_codecid = MM_VIDEO_CODEC_VC1;
			break;
		case AV_CODEC_ID_WMV3:
			ret_codecid = MM_VIDEO_CODEC_WMV;
			break;
		case AV_CODEC_ID_AVS:
			ret_codecid = MM_VIDEO_CODEC_AVS;
			break;
		case AV_CODEC_ID_RL2:
			ret_codecid = MM_VIDEO_CODEC_REAL;
			break;
		default:
			ret_codecid = MM_VIDEO_CODEC_NONE;
			break;
	}

	return ret_codecid;
}


static int ConvertAudioCodecEnum (int AVAudioCodecID)
{
	int ret_codecid = 0;

	switch (AVAudioCodecID)
	{
		case AV_CODEC_ID_AMR_NB:
		case AV_CODEC_ID_AMR_WB:
			ret_codecid = MM_AUDIO_CODEC_AMR;
			break;
		/* RealAudio codecs*/
		case AV_CODEC_ID_RA_144:
		case AV_CODEC_ID_RA_288:
			ret_codecid = MM_AUDIO_CODEC_REAL;
			break;
		case AV_CODEC_ID_MP2:
			ret_codecid = MM_AUDIO_CODEC_MP2;
			break;
		case AV_CODEC_ID_MP3:
		case AV_CODEC_ID_MP3ADU:
		case AV_CODEC_ID_MP3ON4:
			ret_codecid = MM_AUDIO_CODEC_MP3;
			break;
		case AV_CODEC_ID_AAC:
			ret_codecid = MM_AUDIO_CODEC_AAC;
			break;
		case AV_CODEC_ID_AC3:
			ret_codecid = MM_AUDIO_CODEC_AC3;
			break;
		case AV_CODEC_ID_VORBIS:
			ret_codecid = MM_AUDIO_CODEC_VORBIS;
			break;
		case AV_CODEC_ID_WMAV1:
		case AV_CODEC_ID_WMAV2:
		case AV_CODEC_ID_WMAVOICE:
		case AV_CODEC_ID_WMAPRO:
		case AV_CODEC_ID_WMALOSSLESS:
			ret_codecid = MM_AUDIO_CODEC_WMA;
			break;
		case AV_CODEC_ID_FLAC:
			ret_codecid = MM_AUDIO_CODEC_FLAC;
			break;
		case AV_CODEC_ID_ALAC:
			ret_codecid = MM_AUDIO_CODEC_ALAC;
			break;
		case AV_CODEC_ID_WAVPACK:
			ret_codecid = MM_AUDIO_CODEC_WAVE;
			break;
		case AV_CODEC_ID_ATRAC3:
		case AV_CODEC_ID_ATRAC3P:
		case AV_CODEC_ID_EAC3:
			ret_codecid = MM_AUDIO_CODEC_AC3;
			break;
		default:
			ret_codecid = MM_AUDIO_CODEC_NONE;
			break;
	}

	return ret_codecid;
}



static int getMimeType(int formatId, char *mimeType)
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


