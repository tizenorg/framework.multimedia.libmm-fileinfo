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

#ifndef _MMFILE_FORMATS_H_
#define _MMFILE_FORMATS_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <glib.h>

#define MAXSTREAMS              20
#define MMFILE_VIDEO_STREAM     0
#define MMFILE_AUDIO_STREAM     1

#define MMFILE_FORMAT_SUCCESS   1
#define MMFILE_FORMAT_FAIL      0


#define MM_FILE_SET_MEDIA_FILE_SRC(Media, Filename)		do { \
		(Media).type = MM_FILE_SRC_TYPE_FILE; \
		(Media).file.path = ((strstr(Filename, "file://")!=NULL) ? Filename+7:Filename); \
	} while (0);

#define MM_FILE_SET_MEDIA_MEM_SRC(Media, Memory, Size, Format)		do { \
		(Media).type = MM_FILE_SRC_TYPE_MEMORY; \
		(Media).memory.ptr = (Memory); \
		(Media).memory.size = (Size); \
		(Media).memory.format = (Format); \
	} while (0);



enum {
	MM_FILE_SRC_TYPE_FILE,
	MM_FILE_SRC_TYPE_MEMORY,
};

typedef struct _mm_file_source {
	int		type;
	union {
		struct {
			const char *path;
		} file;
		struct {
			const void *ptr;
			unsigned int size;
			int format;     /* _mmfileformats */
		} memory;
	};
} MMFileSourceType;


typedef struct _mmfilesteam {
	int	streamType;
	int	codecId;
	int	version;
	int	bitRate;
	int	framePerSec;
	int	width;
	int	height;
	int	nbChannel;
	int	samplePerSec;
	int 	bitPerSample;
	bool is_uhqa;
} MMFileFormatStream;

typedef struct _mmfileformatframe {
	unsigned char	bCompressed;
	unsigned int	frameSize;
	unsigned int	frameWidth;
	unsigned int	frameHeight;
	unsigned int	configLenth;
	unsigned char	*frameData;
	void			*configData;
	unsigned int	timestamp;
	unsigned int	frameNumber;
} MMFileFormatFrame;


typedef struct _MMFileFormatContext MMFileFormatContext;

struct _MMFileFormatContext {
	int notsupport;
	int formatType;
	int commandType;	/* TAG or CONTENTS */
	int pre_checked;	/*filefomat already detected.*/

	MMFileSourceType *filesrc;	/*ref only*/
	char *uriFileName;

	/* contents information */
	int duration;	/* milliseconds */
	int isseekable;
	int videoTotalTrackNum;
	int audioTotalTrackNum;
	int nbStreams;
	int audioStreamId;
	int videoStreamId;
	MMFileFormatStream *streams[MAXSTREAMS];

	/* thumbnail info */
	MMFileFormatFrame *thumbNail;

	/* tag info */
	char *title;
	char *artist;
	char *author;
	char *composer;
	char *album;
	char *album_artist;
	char *copyright;
	char *description;
	char *comment;
	char *genre;
	char *classification;
	char *year;
	char *recDate;
	char *tagTrackNum;
	char *rating;
	int artworkSize;
	char *artworkMime;
	unsigned char *artwork;
	float  longitude;
	float  latitude;
	float  altitude;

	char *conductor;
	char *unsyncLyrics;
	char *rotate;
	GList *syncLyrics;
	int syncLyricsNum;
	int cdis;
	int smta;

	/* private data */
	void *privateFormatData;
	void *privateCodecData;

	/* function pointer */
	int (*ReadStream)(MMFileFormatContext *);
	int (*ReadFrame)(MMFileFormatContext *, unsigned int, MMFileFormatFrame *);
	int (*ReadTag)(MMFileFormatContext *);
	int (*Close)(MMFileFormatContext *);
};

#ifndef __MMFILE_DYN_LOADING__
int mmfile_format_open(MMFileFormatContext **formatContext, MMFileSourceType *fileSrc);
int mmfile_format_read_stream(MMFileFormatContext *formatContext);
int mmfile_format_read_frame(MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int mmfile_format_read_tag(MMFileFormatContext *formatContext);
int mmfile_format_close(MMFileFormatContext *formatContext);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MMFILE_FORMATS_H_ */

