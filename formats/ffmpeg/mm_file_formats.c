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
#include <string.h>
#include <stdlib.h>

#ifdef DRM_SUPPORT
#include <drm_client.h>
#endif


#include "mm_debug.h"
#include "mm_file_format_private.h"
#include "mm_file_utils.h"

#define _MMF_FILE_FILEEXT_MAX 128

int (*MMFileOpenFunc[MM_FILE_FORMAT_NUM+1]) (MMFileFormatContext *fileContext) = {
	mmfile_format_open_ffmpg,	/* 3GP */
	mmfile_format_open_ffmpg,	/* ASF */
	mmfile_format_open_ffmpg,	/* AVI */
	mmfile_format_open_ffmpg,	/* MATROSAK */
	mmfile_format_open_ffmpg,	/* MP4 */
	mmfile_format_open_ffmpg,	/* OGG */
	NULL,						/* NUT */
	mmfile_format_open_ffmpg,						/* QT */
	NULL,						/* REAL */
	mmfile_format_open_amr,		/* AMR */
	mmfile_format_open_aac,		/* AAC */
	mmfile_format_open_mp3,		/* MP3 */
	NULL,						/* AIFF */
	NULL,						/* AU */
	mmfile_format_open_wav,		/* WAV */
	mmfile_format_open_mid,		/* MID */
	mmfile_format_open_mmf,		/* MMF */
	mmfile_format_open_ffmpg,	/* DIVX */
	mmfile_format_open_ffmpg,	/* FLV */
	NULL,						/* VOB */
	mmfile_format_open_imy,		/* IMY */
	mmfile_format_open_ffmpg,	/* WMA */
	mmfile_format_open_ffmpg,	/* WMV */
	NULL,						/* JPG */
	mmfile_format_open_ffmpg,	/* FLAC */
	NULL,
};

static int _CleanupFrameContext (MMFileFormatContext *formatContext, bool clean_all)
{
	if (formatContext) {

		if (formatContext->ReadStream)	formatContext->ReadStream	= NULL;
		if (formatContext->ReadFrame)		formatContext->ReadFrame	= NULL;
		if (formatContext->ReadTag)		formatContext->ReadTag		= NULL;
		if (formatContext->Close)			formatContext->Close			= NULL;

		if (formatContext->uriFileName)		mmfile_free(formatContext->uriFileName);
		if (formatContext->title)				mmfile_free(formatContext->title);
		if (formatContext->artist)				mmfile_free(formatContext->artist);
		if (formatContext->author)			mmfile_free(formatContext->author);
		if (formatContext->composer)			mmfile_free(formatContext->composer);
		if (formatContext->album)				mmfile_free(formatContext->album);
		if (formatContext->album_artist)		mmfile_free(formatContext->album_artist);
		if (formatContext->copyright)			mmfile_free(formatContext->copyright);
		if (formatContext->description)			mmfile_free(formatContext->description);
		if (formatContext->comment)			mmfile_free(formatContext->comment);
		if (formatContext->genre)				mmfile_free(formatContext->genre);
		if (formatContext->classification)		mmfile_free(formatContext->classification);
		if (formatContext->year)				mmfile_free(formatContext->year);
		if (formatContext->recDate) 			mmfile_free(formatContext->recDate);
		if (formatContext->tagTrackNum) 		mmfile_free(formatContext->tagTrackNum);
		if (formatContext->rating)				mmfile_free(formatContext->rating);
		if (formatContext->artworkMime)		mmfile_free(formatContext->artworkMime);
		if (formatContext->artwork)			mmfile_free(formatContext->artwork);
		if (formatContext->conductor)			mmfile_free(formatContext->conductor);
		if (formatContext->unsyncLyrics)		mmfile_free(formatContext->unsyncLyrics);
		if (formatContext->rotate)				mmfile_free(formatContext->rotate);

		if(clean_all)	//syncLyrics has to be freed in mm_file_destroy_tag_attrs() except abnormal status
			if (formatContext->syncLyrics)			mm_file_free_synclyrics_list(formatContext->syncLyrics);

		if (formatContext->privateFormatData)	mmfile_free(formatContext->privateFormatData);
		if (formatContext->privateCodecData)	mmfile_free(formatContext->privateCodecData);

		if (formatContext->nbStreams > 0) {
			int i = 0;

			/*formatContext->streams[0] is video, formatContext->streams[1] is audio.*/
			if (formatContext->streams[0]) mmfile_free(formatContext->streams[0]);
			if (formatContext->streams[1]) mmfile_free(formatContext->streams[1]);

			for (i = 2; (i < formatContext->nbStreams) && (i < MAXSTREAMS); i++) {
				if (formatContext->streams[i]) mmfile_free(formatContext->streams[i]);
			}
		}

		if (formatContext->thumbNail) {
			if (formatContext->thumbNail->frameData)
				mmfile_free (formatContext->thumbNail->frameData);

			if (formatContext->thumbNail->configData)
				mmfile_free (formatContext->thumbNail->configData);

			mmfile_free (formatContext->thumbNail);
		}

		formatContext->videoTotalTrackNum = 0;
		formatContext->audioTotalTrackNum = 0;
		formatContext->nbStreams = 0;
	}

	return MMFILE_FORMAT_SUCCESS;
}

static int
_PreprocessFile (MMFileSourceType *fileSrc, char **urifilename, int *formatEnum, int *isdrm)
{
	const char	*fileName = NULL;
	char		extansion_name[_MMF_FILE_FILEEXT_MAX];
	int			pos = 0;
	int			filename_len = 0;
	int			index = 0, skip_index = 0;

	if (fileSrc->type == MM_FILE_SRC_TYPE_FILE) {
		fileName = (const char *)(fileSrc->file.path);
		filename_len = strlen (fileName);
		pos = filename_len;

		/**
		 * Get file extension from file's name
		 */
		while (pos > 0) {
			pos--;
			if (fileName[pos] == '.')
				break;
		}

		memset (extansion_name, 0x00, _MMF_FILE_FILEEXT_MAX);

		/*extract metadata for all file. ex)a , a. , a.mp3*/
		if (pos == 0) {
			/*even though there is no file extension, extracto metadata*/
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ("no file extension");
			#endif
		}
		else if (_MMF_FILE_FILEEXT_MAX > (filename_len - pos - 1)) {
			strncpy (extansion_name, fileName + pos +1 , (filename_len - pos - 1));
			extansion_name[filename_len - pos - 1] = '\0';
		} else {
			debug_error ("invalid filename. destination length: %d, source length: %d.\n", _MMF_FILE_FILEEXT_MAX, (filename_len - pos - 1));
			return MMFILE_FORMAT_FAIL;		/*invalid file name*/
		}

#ifdef DRM_SUPPORT
		/**
		 * Make URI name with file name
		 */
		drm_bool_type_e res = DRM_TRUE;
		drm_file_type_e file_type = DRM_TYPE_UNDEFINED;
		int ret = 0;
		bool is_drm = FALSE;

		ret = drm_is_drm_file (fileSrc->file.path, &res);
		if (ret == DRM_RETURN_SUCCESS && DRM_TRUE == res)
		{
			ret = drm_get_file_type(fileSrc->file.path, &file_type);
			if((ret == DRM_RETURN_SUCCESS) && ((file_type == DRM_TYPE_OMA_V1) ||(file_type == DRM_TYPE_OMA_V2)))
			{
				is_drm = TRUE;
			}
		}

		if (is_drm)
		{
			*isdrm = MM_FILE_DRM_OMA;
			debug_error ("OMA DRM detected. Not Support DRM Content\n");
			return MMFILE_FORMAT_FAIL;		/*Not Support DRM Content*/
		} 
		else 
#endif // DRM_SUPPORT			
		{
			*isdrm = MM_FILE_DRM_NONE;
#ifdef __MMFILE_MMAP_MODE__
			*urifilename = mmfile_malloc (MMFILE_MMAP_URI_LEN + filename_len + 1);
			if (!*urifilename) {
				debug_error ("error: mmfile_malloc uriname\n");
				return MMFILE_FORMAT_FAIL;
			}

			memset (*urifilename, 0x00, MMFILE_MMAP_URI_LEN + filename_len + 1);
			strncpy (*urifilename, MMFILE_MMAP_URI, MMFILE_MMAP_URI_LEN);
			strncat (*urifilename, fileName, filename_len);
			(*urifilename)[MMFILE_MMAP_URI_LEN + filename_len] = '\0';

#else
			*urifilename = mmfile_malloc (MMFILE_FILE_URI_LEN + filename_len + 1);
			if (!*urifilename) {
				debug_error ("error: mmfile_malloc uriname\n");
				return MMFILE_FORMAT_FAIL;
			}

			memset (*urifilename, 0x00, MMFILE_FILE_URI_LEN + filename_len + 1);
			strncpy (*urifilename, MMFILE_FILE_URI, MMFILE_FILE_URI_LEN+1);
			strncat (*urifilename, fileName, filename_len);
			(*urifilename)[MMFILE_FILE_URI_LEN + filename_len] = '\0';
#endif
		}

		///////////////////////////////////////////////////////////////////////
		//                 Check File format                                 //
		///////////////////////////////////////////////////////////////////////

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("Get codec type of [%s].\n", extansion_name);
		#endif

		if (strcasecmp (extansion_name, "mp4") == 0   ||
			strcasecmp (extansion_name, "mpeg4") == 0 ||
			strcasecmp (extansion_name, "m4a") == 0   ||
			strcasecmp (extansion_name, "mpg") == 0   ||
			strcasecmp (extansion_name, "mpg4") == 0  ||
			strcasecmp (extansion_name, "m4v") == 0 ) {

			if (MMFileFormatIsValidMP4 (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_MP4;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_MP4;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "3gp") == 0) {
			if (MMFileFormatIsValidMP4 (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_3GP;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_3GP;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "amr") == 0 ||
				   strcasecmp (extansion_name, "awb") == 0) {

			if (MMFileFormatIsValidAMR (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_AMR;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_AMR;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "wav") == 0) {
			if (MMFileFormatIsValidWAV (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_WAV;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_WAV;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "mid")  == 0 ||
				   strcasecmp (extansion_name, "midi") == 0 ||
				   strcasecmp (extansion_name, "spm")  == 0 ) {

			if (MMFileFormatIsValidMID (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_MID;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_MID;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "mp3") == 0) {
			if (MMFileFormatIsValidMP3 (*urifilename,5)) {
				*formatEnum = MM_FILE_FORMAT_MP3;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_MP3;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "aac") == 0) {
			if (MMFileFormatIsValidAAC (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_AAC;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_AAC;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "xmf") == 0 ||
				 strcasecmp (extansion_name, "mxmf") == 0) {
			if (MMFileFormatIsValidMID (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_MID;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_MID;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (!strcasecmp (extansion_name, "mmf") ||
				   !strcasecmp (extansion_name, "ma2")) {
			if (MMFileFormatIsValidMMF (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_MMF;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_MMF;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "imy") == 0) {
			if (MMFileFormatIsValidIMY (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_IMELODY;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_IMELODY;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "avi") == 0) {
			if (MMFileFormatIsValidAVI (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_AVI;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_AVI;
			goto PROBE_PROPER_FILE_TYPE;
		} else if (strcasecmp (extansion_name, "divx") == 0) {
			if (MMFileFormatIsValidAVI (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_DIVX;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_DIVX;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "asf") == 0 ||
				   strcasecmp (extansion_name, "asx") == 0 ) {
			if (MMFileFormatIsValidASF (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_ASF;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_ASF;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "wma") == 0) {
			if (MMFileFormatIsValidWMA (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_WMA;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_WMA;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "wmv") == 0) {
			if (MMFileFormatIsValidWMV (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_WMV;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_WMV;
			goto PROBE_PROPER_FILE_TYPE;

		} else if (strcasecmp (extansion_name, "ogg") == 0) {
			if (MMFileFormatIsValidOGG (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_OGG;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_OGG;
			goto PROBE_PROPER_FILE_TYPE;
		} else if (strcasecmp (extansion_name, "mkv") == 0 ||
				   strcasecmp (extansion_name, "mka") == 0) {
			if (MMFileFormatIsValidMatroska (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_MATROSKA;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_MATROSKA;
			goto PROBE_PROPER_FILE_TYPE;
		} else if (strcasecmp (extansion_name, "mov") == 0) {
			if (MMFileFormatIsValidMP4 (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_QT;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_QT;
			goto PROBE_PROPER_FILE_TYPE;
		} else if (strcasecmp (extansion_name, "flac") == 0) {
			if (MMFileFormatIsValidFLAC (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_FLAC;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_FLAC;
			goto PROBE_PROPER_FILE_TYPE;
		} else if (strcasecmp (extansion_name, "flv") == 0) {
			if (MMFileFormatIsValidFLV (*urifilename)) {
				*formatEnum = MM_FILE_FORMAT_FLV;
				return MMFILE_FORMAT_SUCCESS;
			}
			skip_index = MM_FILE_FORMAT_FLV;
			goto PROBE_PROPER_FILE_TYPE;
		} else {
			debug_warning ("probe file type=%s\n", fileName);
			skip_index = -1;
			goto PROBE_PROPER_FILE_TYPE;
		}
	} else if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) {
		char tempURIBuffer[MMFILE_URI_MAX_LEN] = {0,};

		sprintf (tempURIBuffer, "%s%u:%u", MMFILE_MEM_URI, (unsigned int)fileSrc->memory.ptr, fileSrc->memory.size);
		*urifilename = mmfile_strdup (tempURIBuffer);
		if (!*urifilename) {
			debug_error ("error: uri is NULL\n");
			return MMFILE_FORMAT_FAIL;
		}

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("uri: %s\n", *urifilename);
		#endif

		switch (fileSrc->memory.format) {
			case MM_FILE_FORMAT_3GP: {
				if (MMFileFormatIsValidMP4 (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_3GP;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_3GP;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_MP4: {
				if (MMFileFormatIsValidMP4 (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_MP4;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_MP4;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_AMR: {
				if (MMFileFormatIsValidAMR (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_AMR;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_AMR;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_WAV: {
				if (MMFileFormatIsValidWAV (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_WAV;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_WAV;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_MID: {
				if (MMFileFormatIsValidMID (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_MID;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_MID;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_MP3: {
				if (MMFileFormatIsValidMP3 (*urifilename,5)) {
					*formatEnum = MM_FILE_FORMAT_MP3;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_MP3;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_AAC: {
				if (MMFileFormatIsValidAAC (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_AAC;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_AAC;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_MMF: {
				if (MMFileFormatIsValidMMF (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_MMF;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_MMF;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_IMELODY: {
				if (MMFileFormatIsValidIMY (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_IMELODY;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_IMELODY;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_AVI: {
				if (MMFileFormatIsValidAVI (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_AVI;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_AVI;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_DIVX: {
				if (MMFileFormatIsValidAVI (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_DIVX;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_DIVX;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_ASF: {
				if (MMFileFormatIsValidASF (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_ASF;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_ASF;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_WMA: {
				if (MMFileFormatIsValidWMA (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_WMA;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_WMA;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_WMV: {
				if (MMFileFormatIsValidWMV (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_WMV;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_WMV;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}

			case MM_FILE_FORMAT_OGG: {
				if (MMFileFormatIsValidOGG (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_OGG;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_OGG;
				goto PROBE_PROPER_FILE_TYPE;
			}

			case MM_FILE_FORMAT_MATROSKA: {
				if (MMFileFormatIsValidMatroska (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_MATROSKA;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_MATROSKA;
				goto PROBE_PROPER_FILE_TYPE;
			}

			case MM_FILE_FORMAT_QT: {
				if (MMFileFormatIsValidMP4 (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_QT;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_QT;
				goto PROBE_PROPER_FILE_TYPE;
			}

			case MM_FILE_FORMAT_FLAC: {
				if (MMFileFormatIsValidFLAC (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_FLAC;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_FLAC;
				goto PROBE_PROPER_FILE_TYPE;
			}

			case MM_FILE_FORMAT_FLV: {
				if (MMFileFormatIsValidFLV (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_FLV;
					return MMFILE_FORMAT_SUCCESS;
				}
				skip_index = MM_FILE_FORMAT_FLV;
				goto PROBE_PROPER_FILE_TYPE;
			}

			default: {
				debug_warning ("probe fileformat type=%d (%d: autoscan)\n", fileSrc->memory.format, MM_FILE_FORMAT_INVALID);
				skip_index = -1;
				goto PROBE_PROPER_FILE_TYPE;
				break;
			}
		}
	} else {
		debug_error ("error: invaild input type[memory|file]\n");
		return MMFILE_FORMAT_FAIL;
	}

PROBE_PROPER_FILE_TYPE:
	for (index = 0; index < MM_FILE_FORMAT_NUM; index++) {
		if (index == skip_index)
			continue;

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("search index = [%d]\n", index);
		#endif

		switch (index) {
			case MM_FILE_FORMAT_QT:
			case MM_FILE_FORMAT_3GP:
			case MM_FILE_FORMAT_MP4: {
				if (skip_index == MM_FILE_FORMAT_QT || skip_index == MM_FILE_FORMAT_3GP || skip_index == MM_FILE_FORMAT_MP4)
					break;

				if (MMFileFormatIsValidMP4 (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_3GP;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_3GP;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_ASF:
			case MM_FILE_FORMAT_WMA:
			case MM_FILE_FORMAT_WMV: {
				if (skip_index == MM_FILE_FORMAT_ASF || skip_index == MM_FILE_FORMAT_WMA || skip_index == MM_FILE_FORMAT_WMV)
					break;

				if (MMFileFormatIsValidASF (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_ASF;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_ASF;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_DIVX:
			case MM_FILE_FORMAT_AVI: {
				if (skip_index == MM_FILE_FORMAT_DIVX || skip_index == MM_FILE_FORMAT_AVI)
					break;

				if (MMFileFormatIsValidAVI(*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_AVI;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_AVI;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_OGG: {
				if (MMFileFormatIsValidOGG (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_OGG;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_OGG;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_AMR: {
				if (MMFileFormatIsValidAMR (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_AMR;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_AMR;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_AAC: {
				if (MMFileFormatIsValidAAC (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_AAC;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_AAC;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_MP3: {
				if (MMFileFormatIsValidMP3 (*urifilename,50)) {
					*formatEnum = MM_FILE_FORMAT_MP3;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_MP3;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_WAV: {
				if (MMFileFormatIsValidWAV (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_WAV;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_WAV;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_MID: {
				if (MMFileFormatIsValidMID (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_MID;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_MID;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_MMF: {
				if (MMFileFormatIsValidMMF (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_MMF;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_MMF;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_IMELODY: {
				if (MMFileFormatIsValidIMY (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_IMELODY;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_IMELODY;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_MATROSKA: {
				if (MMFileFormatIsValidMatroska (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_MATROSKA;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_MATROSKA;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_FLAC: {
				if (MMFileFormatIsValidFLAC (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_FLAC;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_FLAC;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

			case MM_FILE_FORMAT_FLV: {
				if (MMFileFormatIsValidFLV (*urifilename)) {
					*formatEnum = MM_FILE_FORMAT_FLV;
					if (fileSrc->type == MM_FILE_SRC_TYPE_MEMORY) fileSrc->memory.format = MM_FILE_FORMAT_FLV;
					return MMFILE_FORMAT_SUCCESS;
				}
				break;
			}

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

	if (index == MM_FILE_FORMAT_NUM)
		debug_error("Can't probe file type\n");

	*formatEnum = -1;
	return MMFILE_FORMAT_FAIL;

}

static int _mmfile_format_close (MMFileFormatContext *formatContext, bool clean_all)
{
	if (NULL == formatContext) {
		debug_error ("error: invalid params\n");
		return MMFILE_FORMAT_FAIL;
	}

	if (formatContext->Close) {
		formatContext->Close(formatContext);
		formatContext->Close = NULL;
	}

	_CleanupFrameContext(formatContext, clean_all);

	if (formatContext)
		mmfile_free (formatContext);

	return MMFILE_FORMAT_SUCCESS;
}


EXPORT_API
int mmfile_format_open (MMFileFormatContext **formatContext, MMFileSourceType *fileSrc)
{
	int index = 0;
	int ret = 0;
	MMFileFormatContext *formatObject = NULL;

	if (NULL == fileSrc) {
		debug_error ("error: invalid params\n");
		return MMFILE_FORMAT_FAIL;
	}

	/* create formatContext object */
	formatObject = mmfile_malloc (sizeof (MMFileFormatContext));
	if (NULL == formatObject) {
		debug_error ("error: mmfile_malloc fail for formatObject\n");
		*formatContext = NULL;
		return MMFILE_FORMAT_FAIL;
	}

	memset (formatObject, 0x00, sizeof (MMFileFormatContext));

	mmfile_register_io_all();

	/* parsing file extension */
	formatObject->filesrc = fileSrc;

	formatObject->pre_checked = 0;	/*not yet format checked.*/

	/**
	 * Format detect and validation check.
	 */
	ret = _PreprocessFile (fileSrc, &formatObject->uriFileName,  &formatObject->formatType, &formatObject->isdrm);
	if (MMFILE_FORMAT_SUCCESS != ret) {
		debug_error ("error: _PreprocessFile fail\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	formatObject->pre_checked = 1;	/*already file format checked.*/

	/**
	 * Open format function.
	 */
	if (NULL == MMFileOpenFunc[formatObject->formatType]) {
		debug_error ("error: Not implemented \n");
		ret = MMFILE_FORMAT_FAIL;
		goto find_valid_handler;
	}

	ret = MMFileOpenFunc[formatObject->formatType] (formatObject);
	if (MMFILE_FORMAT_FAIL == ret) {
		debug_error ("error: Try other formats\n");
		ret = MMFILE_FORMAT_FAIL;
		goto find_valid_handler;
	}

	*formatContext = formatObject;
	return MMFILE_FORMAT_SUCCESS;

find_valid_handler:
	formatObject->pre_checked = 0;	/*do check file format*/

	for (index = 0; index < MM_FILE_FORMAT_NUM+1; index++) {
		if (NULL == MMFileOpenFunc[index]) {
			debug_error ("error: Not implemented \n");
			ret = MMFILE_FORMAT_FAIL;
			continue;
		}

		if (formatObject->formatType == index)
			continue;

		ret = MMFileOpenFunc[index] (formatObject);
		if (MMFILE_FORMAT_FAIL == ret) {
//			_CleanupFrameContext (formatObject, true);
			continue;
		}

		break;
	}

	formatObject->formatType = index;

	if (index == MM_FILE_FORMAT_NUM + 1 && MMFILE_FORMAT_FAIL == ret) {
		debug_error ("can't find file format handler\n");
		_CleanupFrameContext (formatObject, true);
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	formatObject->formatType = index;
	*formatContext = formatObject;

	return MMFILE_FORMAT_SUCCESS;

exception:
	_mmfile_format_close (formatObject, true);
	*formatContext = NULL;

	return ret;
}

EXPORT_API
int mmfile_format_read_stream (MMFileFormatContext *formatContext)
{
	if (NULL == formatContext || NULL == formatContext->ReadStream) {
		debug_error ("error: invalid params\n");
		return MMFILE_FORMAT_FAIL;
	}

	return formatContext->ReadStream (formatContext);
}

EXPORT_API
int mmfile_format_read_frame (MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame)
{
	if (NULL == formatContext || NULL == frame || NULL == formatContext->ReadFrame ) {
		debug_error ("error: invalid params\n");
		return MMFILE_FORMAT_FAIL;
	}

	return formatContext->ReadFrame (formatContext, timestamp, frame);
}

EXPORT_API
int mmfile_format_read_tag (MMFileFormatContext *formatContext)
{
	if (NULL == formatContext || NULL == formatContext->ReadTag) {
		debug_error ("error: invalid params\n");
		return MMFILE_FORMAT_FAIL;
	}

	return formatContext->ReadTag (formatContext);
}


EXPORT_API
int mmfile_format_close (MMFileFormatContext *formatContext)
{
	return _mmfile_format_close(formatContext, false);
}
