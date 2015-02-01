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
#include <stdlib.h>
#include <unistd.h>	/*for access*/
#include <string.h>	/*for strXXX*/
#include <dlfcn.h>

/* exported MM header files */
#include <mm_types.h>
#include <mm_error.h>
#include <mm_file.h>

/* internal MM header files */
#include <mm_attrs_private.h>
#include <mm_debug.h>

/* internal MM File headers */
#include "mm_file_formats.h"
#include "mm_file_format_frame.h"
#include "mm_file_codecs.h"
#include "mm_file_utils.h"


#include <sys/time.h>

//#define CHECK_TIME

#ifdef CHECK_TIME
int64_t gettime(void)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}
#endif


/**
 * Defines.
 */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof((arr)[0]))
#endif

#define _SEEK_POINT_	3000		/*1000 = 1 seconds*/

#define	MM_FILE_TAG_SYNCLYRICS         	"tag-synclyrics"  		/**< Synchronized Lyrics Information*/


enum {
	MM_FILE_TAG,
	MM_FILE_CONTENTS,
	MM_FILE_INVALID,
};

enum {
	MM_FILE_PARSE_TYPE_SIMPLE,		/*parse audio/video track num only*/
	MM_FILE_PARSE_TYPE_NORMAL,		/*parse infomation without thumbnail*/
	MM_FILE_PARSE_TYPE_ALL,			/*parse all infomation*/
	MM_FILE_PARSE_TYPE_SAFE,		/*parse infomation without both thumbnail and stream full-searching*/
};

typedef struct {
	int	type;
	int	audio_track_num;
	int	video_track_num;
	bool is_uhqa;
} MMFILE_PARSE_INFO;

typedef struct {
	void *formatFuncHandle;
	void *codecFuncHandle;
} MMFILE_FUNC_HANDLE;



/**
 * global values.
 */
static mmf_attrs_construct_info_t g_tag_attrs[] = {
	{"tag-artist",			MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-title",			MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-album",			MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-album-artist",	MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-genre",			MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-author",			MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-copyright",		MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-date",			MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-description",		MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-comment",		MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-artwork",		MMF_VALUE_TYPE_DATA,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-artwork-size",	MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"tag-artwork-mime",	MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-track-num",		MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-classification",	MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-rating",			MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-longitude",		MM_ATTRS_TYPE_DOUBLE,	MM_ATTRS_FLAG_RW, (void *)0},
	{"tag-latitude",		MM_ATTRS_TYPE_DOUBLE,	MM_ATTRS_FLAG_RW, (void *)0},
	{"tag-altitude",		MM_ATTRS_TYPE_DOUBLE,	MM_ATTRS_FLAG_RW, (void *)0},
	{"tag-conductor",		MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-unsynclyrics",	MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-synclyrics-num",	MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"tag-synclyrics",		MMF_VALUE_TYPE_DATA,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-recdate",		MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-rotate",			MMF_VALUE_TYPE_STRING,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"tag-cdis",			MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"tag-smta",			MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
};

static mmf_attrs_construct_info_t g_content_attrs[] = {
	{"content-duration",			MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-video-codec",		MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-video-bitrate",		MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-video-fps",			MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-video-width",		MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-video-height",		MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-video-thumbnail",		MMF_VALUE_TYPE_DATA,	MM_ATTRS_FLAG_RW, (void *)NULL},
	{"content-video-track-index",	MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-video-track-count",	MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-audio-codec",		MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-audio-bitrate",		MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-audio-channels",		MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-audio-samplerate",	MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-audio-track-index",	MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-audio-track-count",	MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
	{"content-audio-bitpersample",	MMF_VALUE_TYPE_INT,		MM_ATTRS_FLAG_RW, (void *)0},
};

#ifdef __MMFILE_DYN_LOADING__
#define MMFILE_FORMAT_SO_FILE_NAME  "libmmfile_formats.so"
#define MMFILE_CODEC_SO_FILE_NAME   "libmmfile_codecs.so"

int (*mmfile_format_open)			(MMFileFormatContext **formatContext, MMFileSourceType *fileSrc);
int (*mmfile_format_read_stream)	(MMFileFormatContext *formatContext);
int (*mmfile_format_read_frame)		(MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int (*mmfile_format_read_tag)		(MMFileFormatContext *formatContext);
int (*mmfile_format_close)			(MMFileFormatContext *formatContext);
int (*mmfile_codec_open)				(MMFileCodecContext **codecContext, int codecType, int codecId, MMFileCodecFrame *input);
int (*mmfile_codec_decode)			(MMFileCodecContext *codecContext, MMFileCodecFrame *output);
int (*mmfile_codec_close)			(MMFileCodecContext *codecContext);
int (*mmfile_format_get_frame)		(const char* path, double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height);
int (*mmfile_format_get_frame_from_memory)		(const void *data, unsigned int datasize, double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height);
#endif

#ifdef __MMFILE_DYN_LOADING__
static int _load_dynamic_functions (MMFILE_FUNC_HANDLE* pHandle)
{
//	static int dll_func_initialized = 0; //disabled

	int ret = 0;

	/* Get from function argument */
	void *formatFuncHandle = NULL;
	void *codecFuncHandle = NULL;	

	/* disabled
	if (dll_func_initialized) {
		return 1;
	}
	*/

	formatFuncHandle = dlopen (MMFILE_FORMAT_SO_FILE_NAME, RTLD_LAZY);
	if (!formatFuncHandle) {
		debug_error ("error: %s\n", dlerror());
		ret = 0;
		goto exception;
	}

	mmfile_format_open			= dlsym (formatFuncHandle, "mmfile_format_open");
	mmfile_format_read_stream	= dlsym (formatFuncHandle, "mmfile_format_read_stream");
	mmfile_format_read_frame	= dlsym (formatFuncHandle, "mmfile_format_read_frame");
	mmfile_format_read_tag		= dlsym (formatFuncHandle, "mmfile_format_read_tag");
	mmfile_format_close			= dlsym (formatFuncHandle, "mmfile_format_close");

	if ( !mmfile_format_open ||
		 !mmfile_format_read_stream ||
		 !mmfile_format_read_frame ||
		 !mmfile_format_read_tag ||
		 !mmfile_format_close) {

		debug_error ("error: %s\n", dlerror());
		ret = 0;
		goto exception;
	}

	/*closed at app termination.*/
	//dlclose (formatFuncHandle);

	codecFuncHandle = dlopen (MMFILE_CODEC_SO_FILE_NAME, RTLD_LAZY | RTLD_GLOBAL);
	if (!codecFuncHandle) {
		debug_error ("error: %s\n", dlerror());
		ret = 0;
		goto exception;
	}

	mmfile_codec_open		= dlsym (codecFuncHandle, "mmfile_codec_open");
	mmfile_codec_decode	= dlsym (codecFuncHandle, "mmfile_codec_decode");
	mmfile_codec_close		= dlsym (codecFuncHandle, "mmfile_codec_close");

	if ( !mmfile_codec_open || !mmfile_codec_decode || !mmfile_codec_close) {
		debug_error ("error: %s\n", dlerror());
		ret = 0;
		goto exception;
	}

	/*closed at app termination.*/
	//dlclose (codecFuncHandle);

//	dll_func_initialized = 1; // disabled

	pHandle->codecFuncHandle = codecFuncHandle;
	pHandle->formatFuncHandle = formatFuncHandle;

	return 1;

exception:
	if (formatFuncHandle) dlclose (formatFuncHandle);
	if (codecFuncHandle)  dlclose (codecFuncHandle);

	return ret;
}

static void _unload_dynamic_functions (MMFILE_FUNC_HANDLE* pHandle)
{
#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

	if (pHandle->formatFuncHandle) 
	{
		dlclose (pHandle->formatFuncHandle);		
	}
	if (pHandle->codecFuncHandle)  
	{
		dlclose (pHandle->codecFuncHandle);		
	}
	
#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif
}


#endif /* __MMFILE_DYN_LOADING__ */

/**
 * local functions.
 */
static int
_is_file_exist (const char *filename)
{
	int ret = 1;
	if (filename) {
		const char* to_access = (strstr(filename,"file://")!=NULL)? filename+7:filename;
		ret = access (to_access, R_OK );
		if (ret != 0) {
			debug_error  ("file [%s] not found.\n", to_access);
		}
	}
	return !ret;
}

static int
_info_set_attr_media (mmf_attrs_t *attrs, MMFileFormatContext *formatContext)
{
	int ret = 0;
	MMHandleType hattrs = CAST_MM_HANDLE(attrs);

	if (formatContext->commandType == MM_FILE_TAG) 
	{
		if (formatContext->title)				mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_TITLE, formatContext->title);
		if (formatContext->artist)				mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_ARTIST, formatContext->artist);
		if (formatContext->author)			mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_AUTHOR, formatContext->author);
		if (formatContext->composer && formatContext->author == NULL)	
											mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_AUTHOR, formatContext->composer);
		if (formatContext->album)				mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_ALBUM, formatContext->album);
		if (formatContext->album_artist)				mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_ALBUM_ARTIST, formatContext->album_artist);
		if (formatContext->copyright)			mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_COPYRIGHT, formatContext->copyright);
		if (formatContext->description)			mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_DESCRIPTION, formatContext->description);
		if (formatContext->comment)			mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_COMMENT, formatContext->comment);
		if (formatContext->genre)				mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_GENRE, formatContext->genre);
		if (formatContext->classification)  		mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_CLASSIFICATION, formatContext->classification);
		if (formatContext->year)				mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_DATE, formatContext->year); 
		if (formatContext->tagTrackNum)		mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_TRACK_NUM, formatContext->tagTrackNum);	
		if (formatContext->rating)				mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_RATING, formatContext->rating);
		if (formatContext->conductor)       	mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_CONDUCTOR, formatContext->conductor);
		if (formatContext->recDate)       		mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_RECDATE, formatContext->recDate);
		if (formatContext->rotate)       		mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_ROTATE, formatContext->rotate);
											mm_attrs_set_double_by_name(hattrs, MM_FILE_TAG_LONGITUDE, formatContext->longitude);
											mm_attrs_set_double_by_name(hattrs, MM_FILE_TAG_LATIDUE, formatContext->latitude);
											mm_attrs_set_double_by_name(hattrs, MM_FILE_TAG_ALTIDUE, formatContext->altitude); 
											mm_attrs_set_int_by_name(hattrs, MM_FILE_TAG_SYNCLYRICS_NUM, formatContext->syncLyricsNum);
											mm_attrs_set_int_by_name(hattrs, MM_FILE_TAG_CDIS, formatContext->cdis);
											mm_attrs_set_int_by_name(hattrs, MM_FILE_TAG_SMTA, formatContext->smta);

		if ((formatContext->syncLyricsNum > 0) && (formatContext->syncLyrics))
			mm_attrs_set_data_by_name (hattrs, MM_FILE_TAG_SYNCLYRICS, formatContext->syncLyrics, formatContext->syncLyricsNum);

		if (formatContext->unsyncLyrics)		mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_UNSYNCLYRICS, formatContext->unsyncLyrics);
		
		if (formatContext->artwork && formatContext->artworkSize > 0) {
			void *artworkCopy = NULL;
			artworkCopy = mmfile_malloc ((formatContext->artworkSize));
			if ( NULL != artworkCopy ) {
				memcpy (artworkCopy, formatContext->artwork, formatContext->artworkSize);
				mm_attrs_set_data_by_name (hattrs, MM_FILE_TAG_ARTWORK,artworkCopy, formatContext->artworkSize);
				mm_attrs_set_int_by_name (hattrs, MM_FILE_TAG_ARTWORK_SIZE, formatContext->artworkSize);
				if (formatContext->artworkMime)	mm_attrs_set_string_by_name(hattrs, MM_FILE_TAG_ARTWORK_MIME, formatContext->artworkMime);
			}
		}
	} 
	else if (formatContext->commandType == MM_FILE_CONTENTS) 
	{
		/*get duration*/
		mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_DURATION, formatContext->duration);
		mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_AUDIO_TRACK_COUNT, formatContext->audioTotalTrackNum);
		mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_VIDEO_TRACK_COUNT, formatContext->videoTotalTrackNum);

		if (formatContext->videoTotalTrackNum > 0 &&
			formatContext->nbStreams > 0 &&
			formatContext->streams[MMFILE_VIDEO_STREAM]) {

			MMFileFormatStream *videoStream = formatContext->streams[MMFILE_VIDEO_STREAM];

			mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_VIDEO_CODEC, videoStream->codecId);
			mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_VIDEO_BITRATE, videoStream->bitRate);
			mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_VIDEO_FPS, videoStream->framePerSec);			
			mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_VIDEO_WIDTH, videoStream->width);
			mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_VIDEO_HEIGHT, videoStream->height);

			if (formatContext->thumbNail && formatContext->thumbNail->frameData) {
				void *thumbNailCopy = NULL;
				thumbNailCopy = mmfile_malloc (formatContext->thumbNail->frameSize);

				if (NULL != thumbNailCopy) {
					memcpy (thumbNailCopy, formatContext->thumbNail->frameData, formatContext->thumbNail->frameSize);
					mm_attrs_set_data_by_name (hattrs, MM_FILE_CONTENT_VIDEO_THUMBNAIL, thumbNailCopy, formatContext->thumbNail->frameSize);
					mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_VIDEO_WIDTH, formatContext->thumbNail->frameWidth);
					mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_VIDEO_HEIGHT, formatContext->thumbNail->frameHeight);
				}
			}
		}

		if (formatContext->audioTotalTrackNum > 0 &&
			formatContext->nbStreams > 0 &&
			formatContext->streams[MMFILE_AUDIO_STREAM]) {

			MMFileFormatStream *audioStream = formatContext->streams[MMFILE_AUDIO_STREAM];

			mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_AUDIO_CODEC, audioStream->codecId);
			mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_AUDIO_CHANNELS, audioStream->nbChannel);
			mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_AUDIO_BITRATE, audioStream->bitRate);
			mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_AUDIO_SAMPLERATE, audioStream->samplePerSec);
			mm_attrs_set_int_by_name (hattrs, MM_FILE_CONTENT_AUDIO_BITPERSAMPLE, audioStream->bitPerSample);
		}	
	} 
	else 
	{
		ret = -1;
	}

	/*commit attrs*/
	ret = mmf_attrs_commit ((MMHandleType)hattrs);

	return ret;
}

static int
_get_contents_info (mmf_attrs_t *attrs, MMFileSourceType *src, MMFILE_PARSE_INFO *parse)
{
	MMFileFormatContext *formatContext = NULL;
	MMFileCodecContext  *codecContext = NULL;
	MMFileFormatFrame    frameContext = {0,};
	MMFileCodecFrame     codecFrame = {0,};
	MMFileCodecFrame     decodedFrame = {0,};

	int ret = 0;
	
	if (!src || !parse)
		return MM_ERROR_FILE_INTERNAL;

	ret = mmfile_format_open (&formatContext, src);
	if (MMFILE_FORMAT_FAIL == ret || formatContext == NULL) {
		debug_error ("error: mmfile_format_open\n");
		ret = MM_ERROR_FILE_INTERNAL;
		goto exception;
	}

	/**
	 * if MM_FILE_PARSE_TYPE_SIMPLE, just get number of each stream.
	 */
	parse->audio_track_num = formatContext->audioTotalTrackNum;
	parse->video_track_num = formatContext->videoTotalTrackNum;

	if (parse->type >= MM_FILE_PARSE_TYPE_NORMAL) {
		if (parse->type == MM_FILE_PARSE_TYPE_SAFE)
			formatContext->cdis = 1;
		ret = mmfile_format_read_stream (formatContext);
		if (MMFILE_FORMAT_FAIL == ret) {
			debug_error ("error: mmfile_format_read_stream\n");
			ret = MM_ERROR_FILE_INTERNAL;
			goto exception;
		}

		/* check uhqa content*/
		if (parse->is_uhqa = formatContext->streams[MMFILE_AUDIO_STREAM] != NULL)
			parse->is_uhqa = formatContext->streams[MMFILE_AUDIO_STREAM]->is_uhqa;

		if (parse->type >= MM_FILE_PARSE_TYPE_ALL) {
			if (formatContext->videoTotalTrackNum > 0) {

				if (parse->type != MM_FILE_PARSE_TYPE_SAFE) {
					if (formatContext->formatType == MM_FILE_FORMAT_3GP ||formatContext->formatType == MM_FILE_FORMAT_MP4) {
						MMFileUtilGetMetaDataFromMP4 (formatContext);
					}
				}
				MMFileFormatStream *videoStream = formatContext->streams[MMFILE_VIDEO_STREAM];
				unsigned int timestamp = _SEEK_POINT_;

				ret = mmfile_format_read_frame (formatContext, timestamp, &frameContext);
				if (MMFILE_FORMAT_FAIL == ret) {
					debug_error ("error: mmfile_format_read_frame\n");
					ret = MM_ERROR_FILE_INTERNAL;
					goto warning;
				}

				if (frameContext.bCompressed) {
					codecFrame.frameDataSize = frameContext.frameSize;
					codecFrame.width = frameContext.frameWidth;
					codecFrame.height = frameContext.frameHeight;
					codecFrame.frameData = frameContext.frameData;
					codecFrame.configLen = frameContext.configLenth;
					codecFrame.configData = frameContext.configData;
					codecFrame.version = videoStream->version;

					ret = mmfile_codec_open (&codecContext, MMFILE_VIDEO_DECODE, videoStream->codecId, &codecFrame);
					if (MMFILE_FORMAT_FAIL == ret) {
						debug_error ("error: mmfile_codec_open\n");
						ret = MM_ERROR_FILE_INTERNAL;
						goto warning;
					}

					ret = mmfile_codec_decode (codecContext, &decodedFrame);
					if (MMFILE_FORMAT_FAIL == ret) {
						debug_error ("error: mmfile_codec_decode\n");
						ret = MM_ERROR_FILE_INTERNAL;
						goto warning;
					}
					
					/* set video thumbnail */
					formatContext->thumbNail = mmfile_malloc (sizeof(MMFileFormatFrame));
					if (NULL == formatContext->thumbNail) {
						debug_error ("error: mmfile_malloc\n");
						ret = MM_ERROR_FILE_INTERNAL;
						goto warning;
					}

					formatContext->thumbNail->frameSize = decodedFrame.frameDataSize;
					formatContext->thumbNail->frameWidth = decodedFrame.width;
					formatContext->thumbNail->frameHeight = decodedFrame.height;
					formatContext->thumbNail->frameData = decodedFrame.frameData;
					formatContext->thumbNail->configLenth = 0;
					formatContext->thumbNail->configData = NULL;
				} else {
					formatContext->thumbNail = mmfile_malloc (sizeof(MMFileFormatFrame));
					if (NULL == formatContext->thumbNail) {
						debug_error ("error: mmfile_format_read_frame\n");
						ret = MM_ERROR_FILE_INTERNAL;
						goto warning;
					}

					formatContext->thumbNail->frameSize = frameContext.frameSize;
					formatContext->thumbNail->frameWidth = frameContext.frameWidth;
					formatContext->thumbNail->frameHeight = frameContext.frameHeight;
					formatContext->thumbNail->frameData = frameContext.frameData;
					formatContext->thumbNail->configLenth = 0;
					formatContext->thumbNail->configData = NULL;
				}
			}
		}
	}

#ifdef __MMFILE_TEST_MODE__
	mmfile_format_print_frame (&frameContext);
#endif

	formatContext->commandType = MM_FILE_CONTENTS;

	if (parse->type >= MM_FILE_PARSE_TYPE_NORMAL)
		_info_set_attr_media (attrs, formatContext);

	if (frameContext.bCompressed) {
		if (frameContext.frameData) mmfile_free (frameContext.frameData); 
		if (frameContext.configData) mmfile_free (frameContext.configData);

		if (decodedFrame.frameData) {
			mmfile_free (decodedFrame.frameData);
			formatContext->thumbNail->frameData = NULL;
		}
		if (decodedFrame.configData) {
			mmfile_free (decodedFrame.configData);
			formatContext->thumbNail->configData = NULL;
		}
	} else {
		if (frameContext.frameData) {
			mmfile_free (frameContext.frameData); 
			formatContext->thumbNail->frameData = NULL;
		}
		if (frameContext.configData) {
			mmfile_free (frameContext.configData);
			formatContext->thumbNail->configData = NULL;
		}
	}

	if (formatContext)  { mmfile_format_close (formatContext); }
	if (codecContext)   { mmfile_codec_close (codecContext); }

	return MM_ERROR_NONE;

warning:
	formatContext->commandType = MM_FILE_CONTENTS;

	if (frameContext.bCompressed) {
		if (frameContext.frameData)
			mmfile_free (frameContext.frameData); 

		if (frameContext.configData) 
			mmfile_free (frameContext.configData);

		if (decodedFrame.frameData) {
			mmfile_free (decodedFrame.frameData);
			formatContext->thumbNail->frameData = NULL;
		}

		if (decodedFrame.configData) {
			mmfile_free (decodedFrame.configData);
			formatContext->thumbNail->configData = NULL;
		}
	} else {
		if (frameContext.frameData) {
			mmfile_free (frameContext.frameData); 
			formatContext->thumbNail->frameData = NULL;
		}
		
		if (frameContext.configData) {
			mmfile_free (frameContext.configData);
			formatContext->thumbNail->configData = NULL;
		}
	}

	if (parse->type >= MM_FILE_PARSE_TYPE_NORMAL)
		_info_set_attr_media (attrs, formatContext);

	if (formatContext)  { mmfile_format_close (formatContext); }
	if (codecContext)   { mmfile_codec_close (codecContext); }
	return MM_ERROR_NONE;


exception:
	if (frameContext.bCompressed) {
		if (frameContext.frameData)
			mmfile_free (frameContext.frameData); 

		if (frameContext.configData) 
			mmfile_free (frameContext.configData);

		if (decodedFrame.frameData) {
			mmfile_free (decodedFrame.frameData);
			formatContext->thumbNail->frameData = NULL;
		}

		if (decodedFrame.configData) {
			mmfile_free (decodedFrame.configData);
			formatContext->thumbNail->configData = NULL;
		}
	} else {
		if (frameContext.frameData) {
			mmfile_free (frameContext.frameData); 
			formatContext->thumbNail->frameData = NULL;
		}
		
		if (frameContext.configData) {
			mmfile_free (frameContext.configData);
			formatContext->thumbNail->configData = NULL;
		}
	}

	if (formatContext)  { mmfile_format_close (formatContext); }
	// if (codecContext)   { mmfile_codec_close (codecContext); }	/*dead code*/

	return ret;
}


static int
_get_tag_info (mmf_attrs_t *attrs, MMFileSourceType *src)
{
	MMFileFormatContext *formatContext = NULL;
	int ret = 0;

	ret = mmfile_format_open (&formatContext, src);
	if (MMFILE_FORMAT_FAIL == ret || formatContext == NULL) {
		debug_error ("error: mmfile_format_open\n");
		ret = MM_ERROR_FILE_INTERNAL;
		goto exception;
	}

	ret = mmfile_format_read_tag (formatContext);
	if (MMFILE_FORMAT_FAIL == ret) {
		debug_warning ("reading tag is fail\n");
		ret = MM_ERROR_FILE_INTERNAL;
		goto exception;
	}

	formatContext->commandType = MM_FILE_TAG;

	_info_set_attr_media (attrs, formatContext);

	if (formatContext)  { mmfile_format_close (formatContext); }

	return MM_ERROR_NONE;


exception:
	if (formatContext)  { mmfile_format_close (formatContext); }

	return MM_ERROR_FILE_INTERNAL;
}


/**
 * global functions.
 */
int mm_file_get_attrs(MMHandleType attrs, char **err_attr_name, const char *first_attribute_name, ...)
{
	int ret = MM_ERROR_NONE;
	va_list var_args;

	if ( !attrs )	
	{
		debug_error ("Invalid arguments [attrs 0]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}
	
	if ( first_attribute_name == NULL)	
	{
		debug_error ("Invalid arguments [first_attribute_name null]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}

	/* get requested attributes */
	va_start (var_args, first_attribute_name);
	ret = mm_attrs_get_valist(attrs, err_attr_name, first_attribute_name, var_args);
	va_end (var_args);

	if (ret != MM_ERROR_NONE)
	{
		if (err_attr_name)
		{
			debug_error ("failed to get %s\n", *err_attr_name);
		}
	}

	return ret;
}

int mm_file_get_synclyrics_info(MMHandleType tag_attrs, int index, unsigned long *time_info, char **lyrics)
{
	int ret = MM_ERROR_NONE;
	AvSynclyricsInfo* sync_lyric_item = NULL;
	GList *synclyrics_list = NULL;

#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

	if ( (mmf_attrs_t*)tag_attrs == NULL) {
		debug_error ("invalid handle");
		return MM_ERROR_INVALID_ARGUMENT;
	}

	ret = mm_attrs_get_data_by_name (tag_attrs, MM_FILE_TAG_SYNCLYRICS, (void **)&synclyrics_list);
	if(ret != MM_ERROR_NONE) {
		#ifdef __MMFILE_TEST_MODE__
			debug_warning (  "get data fail");
		#endif
		return ret;
	}
	
	if(synclyrics_list != NULL) {

		sync_lyric_item = (AvSynclyricsInfo*)g_list_nth_data(synclyrics_list, index);

		if(sync_lyric_item == NULL) {
		#ifdef __MMFILE_TEST_MODE__
				debug_warning (  "synclyric item is NULL");
		#endif
			return MM_ERROR_COMMON_ATTR_NOT_EXIST;
		}

		*time_info = sync_lyric_item->time_info;
		*lyrics = sync_lyric_item->lyric_info;

	} else {
		#ifdef __MMFILE_TEST_MODE__
			debug_warning (  "synclyrics_list is NULL");
		#endif
		return MM_ERROR_COMMON_ATTR_NOT_EXIST;
	}
	
	return ret;
	
}

int mm_file_create_tag_attrs(MMHandleType *tag_attrs, const char *filename)
{
	int ret = MM_ERROR_NONE;
	mmf_attrs_t *attrs = NULL;
	MMFileSourceType src;

#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

	/* Check argument here */
	if (tag_attrs == NULL) {
		debug_error ("Invalid arguments [tag null]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}
	if (filename == NULL) {
		debug_error ("Invalid arguments [filename null]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}
	if ( strlen (filename) == 0)	{
		debug_error ("Invalid arguments [filename size 0]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}


#ifdef __MMFILE_DYN_LOADING__
	MMFILE_FUNC_HANDLE func_handle;
	
	ret = _load_dynamic_functions (&func_handle);
	if (ret == 0) {
		debug_error ("load library error\n");
		return MM_ERROR_FILE_INTERNAL;
	}
#endif

	/*set source file infomation*/
	MM_FILE_SET_MEDIA_FILE_SRC (src, filename);

	ret = _is_file_exist (filename);
	if (!ret) {
		ret = MM_ERROR_FILE_NOT_FOUND;
		goto END;
	}

	/*set attrs*/
	attrs = (mmf_attrs_t *) mmf_attrs_new_from_data ("tag", g_tag_attrs, ARRAY_SIZE (g_tag_attrs), NULL, NULL);
	if (!attrs) {
		debug_error ("attribute internal error.\n");
		ret = MM_ERROR_FILE_INTERNAL;
		goto END;
	}

	ret = _get_tag_info (attrs, &src);
	if (ret != MM_ERROR_NONE) {
		mmf_attrs_free ((MMHandleType)attrs);
		attrs = NULL;
		debug_error ("failed to get tag: %s\n", filename);
	}

	*tag_attrs = (MMHandleType)attrs;

END:
#ifdef __MMFILE_DYN_LOADING__
	_unload_dynamic_functions (&func_handle);
#endif

#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif

	return ret;
}


EXPORT_API
int mm_file_destroy_tag_attrs(MMHandleType tag_attrs)
{
	void *artwork = NULL;
	GList *synclyrics_list = NULL;
	int ret = MM_ERROR_NONE;

#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

	if ( (mmf_attrs_t*)tag_attrs == NULL) {
		debug_error ("invalid handle.\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}

	ret = mm_attrs_get_data_by_name (tag_attrs, MM_FILE_TAG_ARTWORK, &artwork);
	
	if (artwork != NULL) {
		mmfile_free (artwork);
	}

	ret = mm_attrs_get_data_by_name (tag_attrs, MM_FILE_TAG_SYNCLYRICS, (void **)&synclyrics_list);

	if(synclyrics_list != NULL) {
		mm_file_free_synclyrics_list(synclyrics_list);
	}

	mmf_attrs_free (tag_attrs);

#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif

	return ret;
}

EXPORT_API
int mm_file_create_content_attrs (MMHandleType *contents_attrs, const char *filename)
{
	mmf_attrs_t *attrs = NULL;
	MMFileSourceType src = {0,};
	MMFILE_PARSE_INFO parse = {0,};
	int ret = 0;

#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

	/* Check argument here */
	if (contents_attrs == NULL) {
		debug_error ("Invalid arguments [contents null]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}
	if (filename == NULL) {
		debug_error ("Invalid arguments [filename null]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}
	if ( strlen (filename) == 0)	{
		debug_error ("Invalid arguments [filename size 0]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}


#ifdef __MMFILE_DYN_LOADING__
	MMFILE_FUNC_HANDLE func_handle;

	#ifdef CHECK_TIME
	 int64_t ti;
	ti = gettime();
	#endif 
	
	ret = _load_dynamic_functions (&func_handle);
	if (ret == 0) {
		debug_error ("load library error\n");
		return MM_ERROR_FILE_INTERNAL;
	}

	#ifdef CHECK_TIME
	debug_msg("_load_dynamic_functions() = %lld\n", gettime() - ti);
       #endif
	 
#endif

	/*set source file infomation*/
	MM_FILE_SET_MEDIA_FILE_SRC (src, filename);

	ret = _is_file_exist (filename);
	if (!ret) {
		ret = MM_ERROR_FILE_NOT_FOUND;
		goto END;
	}

	/*set attrs*/
	attrs = (mmf_attrs_t *) mmf_attrs_new_from_data ("content", g_content_attrs, ARRAY_SIZE (g_content_attrs), NULL, NULL);
	if (!attrs) {
		debug_error ("attribute internal error.\n");
		ret = MM_ERROR_FILE_INTERNAL;
		goto END;
	}
	

	parse.type = MM_FILE_PARSE_TYPE_ALL;
	ret = _get_contents_info (attrs, &src, &parse);
	if (ret != MM_ERROR_NONE) {
		mmf_attrs_free ((MMHandleType)attrs);
		attrs = NULL;
		debug_error ("failed to get contents: %s\n", filename);
	}

	*contents_attrs = (MMHandleType) attrs;


END:
#ifdef __MMFILE_DYN_LOADING__

	#ifdef CHECK_TIME 
	ti = gettime();
	#endif

	_unload_dynamic_functions (&func_handle);

	#ifdef CHECK_TIME
	debug_msg("_unload_dynamic_functions() = %lld\n", gettime() - ti);
	#endif

#endif

#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif

	return ret;
}


EXPORT_API
int mm_file_create_tag_attrs_from_memory (MMHandleType *tag_attrs, const void *data, unsigned int size, int format)
{
	mmf_attrs_t *attrs = NULL;
	MMFileSourceType src;
	MMFILE_PARSE_INFO parse = {0,};
	int ret = 0;

#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

	/* Check argument here */
	if (tag_attrs == NULL || data == NULL) {
		debug_error ("Invalid arguments\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}

#ifdef __MMFILE_DYN_LOADING__
	MMFILE_FUNC_HANDLE func_handle;

	ret = _load_dynamic_functions (&func_handle);
	if (ret == 0) {
		debug_error ("load library error\n");
		return MM_ERROR_FILE_INTERNAL;
	}
#endif

	MM_FILE_SET_MEDIA_MEM_SRC (src, data, size, format);

	/*set attrs*/
	attrs = (mmf_attrs_t *) mmf_attrs_new_from_data ("tag", g_tag_attrs, ARRAY_SIZE (g_tag_attrs), NULL, NULL);
	if (!attrs) {
		debug_error ("attribute internal error.\n");
		ret = MM_ERROR_FILE_INTERNAL;
		goto END;
	}

	parse.type = MM_FILE_PARSE_TYPE_ALL;
	ret = _get_tag_info (attrs, &src);
	if (ret != MM_ERROR_NONE) {
		mmf_attrs_free ((MMHandleType)attrs);
		attrs = NULL;
		debug_error ("failed to get tag");
	}

	*tag_attrs = (MMHandleType)attrs;

END:
#ifdef __MMFILE_DYN_LOADING__
	_unload_dynamic_functions (&func_handle);
#endif

#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif

	return ret;
}


EXPORT_API
int mm_file_create_content_attrs_from_memory (MMHandleType *contents_attrs, const void *data, unsigned int size, int format)
{
	mmf_attrs_t *attrs = NULL;
	MMFileSourceType src;
	MMFILE_PARSE_INFO parse = {0,};
	int ret = 0;

#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

	/* Check argument here */
	if (contents_attrs == NULL || data == NULL) {
		debug_error ("Invalid arguments\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}

#ifdef __MMFILE_DYN_LOADING__
	MMFILE_FUNC_HANDLE func_handle;
	
	ret = _load_dynamic_functions (&func_handle);
	if (ret == 0) {
		debug_error ("load library error\n");
		return MM_ERROR_FILE_INTERNAL;
	}
#endif

	MM_FILE_SET_MEDIA_MEM_SRC (src, data, size, format);

	/*set attrs*/
	attrs = (mmf_attrs_t *) mmf_attrs_new_from_data ("content", g_content_attrs, ARRAY_SIZE (g_content_attrs), NULL, NULL);
	if (!attrs) {
		debug_error ("attribute internal error.\n");
		ret = MM_ERROR_FILE_INTERNAL;
		goto END;
	}

	parse.type = MM_FILE_PARSE_TYPE_ALL;
	ret = _get_contents_info (attrs, &src, &parse);
	if (ret != MM_ERROR_NONE) {
		mmf_attrs_free ((MMHandleType)attrs);
		attrs = NULL;
		debug_error ("failed to get contents");
	}

	*contents_attrs = (MMHandleType)attrs;

END:
#ifdef __MMFILE_DYN_LOADING__
	_unload_dynamic_functions (&func_handle);
#endif

#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif

	return ret;
}


EXPORT_API
int mm_file_destroy_content_attrs (MMHandleType contents_attrs)
{
	void *thumbnail = NULL;
	int ret = MM_ERROR_NONE;

#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

	if ((mmf_attrs_t*)contents_attrs == NULL) {
		debug_error ("invalid handle.\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}

	ret = mm_attrs_get_data_by_name(contents_attrs, MM_FILE_CONTENT_VIDEO_THUMBNAIL, &thumbnail);
	if (thumbnail != NULL) {
		mmfile_free (thumbnail);
	}

	mmf_attrs_free (contents_attrs);

#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif

	return ret;
}


EXPORT_API
int mm_file_get_stream_info(const char* filename, int *audio_stream_num, int *video_stream_num)
{
	MMFileSourceType     src = {0,};
	MMFILE_PARSE_INFO    parse = {0,};

	int ret = 0;

#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

	if (filename == NULL || strlen (filename) == 0 || audio_stream_num == NULL || video_stream_num == NULL) {
		debug_error ("Invalid arguments\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}

#ifdef __MMFILE_DYN_LOADING__
	MMFILE_FUNC_HANDLE func_handle;

	ret = _load_dynamic_functions (&func_handle);
	if (ret == 0) {
		debug_error ("load library error\n");
		return MM_ERROR_FILE_INTERNAL;
	}
#endif

	/*set source file infomation*/
	MM_FILE_SET_MEDIA_FILE_SRC (src, filename);

	ret = _is_file_exist (filename);
	if (!ret) {
		ret = MM_ERROR_FILE_NOT_FOUND;
		goto END;
	}

	parse.type = MM_FILE_PARSE_TYPE_SIMPLE;
	ret = _get_contents_info (NULL, &src, &parse);
	if (ret != MM_ERROR_NONE) {
		debug_error ("failed to get stream info: %s\n", filename);
	}

	/*set number of each stream*/
	*audio_stream_num = parse.audio_track_num;
	*video_stream_num = parse.video_track_num;

END:
#ifdef __MMFILE_DYN_LOADING__
	_unload_dynamic_functions (&func_handle);
#endif

#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif

	return ret;
}

EXPORT_API
int mm_file_create_content_attrs_simple(MMHandleType *contents_attrs, const char *filename)
{
	mmf_attrs_t *attrs = NULL;
	MMFileSourceType src = {0,};
	MMFILE_PARSE_INFO parse = {0,};
	int ret = 0;

#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

#ifdef __MMFILE_DYN_LOADING__
	MMFILE_FUNC_HANDLE func_handle;
	
	ret = _load_dynamic_functions (&func_handle);
	if (ret == 0) {
		debug_error ("load library error\n");
		return MM_ERROR_FILE_INTERNAL;
	}
#endif
	if (filename == NULL) {
		ret = MM_ERROR_INVALID_ARGUMENT;
		goto END;
	} else {
		if (strlen (filename) == 0) {
			ret = MM_ERROR_INVALID_ARGUMENT;
			goto END;
		}
	}

	/*set source file infomation*/
	MM_FILE_SET_MEDIA_FILE_SRC (src, filename);

	ret = _is_file_exist (filename);
	if (!ret) {
		ret = MM_ERROR_FILE_NOT_FOUND;
		goto END;
	}

	/*set attrs*/
	attrs = (mmf_attrs_t *) mmf_attrs_new_from_data ("content", g_content_attrs, ARRAY_SIZE (g_content_attrs), NULL, NULL);
	if (!attrs) {
		debug_error ("attribute internal error.\n");
		ret = MM_ERROR_FILE_INTERNAL;
		goto END;
	}

	parse.type = MM_FILE_PARSE_TYPE_NORMAL;
	ret = _get_contents_info (attrs, &src, &parse);
	if (ret != MM_ERROR_NONE) {
		mmf_attrs_free ((MMHandleType)attrs);
		attrs = NULL;
		debug_error ("failed to get contents: %s\n", filename);
	}

	*contents_attrs = (MMHandleType) attrs;

END:
#ifdef __MMFILE_DYN_LOADING__
	_unload_dynamic_functions (&func_handle);
#endif

#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif

	return ret;
}

EXPORT_API
int mm_file_create_content_attrs_safe(MMHandleType *contents_attrs, const char *filename)
{
	mmf_attrs_t *attrs = NULL;
	MMFileSourceType src = {0,};
	MMFILE_PARSE_INFO parse = {0,};
	int ret = 0;

#ifdef __MMFILE_TEST_MODE__
	debug_fenter();
#endif

#ifdef __MMFILE_DYN_LOADING__
	MMFILE_FUNC_HANDLE func_handle;

	ret = _load_dynamic_functions (&func_handle);
	if (ret == 0) {
		debug_error ("load library error\n");
		return MM_ERROR_FILE_INTERNAL;
	}
#endif
	if (filename == NULL) {
		ret = MM_ERROR_INVALID_ARGUMENT;
		goto END;
	} else {
		if (strlen (filename) == 0) {
			ret = MM_ERROR_INVALID_ARGUMENT;
			goto END;
		}
	}

	/*set source file infomation*/
	MM_FILE_SET_MEDIA_FILE_SRC (src, filename);

	ret = _is_file_exist (filename);
	if (!ret) {
		ret = MM_ERROR_FILE_NOT_FOUND;
		goto END;
	}

	/*set attrs*/
	attrs = (mmf_attrs_t *) mmf_attrs_new_from_data ("content", g_content_attrs, ARRAY_SIZE (g_content_attrs), NULL, NULL);
	if (!attrs) {
		debug_error ("attribute internal error.\n");
		ret = MM_ERROR_FILE_INTERNAL;
		goto END;
	}

	parse.type = MM_FILE_PARSE_TYPE_SAFE;
	ret = _get_contents_info (attrs, &src, &parse);
	if (ret != MM_ERROR_NONE) {
		mmf_attrs_free ((MMHandleType)attrs);
		attrs = NULL;
		debug_error ("failed to get contents: %s\n", filename);
	}

	*contents_attrs = (MMHandleType) attrs;

END:
#ifdef __MMFILE_DYN_LOADING__
	_unload_dynamic_functions (&func_handle);
#endif

#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif

	return ret;
}

EXPORT_API
int mm_file_get_video_frame(const char* path, double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height)
{
	int ret = 0;
	void *formatFuncHandle = NULL;

	if (path == NULL) {
		debug_error ("Invalid arguments [Path is Null]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}

#ifdef __MMFILE_DYN_LOADING__
	/* Get from function argument */
	formatFuncHandle = dlopen (MMFILE_FORMAT_SO_FILE_NAME, RTLD_LAZY);
	if (!formatFuncHandle) {
		debug_error ("error : dlopen");
		goto exception;
	}

	mmfile_format_get_frame = dlsym (formatFuncHandle, "mmfile_format_get_frame");
	if ( !mmfile_format_get_frame ) {
		debug_error ("error : load library");
		goto exception;
	}
#endif

#ifdef __MMFILE_TEST_MODE__
	debug_msg("file path [%s] is_accurate [%d]", path, is_accurate);
#endif

	ret = mmfile_format_get_frame(path, timestamp, is_accurate, frame, size, width, height);
	if (ret  == MMFILE_FORMAT_FAIL) {
		debug_error ("error : get frame");
		goto exception;
	}

	if (formatFuncHandle) dlclose (formatFuncHandle);

	return MM_ERROR_NONE;

exception:
	if (formatFuncHandle) dlclose (formatFuncHandle);

	return MM_ERROR_FILE_INTERNAL;
}

EXPORT_API
int mm_file_get_video_frame_from_memory(const void *data, unsigned int datasize,  double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height)
{
	int ret = 0;
	void *formatFuncHandle = NULL;

	if (data == NULL) {
		debug_error ("Invalid arguments [data is Null]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}

	if (datasize == 0) {
		debug_error ("Invalid arguments [datasize is zero]\n");
		return MM_ERROR_INVALID_ARGUMENT;
	}

#ifdef __MMFILE_DYN_LOADING__
	/* Get from function argument */
	formatFuncHandle = dlopen (MMFILE_FORMAT_SO_FILE_NAME, RTLD_LAZY);
	if (!formatFuncHandle) {
		debug_error ("error : dlopen");
		goto exception;
	}

	mmfile_format_get_frame_from_memory = dlsym (formatFuncHandle, "mmfile_format_get_frame_from_memory");
	if ( !mmfile_format_get_frame_from_memory ) {
		debug_error ("error : load library");
		goto exception;
	}
#endif

#ifdef __MMFILE_TEST_MODE__
	debug_msg("data [%p], data_size[%d], is_accurate [%d]", data, datasize, is_accurate);
#endif

	ret = mmfile_format_get_frame_from_memory(data, datasize, timestamp, is_accurate, frame, size, width, height);
	if (ret  == MMFILE_FORMAT_FAIL) {
		debug_error ("error : get frame");
		goto exception;
	}

	if (formatFuncHandle) dlclose (formatFuncHandle);

	return MM_ERROR_NONE;

exception:
	if (formatFuncHandle) dlclose (formatFuncHandle);

	return MM_ERROR_FILE_INTERNAL;
}

EXPORT_API
int mm_file_check_uhqa(const char* filename, bool *is_uhqa)
{
	mmf_attrs_t *attrs = NULL;
	MMFileSourceType src = {0,};
	MMFILE_PARSE_INFO parse = {0,};
	int ret = 0;

#ifdef __MMFILE_DYN_LOADING__
	MMFILE_FUNC_HANDLE func_handle;

	ret = _load_dynamic_functions (&func_handle);
	if (ret == 0) {
		debug_error ("load library error\n");
		return MM_ERROR_FILE_INTERNAL;
	}
#endif
	if (filename == NULL) {
		ret = MM_ERROR_INVALID_ARGUMENT;
		goto END;
	} else {
		if (strlen (filename) == 0) {
			ret = MM_ERROR_INVALID_ARGUMENT;
			goto END;
		}
	}

	/*set source file infomation*/
	MM_FILE_SET_MEDIA_FILE_SRC (src, filename);

	ret = _is_file_exist (filename);
	if (!ret) {
		ret = MM_ERROR_FILE_NOT_FOUND;
		goto END;
	}

	/*set attrs*/
	attrs = (mmf_attrs_t *) mmf_attrs_new_from_data ("content", g_content_attrs, ARRAY_SIZE (g_content_attrs), NULL, NULL);
	if (!attrs) {
		debug_error ("attribute internal error.\n");
		ret = MM_ERROR_FILE_INTERNAL;
		goto END;
	}

	parse.type = MM_FILE_PARSE_TYPE_NORMAL;
	ret = _get_contents_info (attrs, &src, &parse);
	if (ret == MM_ERROR_NONE) {
		*is_uhqa = parse.is_uhqa;
	} else {
		debug_error ("_get_contents_info failed\n");
		*is_uhqa = FALSE;
	}

	mmf_attrs_free ((MMHandleType)attrs);
	attrs = NULL;

END:
#ifdef __MMFILE_DYN_LOADING__
	_unload_dynamic_functions (&func_handle);
#endif

#ifdef __MMFILE_TEST_MODE__
	debug_fleave();
#endif

	return ret;
}