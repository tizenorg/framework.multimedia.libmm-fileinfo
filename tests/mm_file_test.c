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
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <mm_file.h>
#include <mm_error.h>

#include "mm_file_traverse.h"

#define MM_TIME_CHECK_START \
{ FILE *msg_tmp_fp = fopen("time_check.txt", "a+"); struct timeval start, finish; gettimeofday(&start, NULL);
#define MM_TIME_CHECK_FINISH(title) \
gettimeofday(&finish, NULL); \
double end_time = (finish.tv_sec + 1e-6*finish.tv_usec); \
double start_time = (start.tv_sec + 1e-6*start.tv_usec); \
fprintf(msg_tmp_fp, "%s\n", title); \
fprintf(msg_tmp_fp, " - start_time:   %3.5lf sec\n", start_time); \
fprintf(msg_tmp_fp, " - finish_time:  %3.5lf sec\n", end_time); \
fprintf(msg_tmp_fp, " - elapsed time: %3.5lf sec\n", end_time - start_time); \
fflush(msg_tmp_fp); fclose(msg_tmp_fp); }

typedef struct _mmfile_value {
	int len;
	union {
		int i_val;
		double d_val;
		char *s_val;
		void *p_val;
	}value;
}mmfile_value_t;

typedef struct _TagContext {
	mmfile_value_t artist;
	mmfile_value_t title;
	mmfile_value_t album;
	mmfile_value_t album_artist;
	mmfile_value_t genre;
	mmfile_value_t author;
	mmfile_value_t copyright;
	mmfile_value_t date; 			//string
	mmfile_value_t recdate;			//string
	mmfile_value_t description;
	mmfile_value_t comment;
	mmfile_value_t artwork;		//data
	mmfile_value_t artwork_size;	//int
	mmfile_value_t artwork_mime;
	mmfile_value_t track_num;
	mmfile_value_t classfication;
	mmfile_value_t rating;
	mmfile_value_t conductor;
	mmfile_value_t longitude;		//-> double
	mmfile_value_t latitude;		
	mmfile_value_t altitude;		//<-double
	mmfile_value_t unsynclyrics;
	mmfile_value_t synclyrics_size;
	mmfile_value_t rotate;			//string
}TagContext_t;

typedef struct _ContentContext {
	int duration;
	int video_codec;
	int video_bitrate;
	int video_fps;
	int video_w;
	int video_h;
	int video_track_id;
	int video_track_num;
	int audio_codec;
	int audio_bitrate;
	int audio_channel;
	int audio_samplerate;
	int audio_track_id;
	int audio_track_num;
	mmfile_value_t thumbnail;
}ContentContext_t;


char * AudioCodecTypeString [] = {
	"AMR", "G723.1", "MP3", "OGG", "AAC", "WMA", "MMF", "ADPCM", "WAVE", "WAVE NEW", "MIDI", "IMELODY", "MXMF", "MPEG1-Layer1 codec", "MPEG1-Layer2 codec",
	"G711", "G722", "G722.1",	"G722.2  (AMR-WB)",	"G723 wideband speech",	"G726 (ADPCM)",	"G728 speech",	"G729",	"G729a",	"G729.1",
	"Real",
	"AAC-Low complexity",	"AAC-Main profile",	"AAC-Scalable sample rate",	"AAC-Long term prediction",	"AAC-High Efficiency v1",	"AAC-High efficiency v2",
	"DolbyDigital",	"Apple Lossless",	"Sony proprietary",	"SPEEX",	"Vorbis",	"AIFF",	"AU",	"None (will be deprecated)",
	"PCM",	"ALAW",	"MULAW",	"MS ADPCM",	"FLAC"
};


char * VideoCodecTypeString [] = {
	"None (will be deprecated)",
	"H263", "H264", "H26L", "MPEG4", "MPEG1", "WMV", "DIVX", "XVID", "H261", "H262/MPEG2-part2", "H263v2",  "H263v3",
	"Motion JPEG", "MPEG2", "MPEG4 part-2 Simple profile",	"MPEG4 part-2 Advanced Simple profile",	"MPEG4 part-2 Main profile",
	"MPEG4 part-2 Core profile", "MPEG4 part-2 Adv Coding Eff profile",	"MPEG4 part-2 Adv RealTime Simple profile",
	"MPEG4 part-10 (h.264)",	"Real",	"VC-1",	"AVS",	"Cinepak",	"Indeo",	"Theora", "Flv"
};



FILE *fpFailList = NULL;

static int mmfile_get_file_infomation (void *data, void* user_data, bool file_test);

inline static int mm_file_is_little_endian (void)
{
    int i = 0x00000001;
    return ((char *)&i)[0];
}

#define READ_FROM_FILE(FILE_PATH, data, size) \
do{	\
	FILE * fp = fopen (FILE_PATH, "r");	\
	if (fp) {	\
			fseek (fp, 0, SEEK_END);	\
			size = ftell(fp);	\
			fseek (fp, 0, SEEK_SET);	\
			data = malloc (size);	\
			if (fread (data, size, sizeof(char), fp) != size) { printf("fread error\n"); }	\
			fclose (fp);	\
			printf("file size = %d\n", size );	\
	}	\
}while(0)

static int
_is_file_exist (const char *filename)
{
	int ret = 1;
	if (filename) {
		const char* to_access = (strstr(filename,"file://")!=NULL)? filename+7:filename;
		ret = access (to_access, R_OK );
		if (ret != 0) {
			printf  ("file [%s] not found.\n", to_access);
		}
	}
	return !ret;
}


int main(int argc, char **argv)
{
    struct stat statbuf;
	bool file_test = true;		//if you want to test mm_file_create_content_XXX_from_memory() set file_test to false

    if (_is_file_exist (argv[1])) {
    	int ret = lstat (argv[1], &statbuf);
   	    if ( ret < 0 ) {
   	    	printf ("lstat error[%d]\n", ret);
   	    	return MMFILE_FAIL;
    	}

		if (fpFailList == NULL) {
			fpFailList = fopen ("/opt/var/log/mmfile_fails.txt", "w");
		}

		if ( S_ISDIR (statbuf.st_mode) )	{
			mmfile_get_file_names (argv[1], mmfile_get_file_infomation, NULL);
		} else {
			mmfile_get_file_infomation (argv[1], NULL, file_test);
		}

		if (fpFailList != NULL) {
			fflush (fpFailList);
			fclose (fpFailList);
		}
    }

    exit(0);
}

static int mmfile_get_file_infomation (void *data, void* user_data, bool file_test)
{
	MMHandleType content_attrs = 0;
	MMHandleType tag_attrs = 0;
	char *err_attr_name = NULL;
	int audio_track_num = 0;
	int video_track_num = 0;
	int ret = 0;
	char filename[512];

	memset (filename, 0x00, 512);
	memcpy (filename, (char*)data, strlen ((char*)data));

	MM_TIME_CHECK_START

	printf ("Extracting information for [%s] \n", filename);
	/* get track info */
	ret = mm_file_get_stream_info(filename, &audio_track_num, &video_track_num);
	if (ret == MM_ERROR_NONE) {
		printf ("# audio=%d, video=%d\n", audio_track_num, video_track_num);
	} else {
		printf ("Failed to mm_file_get_stream_info() error=[%x]\n", ret);
	}

	if(file_test) {
		/* get content handle */
		ret = mm_file_create_content_attrs(&content_attrs, filename);
	} else {
		int file_size = 0;
		unsigned char * buffer = NULL;
		/* Read file */
		READ_FROM_FILE(filename, buffer, file_size);

		ret =mm_file_create_content_attrs_from_memory(&content_attrs, buffer, file_size, MM_FILE_FORMAT_3GP);
	}

	if (ret == MM_ERROR_NONE && content_attrs) {
		ContentContext_t ccontent;
		memset (&ccontent, 0, sizeof (ContentContext_t));

		ret = mm_file_get_attrs(content_attrs, &err_attr_name, MM_FILE_CONTENT_DURATION, &ccontent.duration, NULL);
		printf("# duration: %d\n", ccontent.duration);

		if (ret != MM_ERROR_NONE && err_attr_name)
		{
			printf("failed to get %s\n", err_attr_name);
			free(err_attr_name);
			err_attr_name = NULL;
		}

		if (audio_track_num)
		{
			ret = mm_file_get_attrs(content_attrs,
									NULL,
									MM_FILE_CONTENT_AUDIO_CODEC, &ccontent.audio_codec,
									MM_FILE_CONTENT_AUDIO_SAMPLERATE, &ccontent.audio_samplerate,
									MM_FILE_CONTENT_AUDIO_BITRATE, &ccontent.audio_bitrate,
									MM_FILE_CONTENT_AUDIO_CHANNELS, &ccontent.audio_channel,
									MM_FILE_CONTENT_AUDIO_TRACK_INDEX, &ccontent.audio_track_id,
									MM_FILE_CONTENT_AUDIO_TRACK_COUNT, &ccontent.audio_track_num,
									NULL);

			if(ret != MM_ERROR_NONE) {
				printf("failed to get audio attrs\n");
			} else {
				printf ("[Audio] ----------------------------------------- \n");
				printf("# audio codec: %d ", ccontent.audio_codec);
				printf ("[%s]\n", (ccontent.audio_codec >= 0 && ccontent.audio_codec < MM_AUDIO_CODEC_NUM)? AudioCodecTypeString[ccontent.audio_codec] : "Invalid");
				printf("# audio samplerate: %d Hz\n", ccontent.audio_samplerate);
				printf("# audio bitrate: %d bps\n", ccontent.audio_bitrate);
				printf("# audio channel: %d\n", ccontent.audio_channel);
				printf("# audio track id: %d\n", ccontent.audio_track_id);
				printf("# audio track num: %d\n", ccontent.audio_track_num);
			}
		}
	
		if (video_track_num)
		{
			ret = mm_file_get_attrs(content_attrs,
									NULL,
									MM_FILE_CONTENT_VIDEO_CODEC, &ccontent.video_codec,
									MM_FILE_CONTENT_VIDEO_BITRATE, &ccontent.video_bitrate,
									MM_FILE_CONTENT_VIDEO_FPS, &ccontent.video_fps,
									MM_FILE_CONTENT_VIDEO_TRACK_INDEX, &ccontent.video_track_id,
									MM_FILE_CONTENT_VIDEO_WIDTH, &ccontent.video_w,
									MM_FILE_CONTENT_VIDEO_HEIGHT, &ccontent.video_h,
									MM_FILE_CONTENT_VIDEO_THUMBNAIL, &ccontent.thumbnail.value.p_val, &ccontent.thumbnail.len,
									NULL);

			if(ret != MM_ERROR_NONE) {
				printf("failed to get video attrs\n");
			} else {
				printf ("[Video] ----------------------------------------- \n");
				printf("# video codec: %d ", ccontent.video_codec);
				printf ("[%s]\n", (ccontent.video_codec >= 0 && ccontent.video_codec < MM_VIDEO_CODEC_NUM)? VideoCodecTypeString[ccontent.video_codec] : "Invalid");
				printf("# video bitrate: %d bps\n", ccontent.video_bitrate);
				printf("# video fps: %d\n", ccontent.video_fps);
				printf("# video track id: %d\n", ccontent.video_track_id);
				printf("# video width/height: %d x %d\n", ccontent.video_w, ccontent.video_h);
				printf("# video thumbnail: %p\n", ccontent.thumbnail.value.p_val);
			}
		}

		mm_file_destroy_content_attrs(content_attrs);
	} else {
		printf ("Failed to mm_file_create_content_attrs() error=[%x]\n", ret);
	}

	if(file_test) {
		/* get tag handle */
		ret = mm_file_create_tag_attrs(&tag_attrs, filename);
	} else {
		int file_size = 0;
		unsigned char * buffer = NULL;
		/* Read file */
		READ_FROM_FILE(filename, buffer, file_size);

		ret =mm_file_create_tag_attrs_from_memory(&tag_attrs, buffer, file_size, MM_FILE_FORMAT_3GP);
	}

	if (ret == MM_ERROR_NONE && tag_attrs) {
		TagContext_t ctag;
		memset (&ctag, 0, sizeof (TagContext_t));
		/* get attributes of tag  */
		ret = mm_file_get_attrs( tag_attrs,
									&err_attr_name,
									MM_FILE_TAG_ARTIST, &ctag.artist.value.s_val, &ctag.artist.len,
									MM_FILE_TAG_ALBUM, &ctag.album.value.s_val, &ctag.album.len,
									MM_FILE_TAG_ALBUM_ARTIST, &ctag.album_artist.value.s_val, &ctag.album_artist.len,
									MM_FILE_TAG_TITLE, &ctag.title.value.s_val, &ctag.title.len,
									MM_FILE_TAG_GENRE, &ctag.genre.value.s_val, &ctag.genre.len,
									MM_FILE_TAG_AUTHOR, &ctag.author.value.s_val, &ctag.author.len,
									MM_FILE_TAG_COPYRIGHT, &ctag.copyright.value.s_val, &ctag.copyright.len,
									MM_FILE_TAG_DATE, &ctag.date.value.s_val, &ctag.date.len,
									MM_FILE_TAG_RECDATE, &ctag.recdate.value.s_val, &ctag.recdate.len,
									MM_FILE_TAG_DESCRIPTION, &ctag.description.value.s_val, &ctag.description.len,
									MM_FILE_TAG_COMMENT, &ctag.comment.value.s_val, &ctag.comment.len,
									MM_FILE_TAG_ARTWORK, &ctag.artwork.value.p_val, &ctag.artwork.len,
									MM_FILE_TAG_ARTWORK_SIZE, &ctag.artwork_size.value.i_val,
									MM_FILE_TAG_ARTWORK_MIME, &ctag.artwork_mime.value.s_val, &ctag.artwork_mime.len,
									MM_FILE_TAG_TRACK_NUM, &ctag.track_num.value.s_val, &ctag.track_num.len,
									MM_FILE_TAG_CLASSIFICATION, &ctag.classfication.value.s_val, &ctag.classfication.len,
									MM_FILE_TAG_RATING, &ctag.rating.value.s_val, &ctag.rating.len,
									MM_FILE_TAG_LONGITUDE, &ctag.longitude.value.d_val,
									MM_FILE_TAG_LATIDUE, &ctag.latitude.value.d_val,
									MM_FILE_TAG_ALTIDUE, &ctag.altitude.value.d_val,
									MM_FILE_TAG_CONDUCTOR, &ctag.conductor.value.s_val, &ctag.conductor.len,
									MM_FILE_TAG_UNSYNCLYRICS, &ctag.unsynclyrics.value.s_val, &ctag.unsynclyrics.len,
									MM_FILE_TAG_SYNCLYRICS_NUM, &ctag.synclyrics_size.value.i_val,
									MM_FILE_TAG_ROTATE, &ctag.rotate.value.s_val, &ctag.rotate.len,
									NULL);
		if (ret != MM_ERROR_NONE &&  err_attr_name)
		{
			printf("failed to get %s attrs\n", err_attr_name);
			free(err_attr_name);
			err_attr_name = NULL;

			if (msg_tmp_fp) /* opened by MM_TIME_CHECK_START */
			{
				fclose (msg_tmp_fp);
				msg_tmp_fp = NULL;
			}

			return -1;
		}

		/* print tag information	 */
		printf ("[Tag] =================================== \n");
		printf("# artist: [%s]\n", ctag.artist.value.s_val);
		printf("# title: [%s]\n", ctag.title.value.s_val);
		printf("# album: [%s]\n", ctag.album.value.s_val);
		printf("# album_artist: [%s]\n", ctag.album_artist.value.s_val);
		printf("# genre: [%s]\n", ctag.genre.value.s_val);
		printf("# author: [%s]\n", ctag.author.value.s_val);
		printf("# copyright: [%s]\n", ctag.copyright.value.s_val);
		printf("# year: [%s]\n", ctag.date.value.s_val);
		printf("# recdate: [%s]\n", ctag.recdate.value.s_val);
		printf("# description: [%s]\n", ctag.description.value.s_val);
		printf("# comment: [%s]\n", ctag.comment.value.s_val);
		printf("# artwork: [%p]\n", ctag.artwork.value.p_val);
		printf("# artwork_size: [%d]\n", ctag.artwork_size.value.i_val);
		printf("# artwork_mime: [%s]\n", ctag.artwork_mime.value.s_val);
		printf("# track number: [%s]\n", ctag.track_num.value.s_val);
		printf("# classification: [%s]\n", ctag.classfication.value.s_val);
		printf("# rating: [%s]\n", ctag.rating.value.s_val);
		printf("# longitude: [%f]\n", ctag.longitude.value.d_val);
		printf("# latitude: [%f]\n", ctag.latitude.value.d_val);
		printf("# altitude: [%f]\n", ctag.altitude.value.d_val);
		printf("# conductor: [%s]\n", ctag.conductor.value.s_val);
		printf("# unsynclyrics_length: [%d]\n", ctag.unsynclyrics.len);
		printf("# unsynclyrics: [%s]\n", ctag.unsynclyrics.value.s_val);
		printf("# synclyrics size: [%d]\n", ctag.synclyrics_size.value.i_val);
		printf("# rotate: [%s]\n", ctag.rotate.value.s_val);

		if(ctag.synclyrics_size.value.i_val > 0) {
			int idx = 0;
			unsigned long time_info = 0;
			char * lyrics_info = NULL;

			printf("# synclyrics: \n");
			
			for(idx = 0; idx < ctag.synclyrics_size.value.i_val; idx++) {
				ret = mm_file_get_synclyrics_info(tag_attrs, idx, &time_info, &lyrics_info);
				if(ret == MM_ERROR_NONE) {
					printf("[%2d][%6ld][%s]\n", idx, time_info, lyrics_info);
				} else {
					printf("Error when get lyrics\n");
					break;
				}
			}
		}
		
		/* release tag */
		mm_file_destroy_tag_attrs(tag_attrs);
	} else {
		printf ("Failed to mm_file_create_tag_attrs() error=[%x]\n", ret);
	}


	printf ("=================================================\n\n");

	MM_TIME_CHECK_FINISH (filename);

    return 0;
}
