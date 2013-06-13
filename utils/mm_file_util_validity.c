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
#include <mm_debug.h>
#include "mm_file_utils.h"

/* Description of return value
 * 0: false
 * 1: true
 */

/***********************************************************************/
/*                     Internal functions                              */
/***********************************************************************/
static int _MMFileSearchID3Tag (MMFileIOHandle *fp, unsigned int *offset);
static int _MMFileIsMP3Header  (void *header);
static int _MMFileIsOGGHeader  (void *header);
static int _MMFileIsMP4Header  (void *header);
static int _MMFileIsWAVHeader  (void *header);
static int _MMFileIsAVIHeader  (void *header);
static int _MMFileIsMIDHeader  (void *header);
static int _MMFileIsMMFHeader  (void *header);
static int _MMFileIsIMYHeader  (void *header);
static int _MMFileIsASFHeader  (void *header);
static int _MMFileIsAMRHeader  (void *header);
static int _MMFileIsFLACHeader  (void *header);
static int _MMFileIsFLVHeader  (void *header);



/***********************************************************************/
/*                     MP3 Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidMP3 (const char *mmfileuri, int frameCnt)
{
#define _MMFILE_MP3_HEADER_LENGTH   4
#define _MMFILE_MP3_BUFFER_LENGTH   8200

	MMFileIOHandle *fp = NULL;
	unsigned char *buffer=NULL;
	long long  filesize = 0;
	unsigned int  sizeID3 = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	unsigned int  endoffset = 0;
	unsigned int  i = 0, j = 0;
	int ret = 0,frameSize,count,offset;

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if(ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	mmfile_seek (fp, 0L, MMFILE_SEEK_END);
	filesize = mmfile_tell (fp);
	mmfile_seek (fp, 0L, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_MP3_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	/* Search the existance of ID3 tag */
	ret = _MMFileSearchID3Tag (fp, &sizeID3);
	if (ret == 0) {
		debug_error("Error in searching the ID3 tag\n");
		goto exit;
	}

	ret = 0;

	/* set begin and end point at the file */
	startoffset += sizeID3;
	endoffset = startoffset + 102400;
	if(endoffset > filesize - _MMFILE_MP3_HEADER_LENGTH)
		endoffset = filesize - _MMFILE_MP3_HEADER_LENGTH;
	
	/* find sync bit */
	i = startoffset;
	count = 0;
	
	buffer =  mmfile_malloc (_MMFILE_MP3_BUFFER_LENGTH * sizeof(char));

	if (NULL == buffer) {
		debug_error("Error in allocating memory for MP3 buffer\n");
		goto exit;
	}

	while (i < endoffset) {
		mmfile_seek (fp, i, MMFILE_SEEK_SET);
		readed = mmfile_read (fp, buffer, _MMFILE_MP3_BUFFER_LENGTH);
		if (readed < _MMFILE_MP3_HEADER_LENGTH) {
			debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
			ret = 0;
			break;
		}

		offset = 1;
		for (j = 0; (j <= readed - _MMFILE_MP3_HEADER_LENGTH); j = j + offset) {
			frameSize = _MMFileIsMP3Header (buffer+j);

			offset = 1;
			
			if (frameSize) {
				
				if((j+frameSize) >= (endoffset-(i+_MMFILE_MP3_HEADER_LENGTH))) {
					goto failMP3;
				}

				if((j+frameSize) >= (readed - _MMFILE_MP3_HEADER_LENGTH)) {
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "MP3 coner hit %d %d\n", j, frameSize);
					#endif
					break;
				}

				frameSize = _MMFileIsMP3Header (buffer+j+frameSize);

				if(frameSize) {
					offset = frameSize;
					count++;
					if(count == frameCnt) {
						ret = 1;
						#ifdef __MMFILE_TEST_MODE__
						debug_msg ( "Header Detected at %d\n", i+j);
						#endif
						goto exit;
					}
				} else {
					offset = 1;
				}
			}
		}

		/*If j is zero, this loop is infinite */
		if (j ==0) j++;

		i = i + j;
	}

failMP3:
	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "Header Not Detected at: %d\n",i+j);
	#endif
exit:

	if(buffer) {
		mmfile_free (buffer);
	}

	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}



/***********************************************************************/
/*                     AAC Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidAAC (const char *mmfileuri)
{
#define _MMFILE_AAC_HEADER_LENGTH   4
#define _MMFILE_AAC_BUFFER_LENGTH   8200

	MMFileIOHandle *fp = NULL;
	unsigned char *buffer=NULL;
	unsigned int  sizeID3 = 0;
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	unsigned int  endoffset = 0;
	unsigned int  i = 0, j = 0;
	int ret = 0;
	unsigned int sync,frameSize;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_AAC_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	/* Search the existance of ID3 tag */
	ret = _MMFileSearchID3Tag (fp, &sizeID3);
	if (ret == 0) {
		debug_error("Error in searching the ID3 tag\n");
		goto exit;
	}

	ret = 0;

	/* set begin and end point at the file */
	startoffset += sizeID3;
	endoffset = startoffset + 10240;
	if(endoffset > filesize - _MMFILE_AAC_HEADER_LENGTH)
		endoffset = filesize - _MMFILE_AAC_HEADER_LENGTH;
	
	buffer = mmfile_malloc (_MMFILE_AAC_BUFFER_LENGTH * sizeof(char));

	if (NULL == buffer) {
		debug_error("Error in allocating memory for AAC buffer\n");
		goto exit;
	}
	
	i = startoffset;

	while (i < endoffset) {
		mmfile_seek (fp, i, MMFILE_SEEK_SET);

		readed = mmfile_read (fp, buffer, _MMFILE_AAC_BUFFER_LENGTH);
	
		if (readed < _MMFILE_AAC_HEADER_LENGTH) {
			debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
			ret = 0;
			break;
		}

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "read error. size = %d. i = %d\n", readed,i);
		#endif
		for(j=0; (j < readed - _MMFILE_AAC_HEADER_LENGTH); j++) {

			sync = ((buffer[j]<<8)|(buffer[j+1]));

			if ( (sync & 0xFFF6) == 0xFFF0) {
				frameSize = (((buffer[j+3]&0x03)<<11) | (buffer[j+4]<<3) | ((buffer[j+5]&0xE0)>>5));

				if(frameSize == 0 ) {
					continue; 
				}

				if((j+frameSize) >= (endoffset-(i+2))) {
					goto fail;
				}
				if((j+frameSize) >= (readed - 2)) {
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "AAC coner hit %d %d\n", j, frameSize);
					#endif
					break;
				}

				sync = ( (buffer[j+frameSize]<<8) | (buffer[j+frameSize+1]));

				if ((sync & 0xFFF6) == 0xFFF0) {
					ret = 1;
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "AAC ADTS Header Detected at %d\n", i+j);
					#endif
					goto exit;
				}
			} else if (!memcmp((buffer+j),"ADIF",4)) {
				ret = 1;
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ( "AAC ADIF Header Detected at %d\n", i+j);
				#endif
				goto exit;
			}
		}
		/*If j is zero, this loop is infinite */
		if (j ==0) j++;

		i = i + j;
	}


fail:
	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "Header Detected Failed\n");
	#endif

exit:
	if (buffer) {
		mmfile_free (buffer);
	}

	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}



/***********************************************************************/
/*                     OGG Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidOGG (const char *mmfileuri)
{
#define _MMFILE_OGG_HEADER_LENGTH   4
#define _MMFILE_OGG_BUFFER_LENGTH   512
#define _MMFILE_OGG_CHECK_LIMIT		(_MMFILE_OGG_HEADER_LENGTH * 1000)

	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_OGG_BUFFER_LENGTH] = {0,};
	unsigned int  sizeID3 = 0;
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	unsigned int  endoffset = 0;
	unsigned int  i = 0, j = 0;
	int ret = 0;
	unsigned int check_limit = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		ret = 0;
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		ret = 0;
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_OGG_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	/* Search the existance of ID3 tag */
	ret = _MMFileSearchID3Tag (fp, &sizeID3);
	if(ret == 0) {
		debug_error("Error in searching the ID3 tag\n");
		goto exit;
	}

	ret = 0;

	/* set begin and end point at the file */
	startoffset += sizeID3;
	endoffset = filesize - _MMFILE_OGG_HEADER_LENGTH;

	check_limit = (endoffset > _MMFILE_OGG_CHECK_LIMIT) ? _MMFILE_OGG_CHECK_LIMIT : endoffset;

	i = startoffset;
	while (i <= check_limit) {
		mmfile_seek (fp, i, MMFILE_SEEK_SET);
		readed = mmfile_read (fp, buffer, _MMFILE_OGG_BUFFER_LENGTH);
		if (readed < _MMFILE_OGG_HEADER_LENGTH) {
			debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
			ret = 0;
			break;
		}

		for (j = 0; (j <= readed - _MMFILE_OGG_HEADER_LENGTH); j++) {
			if (1 == _MMFileIsOGGHeader (buffer+j)) {
				ret = 1;
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ( "Header Detected at %d\n", i+j);
				#endif
				goto exit;
			}
		}

		memset (buffer, 0x00, _MMFILE_OGG_BUFFER_LENGTH);

		i = i + j;
	}

exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}



/***********************************************************************/
/*                     MIDI Header Check API                           */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidMID (const char *mmfileuri)
{
#define _MMFILE_MIDI_HEADER_LENGTH 4
#define _MMFILE_MIDI_BUFFER_LENGTH 512
#define _MMFILE_MIDI_CHECK_LIMIT	(_MMFILE_MIDI_HEADER_LENGTH * 1024)

	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_MIDI_BUFFER_LENGTH] = {0,};
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	unsigned int  endoffset = 0;
	unsigned int  i = 0, j = 0;
	int ret = 0;
	unsigned int check_limit = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_MIDI_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;

	/* set begin and end point at the file */
	startoffset = 0;
	endoffset = filesize - _MMFILE_MIDI_HEADER_LENGTH;

	check_limit = (endoffset > _MMFILE_MIDI_CHECK_LIMIT) ? _MMFILE_MIDI_CHECK_LIMIT : endoffset;

	i = startoffset;
	while (i <= check_limit) {
		mmfile_seek (fp, i, MMFILE_SEEK_SET);
		readed = mmfile_read (fp, buffer, _MMFILE_MIDI_BUFFER_LENGTH);
		if (readed < _MMFILE_MIDI_HEADER_LENGTH) {
			debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
			ret = 0;
			break;
		}

		for (j = 0; (j <= readed - _MMFILE_MIDI_HEADER_LENGTH); j++) {
			if (1 == _MMFileIsMIDHeader (buffer+j)) {
				ret = 1;
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ( "Header Detected at %d\n", i+j);
				#endif
				goto exit;
			}
		}

		memset (buffer, 0x00, _MMFILE_MIDI_BUFFER_LENGTH);

		i = i + j;
	}

exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}


/***********************************************************************/
/*                     WAV Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidWAV (const char *mmfileuri)
{
#define _MMFILE_WAV_HEADER_LENGTH 15

	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_WAV_HEADER_LENGTH] = {0,};
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	 /* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_WAV_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;
	
	/* set begin and end point at the file */
	startoffset = 0;
	
	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);

	readed = mmfile_read (fp, buffer, _MMFILE_WAV_HEADER_LENGTH);
	if (_MMFILE_WAV_HEADER_LENGTH != readed) {
		debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	if (1 == _MMFileIsWAVHeader (buffer)) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Header Detected\n");
		#endif
		ret = 1;
		goto exit;
	}


exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}



/***********************************************************************/
/*                     MP4 Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidMP4 (const char *mmfileuri)
{
#define _MMFILE_MP4_HEADER_LENGTH 4
#define _MMFILE_MP4_CHECK_LIMIT		(1024*10)	/*10Kbyte*/
	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_MP4_HEADER_LENGTH] = {0,};
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;
	unsigned int check_limit = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_MP4_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;

	/**@note weak check*/
	check_limit = (filesize > _MMFILE_MP4_CHECK_LIMIT) ? _MMFILE_MP4_CHECK_LIMIT: filesize;
	for (startoffset = 0; check_limit - (startoffset + _MMFILE_MP4_HEADER_LENGTH) > 0; startoffset++) {
		mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);

		readed = mmfile_read (fp, buffer, _MMFILE_MP4_HEADER_LENGTH);
		if (readed != _MMFILE_MP4_HEADER_LENGTH) {
			debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
			ret = 0;
			goto exit;
		}

		/*input is 4byte*/
		if (1 == _MMFileIsMP4Header (buffer)) {
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ("MP4 Header Detected\n");
			#endif
			ret = 1;
			goto exit;
		}
	}

exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}


/***********************************************************************/
/*                     AVI Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidAVI (const char *mmfileuri)
{
#define _MMFILE_AVI_HEADER_LENGTH 12

	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_AVI_HEADER_LENGTH] = {0,};
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	 /* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_AVI_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;        
	startoffset = 0;

	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);    

	readed = mmfile_read (fp, buffer, _MMFILE_AVI_HEADER_LENGTH);
	if (_MMFILE_AVI_HEADER_LENGTH != readed) {
		debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	if (1 == _MMFileIsAVIHeader (buffer)) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Header Detected \n");
		#endif
		ret = 1;
		goto exit;
	}

exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}



/***********************************************************************/
/*                     ASF Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidASF (const char *mmfileuri)
{
#define _MMFILE_ASF_HEADER_LENGTH 16
	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_ASF_HEADER_LENGTH] = {0,};
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_ASF_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;
	startoffset = 0;

	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);        

	readed = mmfile_read (fp, buffer, _MMFILE_ASF_HEADER_LENGTH);
	if (_MMFILE_ASF_HEADER_LENGTH != readed) {
		debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	if (1 == _MMFileIsASFHeader (buffer)) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Header Detected\n");
		#endif
		ret = 1;
		goto exit;
	}

exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}

/***********************************************************************/
/*                     WMA Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidWMA (const char *mmfileuri)
{
#define _MMFILE_ASF_HEADER_LENGTH 16
	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_ASF_HEADER_LENGTH] = {0,};
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_ASF_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;
	startoffset = 0;

	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);        

	readed = mmfile_read (fp, buffer, _MMFILE_ASF_HEADER_LENGTH);
	if (_MMFILE_ASF_HEADER_LENGTH != readed) {
		debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	if (1 == _MMFileIsASFHeader (buffer)) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Header Detected\n");
		#endif
		ret = 1;
		goto exit;
	}

exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}



/***********************************************************************/
/*                     WMV Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidWMV (const char *mmfileuri)
{
 #define _MMFILE_ASF_HEADER_LENGTH 16
	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_ASF_HEADER_LENGTH] = {0,};
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_ASF_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;
	startoffset = 0;

	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);        

	readed = mmfile_read (fp, buffer, _MMFILE_ASF_HEADER_LENGTH);
	if (_MMFILE_ASF_HEADER_LENGTH != readed) {
		debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	if (1 == _MMFileIsASFHeader (buffer)) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Header Detected\n");
		#endif
		ret = 1;
		goto exit;
	}

exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}


/***********************************************************************/
/*                     MMF Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidMMF (const char *mmfileuri)
{
#define _MMFILE_MMF_HEADER_LENGTH 18

	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_MMF_HEADER_LENGTH] = {0,};
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_MMF_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;
	startoffset = 0;

	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);    

	readed = mmfile_read (fp, buffer, _MMFILE_MMF_HEADER_LENGTH);
	if (_MMFILE_MMF_HEADER_LENGTH != readed) {
		debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	if (1 == _MMFileIsMMFHeader (buffer)) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Header Detected\n");
		#endif
		ret = 1;
		goto exit;
	}

exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}



/***********************************************************************/
/*                     MMF Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidIMY (const char *mmfileuri)
{
#define _MMFILE_IMY_HEADER_LENGTH 13

	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_IMY_HEADER_LENGTH] = {0,};
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	 /* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_IMY_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;
	startoffset = 0;

	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);    

	readed = mmfile_read (fp, buffer, _MMFILE_IMY_HEADER_LENGTH);
	if (_MMFILE_IMY_HEADER_LENGTH != readed) {
		debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	if (1 == _MMFileIsIMYHeader (buffer)) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Header Detected\n");
		#endif
		ret = 1;
		goto exit;
	}

exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}



/***********************************************************************/
/*                     AMR Header Check API                            */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidAMR (const char *mmfileuri)
{
#define _MMFILE_AMR_MAX_HEADER_SIZE 15
#define _MMFILE_AMR_MIN_HEADER_SIZE 6

	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_AMR_MAX_HEADER_SIZE] = {0,};
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_AMR_MAX_HEADER_SIZE) {
		debug_error ("header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;
	startoffset = 0;

	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);        

	readed = mmfile_read (fp, buffer, _MMFILE_AMR_MAX_HEADER_SIZE);
	if (_MMFILE_AMR_MAX_HEADER_SIZE != readed) {
		debug_error ("read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	if (1 == _MMFileIsAMRHeader (buffer)) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Header Detected\n");
		#endif
		ret = 1;
		goto exit;
	}

exit:
	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}

/***********************************************************************/
/*                     Matroska Header Check API                       */
/***********************************************************************/

EXPORT_API
int MMFileFormatIsValidMatroska (const char *mmfileuri)
{
#define _MMFILE_EBML_MARKER_LENGTH	4

	MMFileIOHandle *fp = NULL;
	unsigned char* buffer = NULL;
	long long     filesize = 0;
	int           readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	int len_mask = 0x80, size = 1, n = 1, total = 0;
	char probe_data[] = { 'm', 'a', 't', 'r', 'o', 's', 'k', 'a' };

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	 /* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	ret = 0;

	/* set begin and end point at the file */
	startoffset = 0;

	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);

	buffer = mmfile_malloc (_MMFILE_EBML_MARKER_LENGTH * sizeof(char));
	readed = mmfile_read (fp, buffer, _MMFILE_EBML_MARKER_LENGTH);
	if (_MMFILE_EBML_MARKER_LENGTH != readed) {
		debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	/* ebml header? */
	  if (buffer[0] != 0x1A || buffer[1] != 0x45 || buffer[2] != 0xDF || buffer[3] != 0xA3) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("This is not a EBML format\n");
		#endif
	    ret = 0;
	    goto exit;
	  }

	  /* length of header */
	    mmfile_read (fp, (unsigned char*)(&total), 1);
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("Initial total header size = [0x%x]\n", total);
		#endif

	    while (size <= 8 && !(total & len_mask)) {
	      debug_error ("This case can not be handled yet....")
	      size++;
	      len_mask >>= 1;
	    }
	    if (size > 8) {
	    	debug_error ("This case can not be handled yet....")
	    	ret = 0;
	    	goto exit;
	    }
	    total &= (len_mask - 1);
	    while (n < size) {
	      debug_error ("This case can not be handled yet....")
	      ret = 0;
	      goto exit;
	    }

		#ifdef __MMFILE_TEST_MODE__
	    debug_msg ("Final total header size = [%d]\n", total);
		#endif

	    if (buffer)
	    	mmfile_free (buffer);
	    buffer = mmfile_malloc (total * sizeof(char));
	    mmfile_read (fp, buffer, total);

	    for (n = 0; n <= total - sizeof (probe_data); n++) {
	      if (!memcmp (&buffer[n], probe_data, sizeof (probe_data))) {
			#ifdef __MMFILE_TEST_MODE__
				debug_msg ("String matroska found!!!\n");
			#endif
			ret = 1;
			goto exit;
	      }
	    }

exit:
	if (buffer) {
		mmfile_free (buffer);
	}

	if (fp) {
		mmfile_close (fp);
	}

	return ret;
}

/***********************************************************************/
/*                     QT Header Check API                       */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidQT (const char *mmfileuri)
{
	return 1;
}

/***********************************************************************/
/*                     Flac Header Check API                       */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidFLAC (const char *mmfileuri)
{
#define _MMFILE_FLAC_HEADER_LENGTH 5	/*fLaC*/

	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_FLAC_HEADER_LENGTH] = {0,};
	long long	  filesize = 0;
	int 		  readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_FLAC_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;

	/* set begin and end point at the file */
	startoffset = 0;

	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);

	readed = mmfile_read (fp, buffer, _MMFILE_FLAC_HEADER_LENGTH);
	if (_MMFILE_FLAC_HEADER_LENGTH != readed) {
		debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	if (1 == _MMFileIsFLACHeader (buffer)) {
#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Header Detected\n");
#endif
		ret = 1;
		goto exit;
	}


exit:
	if (fp) {
		mmfile_close (fp);
	}

return ret;
}

/***********************************************************************/
/*                     FLV(flash video) Header Check API                       */
/***********************************************************************/
EXPORT_API
int MMFileFormatIsValidFLV (const char *mmfileuri)
{
#define _MMFILE_FLV_HEADER_LENGTH 4	/*FLV*/

	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_FLV_HEADER_LENGTH] = {0,};
	long long	  filesize = 0;
	int 		  readed = 0;
	unsigned int  startoffset = 0;
	int ret = 0;

	if (NULL == mmfileuri) {
		debug_error ("file source is NULL\n");
		return ret;
	}

	ret = mmfile_open (&fp, mmfileuri, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	/* Initialize the members of handle */
	mmfile_seek (fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek (fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_FLV_HEADER_LENGTH) {
		debug_error ( "header is too small.\n");
		ret = 0;
		goto exit;
	}

	ret = 0;

	/* set begin and end point at the file */
	startoffset = 0;

	mmfile_seek (fp, startoffset, MMFILE_SEEK_SET);

	readed = mmfile_read (fp, buffer, _MMFILE_FLV_HEADER_LENGTH);
	if (_MMFILE_FLV_HEADER_LENGTH != readed) {
		debug_error ( "read error. size = %d. Maybe end of file.\n", readed);
		ret = 0;
		goto exit;
	}

	if (1 == _MMFileIsFLVHeader (buffer)) {
#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Header Detected\n");
#endif
		ret = 1;
		goto exit;
	}


exit:
	if (fp) {
		mmfile_close (fp);
	}

return ret;
}

/***********************************************************************/
/*            Implementation of Internal Functions                     */
/***********************************************************************/
static int _MMFileIsASFHeader  (void *header)
{
	/* ID: 30 26 B2 75 8E 66 CF 11 A6 D9 00 AA 00 62 CE 6C */    
	unsigned char *s = header;

	if ( (*(s +  0) == 0x30) &&
		 (*(s +  1) == 0x26) &&
		 (*(s +  2) == 0xB2) &&
		 (*(s +  3) == 0x75) &&
		 (*(s +  4) == 0x8E) &&
		 (*(s +  5) == 0x66) &&
		 (*(s +  6) == 0xCF) &&
		 (*(s +  7) == 0x11) &&
		 (*(s +  8) == 0xA6) &&
		 (*(s +  9) == 0xD9) &&
		 (*(s + 10) == 0x00) &&
		 (*(s + 11) == 0xAA) &&
		 (*(s + 12) == 0x00) &&
		 (*(s + 13) == 0x62) &&
		 (*(s + 14) == 0xCE) &&
		 (*(s + 15) == 0x6C) ) {

		return 1;
	}

	return 0;
}

static int _MMFileIsAMRHeader  (void *header)
{
#define _MMFILE_AMR_SINGLE_CH_HEADER_SIZE       6
#define _MMFILE_AMR_SINGLE_CH_HEADER            "#!AMR\n"

#define _MMFILE_AMR_WB_SINGLE_CH_HEADER_SIZE    9
#define _MMFILE_AMR_WB_SINGLE_CH_HEADER         "#!AMR-WB\n"

#define _MMFILE_AMR_MULTI_CH_HEADER_SIZE        12
#define _MMFILE_AMR_MULTI_CH_HEADER             "#!AMR_MC1.0\n"

#define _MMFILE_AMR_WB_MULTI_CH_HEADER_SIZE     15
#define _MMFILE_AMR_WB_MULTI_CH_HEADER          "#!AMR-WB_MC1.0\n"

	unsigned char *s = header;

	if (!memcmp (s, _MMFILE_AMR_SINGLE_CH_HEADER,    _MMFILE_AMR_SINGLE_CH_HEADER_SIZE) ||        
		!memcmp (s, _MMFILE_AMR_WB_SINGLE_CH_HEADER, _MMFILE_AMR_WB_SINGLE_CH_HEADER_SIZE) ||
		!memcmp (s, _MMFILE_AMR_MULTI_CH_HEADER,     _MMFILE_AMR_MULTI_CH_HEADER_SIZE) ||
		!memcmp (s, _MMFILE_AMR_WB_MULTI_CH_HEADER,  _MMFILE_AMR_WB_MULTI_CH_HEADER_SIZE)) {

		return 1;
	}

	return 0;
}

static int _MMFileIsIMYHeader  (void *header)
{
	unsigned char *s = header;

	if (!memcmp(s, "BEGIN:IMELODY", 13)) {
		return 1;
	}

	return 0;
}

static int _MMFileIsMIDHeader (void *header)
{
	unsigned char *s = header;

	if (!memcmp(s, "MThd", 4)) {	/*general MIDI*/
		return 1;
	} else if (!memcmp (s, "XMF_", 4)) {	/*XMF*/
		return 1;
	} else if (!memcmp(s, "IREZ", 4)) {
		return 1;	/*RMF format*/
	}

	return 0;
}

static int _MMFileIsMMFHeader (void *header)
{
#define _MMFILE_MMF_TYPE_POSITION    ((char)0x11)
	unsigned char *s = header;

	if(!memcmp (s, "MMMD", 4)) {
		/* warning: comparison is always true due to limited range of data type */
		if( *(s + _MMFILE_MMF_TYPE_POSITION) <= 0x2F ) {
			return 1;
		} else if (((*(s + _MMFILE_MMF_TYPE_POSITION) >= 0x30) && (*(s + _MMFILE_MMF_TYPE_POSITION) <= 0x38)) // MA3, MA5 type
				 || ((*(s + _MMFILE_MMF_TYPE_POSITION) >= 0x40) && (*(s + _MMFILE_MMF_TYPE_POSITION) <= 0x48))
				 || ((*(s + _MMFILE_MMF_TYPE_POSITION) >= 0x50) && (*(s + _MMFILE_MMF_TYPE_POSITION) <= 0x58))
				 || ((*(s + _MMFILE_MMF_TYPE_POSITION) == 0xF0))) {

			return 1;
		}
	}

	return 0;
}


static int _MMFileIsAVIHeader (void *header)
{
	unsigned char *s = header;

	if (!memcmp(s, "RIFF", 4) && !memcmp(s+8, "AVI", 3)) {
		return 1;
	}

	return 0;
}


static int _MMFileIsWAVHeader (void *header)
{
	unsigned char *s = header;

	if (!memcmp(s, "RIFF", 4) && !memcmp(s+8, "WAVE", 4)) {
		return 1;
	}

	return 0;
}



static int _MMFileIsMP4Header (void *header)
{
	unsigned char *s = header;

	if (!memcmp (s, "moov", 4) ||
		!memcmp (s, "mdat", 4) ||
		!memcmp (s, "ftyp", 4) ||
		!memcmp (s, "free", 4) ||
		!memcmp (s, "uuid", 4) ||
		!memcmp (s, "skip", 4) ||
		
		!memcmp (s, "PICT", 4) ||
		!memcmp (s, "wide", 4) ||
		!memcmp (s, "prfl", 4)) {
		return 1;
	}

	return 0;
}

static int _MMFileIsOGGHeader (void *header)
{
	unsigned char *s = header;

	if (!memcmp(s, "OggS", 4)) {
		return 1;
	}

	return 0;
}

static int _MMFileIsMP3Header (void *header)
{
	unsigned long head = 0;
	unsigned char *headc = header;
	unsigned int bitrate, layer, length, mode;
	unsigned int coef,samplerate, version, channels;
	static const unsigned int mp3types_bitrates[2][3][16] =	{
		{	{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448,},
			{0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384,},
			{0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320,}},
		{	{0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256,},
			{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160,},
			{0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160,}},
	};

	static const unsigned int mp3types_freqs[3][3] = {
		{11025, 12000, 8000},
		{22050, 24000, 16000},
		{44100, 48000, 32000}
	};

	static const unsigned int mp3FrameCoef[4][4] = {{-1,48,144,72},{-1,-1,-1,-1},{-1,48,144,72},{-1,48,144,144}};

	/* header */
	head = (*(headc+0)<<24 | *(headc+1)<<16 | *(headc+2)<<8 | *(headc+3));

	if ((head & 0xffe00000) != 0xffe00000) {
		 return 0;
	}

	/* we don't need extension, copyright, original or
	* emphasis for the frame length */
	head >>= 6;

	/* mode */
	mode = head & 0x3;
	head >>= 3;

	/* padding */
	length = head & 0x1;
	head >>= 1;

	/* sampling frequency */
	samplerate = head & 0x3;
	if (samplerate == 3) {
		return 0;
	}
	head >>= 2;

	/* bitrate index */
	bitrate = head & 0xF;
 
	if (bitrate == 15) {
		return 0;
	}

	/* ignore error correction, too */
	head >>= 5;

	/* layer */
	layer = 4 - (head & 0x3);
	if (layer == 4) {
		return 0;
	}
	head >>= 2;

	/* version 0=MPEG2.5; 2=MPEG2; 3=MPEG1 */
	version = head & 0x3;
	if (version == 1) {
		return 0;
	}

	/* lookup */
	channels = (mode == 3) ? 1 : 2;
	samplerate = mp3types_freqs[version > 0 ? version - 1 : 0][samplerate];
	{
		/* calculating */
		bitrate = mp3types_bitrates[version == 3 ? 0 : 1][layer - 1][bitrate];
		coef = mp3FrameCoef[version][layer];
		if(layer == 1)
			length = length*4;

		length = length + ((coef*bitrate*1000)/samplerate);
	}

	return length;
}

static int _MMFileSearchID3Tag (MMFileIOHandle *fp, unsigned int *offset)
{
#define _MMFILE_MP3_TAGV2_HEADER_LEN 10
#define _MMFILE_GET_INT_NUMBER(buff) (int)( (((int)(buff)[0]) << 24) | (((int)(buff)[1]) << 16) | (((int)(buff)[2]) << 8) | (((int)(buff)[3])))

	unsigned char tagHeader[_MMFILE_MP3_TAGV2_HEADER_LEN] = {0,};
	unsigned int tagInfoSize = 0;
	unsigned int acc_tagsize = 0;
	int tagVersion = 0;
	int encSize = 0;
	int readed = 0;

	/*init offset*/
	*offset = 0;

	mmfile_seek(fp, 0, MMFILE_SEEK_SET);
	
_START_TAG_SEARCH:

	readed = mmfile_read (fp, tagHeader, _MMFILE_MP3_TAGV2_HEADER_LEN);
	if (readed != _MMFILE_MP3_TAGV2_HEADER_LEN) {
		debug_error ("read error occured.\n");
		return 0;
	}

	if (memcmp (tagHeader, "ID3", 3) == 0) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("'ID3' found.\n");
		#endif
	} else {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("'ID3' not found.\n");
		#endif
		goto search_end;
	}

	/**@note weak id3v2 tag checking*/
	if (tagHeader[3] != 0xFF && tagHeader[4] != 0xFF &&
		(tagHeader[6] & 0x80) == 0 && (tagHeader[7] & 0x80) == 0 &&
		(tagHeader[8] & 0x80) == 0 && (tagHeader[9] & 0x80) == 0) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("good ID3V2 tag.\n");
		#endif
	} else {
		debug_warning ("It's bad ID3V2 tag.\n");
		goto search_end;
	}

	tagVersion = tagHeader[3];

	if (tagVersion > 4) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg("Tag version not supported\n");
		#endif
		goto search_end;
	}

	encSize = _MMFILE_GET_INT_NUMBER(&tagHeader[6]);
	tagInfoSize = _MMFILE_MP3_TAGV2_HEADER_LEN;
	tagInfoSize += (((encSize & 0x0000007F) >> 0) | ((encSize & 0x00007F00) >> 1) | ((encSize & 0x007F0000) >> 2) | ((encSize & 0x7F000000) >> 3));

	/**@note unfortunately, some contents has many id3 tag.*/
	acc_tagsize += tagInfoSize;
	#ifdef __MMFILE_TEST_MODE__
	debug_msg("tag size: %u, offset: %u\n", tagInfoSize, acc_tagsize);
	#endif

	mmfile_seek(fp, acc_tagsize, MMFILE_SEEK_SET);
	*offset = acc_tagsize;
	goto _START_TAG_SEARCH;

search_end:
	return 1;
}

static int _MMFileIsFLACHeader  (void *header)
{
	unsigned char *s = header;

	if (!memcmp(s, "fLaC", 4)) {
		return 1;
	}

	return 0;
}

static int _MMFileIsFLVHeader  (void *header)
{
	unsigned char *s = header;

	if (!memcmp(s, "FLV", 3)) {
		return 1;
	}

	return 0;
}
