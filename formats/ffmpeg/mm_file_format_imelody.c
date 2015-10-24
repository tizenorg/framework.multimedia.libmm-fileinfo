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

#include "mm_file_debug.h"
#include "mm_file_utils.h"
#include "mm_file_format_private.h"
#include "mm_file_format_imelody.h"
#include "mm_file_format_midi.h"

/**
 * Define
 */
#define AV_MIDI_COUNT_MAX		1600
#define AV_MIDI_NOTE_MAX		256
#define AV_MIDI_VOL_MAX			250
#define MIDI_LIMIT				127
#define MIDI_MAX				255

#define VOL_INTERVAL			12

#define MIDI_HEADER_LENGTH		52

static unsigned char midiData[AV_MIDI_COUNT_MAX] ;
static unsigned char midiHeader[MIDI_HEADER_LENGTH] = {0x4d, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x30, 0x4d, 0x54,
                                                       0x72, 0x6b, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x58, 0x04, 0x04, 0x02, 0x18, 0x08, 0x00, 0xff,
                                                       0x59, 0x02, 0x00, 0x00, 0x00, 0xff, 0x51, 0x03, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0xb0,
                                                       0x07, 0x00, 0x00, 0x90
                                                      };


static unsigned char noteTotal[AV_MIDI_COUNT_MAX];
static char octave[AV_MIDI_NOTE_MAX];
static int durationSpec[AV_MIDI_NOTE_MAX];
static int restSpec[AV_MIDI_NOTE_MAX];

static 	struct {
	char flat_sharp;
	char note;
	char duration;
	char duration_specifier;
	char rest;
	char rest_specifier;
	char vol;
} Melody[AV_MIDI_NOTE_MAX];

static	struct {
	int note;
	int duration_on;
	int duration_off;
} noteData[AV_MIDI_NOTE_MAX];

/*imelody key string (to validatation check)*/
static const char *g_imy_key_str[] = {
	"BEGIN:IMELODY",
	"VERSION:",
	"FORMAT:",
	"MELODY:",
	"END:IMELODY",
};


static int __is_good_imelody(unsigned char *src, unsigned int size);
static unsigned char *__get_load_memory(char *src, int *out_size);
static unsigned char __AvMIDISetVolume(char *pMelodyBuf);
static int __AvMIDISetBeat(char *pMelodyBuf);
static char __AvMIDISetStyle(char *pMelodyBuf);
static unsigned char *__AvConvertIMelody2MIDI(char *pMelodyBuf, unsigned int *pBufLen);
static int __get_imelody_tag(const char *uriname, tMMFileImelodyTagInfo *tags);


/* interface functions */
int mmfile_format_read_stream_imy(MMFileFormatContext *formatContext);
int mmfile_format_read_frame_imy(MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int mmfile_format_read_tag_imy(MMFileFormatContext *formatContext);
int mmfile_format_close_imy(MMFileFormatContext *formatContext);


EXPORT_API
int mmfile_format_open_imy(MMFileFormatContext *formatContext)
{
	int ret = 0;

	if (!formatContext) {
		debug_error("formatContext is NULL\n");
		return MMFILE_FORMAT_FAIL;
	}

	if (formatContext->pre_checked == 0) {
		ret = MMFileFormatIsValidIMY(NULL, formatContext->uriFileName);
		if (ret == 0) {
			debug_error("It is not imelody file\n");
			return MMFILE_FORMAT_FAIL;
		}
	}

	formatContext->ReadStream   = mmfile_format_read_stream_imy;
	formatContext->ReadFrame    = mmfile_format_read_frame_imy;
	formatContext->ReadTag      = mmfile_format_read_tag_imy;
	formatContext->Close        = mmfile_format_close_imy;

	formatContext->videoTotalTrackNum = 0;
	formatContext->audioTotalTrackNum = 1;

	formatContext->privateFormatData = NULL;

	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_read_stream_imy(MMFileFormatContext *formatContext)
{
	MIDI_INFO_SIMPLE *info = NULL;
	MMFileFormatStream  *audioStream = NULL;

	unsigned char	*imy = NULL;
	int				imy_size = 0;
	unsigned char	*midi = NULL;
	unsigned int	midi_size = 0;
	char			src2[MMFILE_URI_MAX_LEN];

	int ret = 0;

	if (!formatContext) {
		debug_error("formatContext is NULL\n");
		return MMFILE_FORMAT_FAIL;
	}

	/*convert iMelody to Midi*/
	imy = __get_load_memory(formatContext->uriFileName, &imy_size);
	if (!imy) {
		debug_error("failed to load memory.\n");
		goto exception;
	}
	ret = __is_good_imelody(imy, imy_size);
	if (ret != MMFILE_FORMAT_SUCCESS) {
		debug_error("it's broken file.\n");
		goto exception;
	}
	midi = __AvConvertIMelody2MIDI((char *)imy, &midi_size);
	if (!midi) {
		debug_error("failed to convert.");
		goto exception;
	}

	/*make uri*/
	memset(src2, 0x00, MMFILE_URI_MAX_LEN);
	snprintf(src2, sizeof(src2), "%s%u:%u", MMFILE_MEM_URI, (unsigned int)midi, midi_size);

	/*get infomation*/
	info = mmfile_format_get_midi_infomation(src2);
	if (!info) {
		debug_error("failed to get infomation");
		goto exception;
	}

	formatContext->duration = info->duration;
	formatContext->videoStreamId = -1;
	formatContext->videoTotalTrackNum = 0;
	formatContext->audioTotalTrackNum = info->track_num;
	formatContext->nbStreams = 1;


	audioStream = mmfile_malloc(sizeof(MMFileFormatStream));
	if (NULL == audioStream) {
		debug_error("error: mmfile_malloc audiostream\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	audioStream->streamType = MMFILE_AUDIO_STREAM;
	audioStream->codecId = MM_AUDIO_CODEC_IMELODY;
	audioStream->bitRate = 0;
	audioStream->framePerSec = 0;
	audioStream->width = 0;
	audioStream->height = 0;
	audioStream->nbChannel = 1;
	audioStream->samplePerSec = 0;
	formatContext->streams[MMFILE_AUDIO_STREAM] = audioStream;

#ifdef  __MMFILE_TEST_MODE__
	mmfile_format_print_contents(formatContext);
#endif

	mmfile_free(imy);
	mmfile_free(midi);
	mmfile_format_free_midi_infomation(info);
	return MMFILE_FORMAT_SUCCESS;

exception:
	mmfile_free(imy);
	mmfile_free(midi);
	mmfile_format_free_midi_infomation(info);
	mmfile_free(audioStream);
	return MMFILE_FORMAT_FAIL;
}

EXPORT_API
int mmfile_format_read_frame_imy(MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame)
{
	return MMFILE_FORMAT_SUCCESS;
}


EXPORT_API
int mmfile_format_read_tag_imy(MMFileFormatContext *formatContext)
{
	tMMFileImelodyTagInfo taginfo = {0, };
	unsigned int tag_len;
	unsigned int cnv_len;
	const char *locale = MMFileUtilGetLocale(NULL);

	if (!formatContext) {
		debug_error("formatContext is NULL\n");
		return MMFILE_FORMAT_FAIL;
	}

	__get_imelody_tag(formatContext->uriFileName, &taginfo);

	/**
	 * UTF8 convert
	 */
	if (taginfo.title) {
		tag_len = strlen(taginfo.title);
		cnv_len = 0;
		formatContext->title = mmfile_string_convert((const char *)taginfo.title,
		                                             tag_len,
		                                             "UTF-8",
		                                             locale,
		                                             NULL,
		                                             (unsigned int *)&cnv_len);

		if (formatContext->title == NULL) {
			debug_warning("failed to UTF8 convert.\n");
			formatContext->title = mmfile_strdup(taginfo.title);
		}
		mmfile_free(taginfo.title);
	}

	if (taginfo.composer) {
		tag_len = strlen(taginfo.composer);
		cnv_len = 0;
		formatContext->composer = mmfile_string_convert((const char *)taginfo.composer,
		                                                tag_len,
		                                                "UTF-8",
		                                                locale,
		                                                NULL,
		                                                (unsigned int *)&cnv_len);
		if (formatContext->composer == NULL) {
			debug_warning("failed to UTF8 convert.\n");
			formatContext->composer = mmfile_strdup(taginfo.composer);
		}
		mmfile_free(taginfo.composer);
	}

	if (taginfo.comment) {
		tag_len = strlen(taginfo.comment);
		cnv_len = 0;
		formatContext->comment = mmfile_string_convert((const char *)taginfo.comment,
		                                               tag_len,
		                                               "UTF-8",
		                                               locale,
		                                               NULL,
		                                               (unsigned int *)&cnv_len);
		if (formatContext->comment == NULL) {
			debug_warning("failed to UTF8 convert.\n");
			formatContext->comment = mmfile_strdup(taginfo.comment);
		}
		mmfile_free(taginfo.comment);
	}

	if (taginfo.copyright) {
		tag_len = strlen(taginfo.copyright);
		cnv_len = 0;
		formatContext->copyright = mmfile_string_convert((const char *)taginfo.copyright,
		                                                 tag_len,
		                                                 "UTF-8",
		                                                 locale,
		                                                 NULL,
		                                                 (unsigned int *)&cnv_len);
		if (formatContext->copyright == NULL) {
			debug_warning("failed to UTF8 convert.\n");
			formatContext->copyright = mmfile_strdup(taginfo.copyright);
		}
		mmfile_free(taginfo.copyright);
	}

	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_close_imy(MMFileFormatContext *formatContext)
{
	return MMFILE_FORMAT_SUCCESS;
}

static int __get_imelody_tag(const char *uriname, tMMFileImelodyTagInfo *tags)
{
#define _MMFILE_IMY_TAG_BUFFER_LENGTH   512
#define _MMFILE_IMY_HEADER_LENGTH       20
#define _MMFILE_IMY_KEY_BUFFER_LENGTH   20
#define _MMFILE_IMY_VALUE_BUFFER_LENGTH 128

	MMFileIOHandle *fp = NULL;
	unsigned char buffer[_MMFILE_IMY_TAG_BUFFER_LENGTH] = {0, };
	long long     filesize = 0;
	unsigned int  startoffset = 0;
	unsigned int  endoffset = 0, i = 0;
	int  readed = 0, j = 0;
	char          imy_key_buffer[_MMFILE_IMY_KEY_BUFFER_LENGTH] = {0, };
	unsigned int  imy_key_buffer_index = 0;
	char          imy_value_buffer[_MMFILE_IMY_VALUE_BUFFER_LENGTH] = {0, };
	unsigned int  imy_value_buffer_index = 0;
	int           isKeyBuffer = 1;
	int           isDone  = 0;

	int ret = MMFILE_FORMAT_FAIL;

	if (!uriname || !tags) {
		debug_error("uriname or tags is NULL\n");
		return MMFILE_FORMAT_FAIL;
	}

	ret = mmfile_open(&fp, uriname, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error("open failed.\n");
		return MMFILE_FORMAT_FAIL;
	}

	mmfile_seek(fp, 0, MMFILE_SEEK_END);
	filesize = mmfile_tell(fp);
	mmfile_seek(fp, 0, MMFILE_SEEK_SET);

	if (filesize < _MMFILE_IMY_HEADER_LENGTH) {
		debug_error("header is too small.\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exit;
	}

	/* set begin and end point at the file */
	startoffset = 0;
	endoffset = filesize;

	i = startoffset;
	isKeyBuffer = 1;
	while (i < endoffset) {
		mmfile_seek(fp, i, MMFILE_SEEK_SET);
		readed = mmfile_read(fp, buffer, _MMFILE_IMY_TAG_BUFFER_LENGTH);
		if (readed < 0) {
			debug_error("read error. size = %d. Maybe end of file.\n", readed);
			ret = 0;
			break;
		}

		j = 0;
		while (j < readed) {
			if (*(buffer + j) == 0x3a) {
				isKeyBuffer = 0;
			} else if (*(buffer + j) == 0x0d) {
			} else if (*(buffer + j) == 0x0a) {
				isKeyBuffer = 1;
				isDone = 1;
			} else {
				if (isKeyBuffer) {
					if (imy_key_buffer_index < _MMFILE_IMY_KEY_BUFFER_LENGTH) {
						imy_key_buffer[imy_key_buffer_index++] = *(buffer + j);
					}

				} else {
					if (imy_value_buffer_index < _MMFILE_IMY_VALUE_BUFFER_LENGTH) {
						imy_value_buffer[imy_value_buffer_index++] = *(buffer + j);
					}
				}
			}

			if (isDone) {
				if (!strncmp(imy_key_buffer, "NAME", 4)) {
					if (tags->title != NULL)
						mmfile_free(tags->title);
					tags->title = mmfile_strdup(imy_value_buffer);
				} else if (!strncmp(imy_key_buffer, "COMPOSER", 8)) {
					if (tags->composer != NULL)
						mmfile_free(tags->composer);
					tags->composer = mmfile_strdup(imy_value_buffer);
				} else if (!strncmp(imy_key_buffer, "COPYRIGHT", 9)) {
					if (tags->copyright != NULL)
						mmfile_free(tags->copyright);
					tags->copyright = mmfile_strdup(imy_value_buffer);
				}

				memset(imy_key_buffer, 0x00, _MMFILE_IMY_KEY_BUFFER_LENGTH);
				memset(imy_value_buffer, 0x00, _MMFILE_IMY_VALUE_BUFFER_LENGTH);
				imy_key_buffer_index = 0;
				imy_value_buffer_index = 0;
				isDone = 0;
			}

			j++;
		}

		memset(buffer, 0x00, _MMFILE_IMY_TAG_BUFFER_LENGTH);

		i = i + j;
	}

exit:
	mmfile_close(fp);
	return ret;
}

static unsigned char *
__get_load_memory(char *src, int *out_size)
{
	unsigned char			*buf = NULL;
	MMFileIOHandle	*fp = NULL;
	long long		src_size = 0L;
	int				readed = 0;
	int				ret = 0;

	/*open*/
	ret = mmfile_open(&fp, src, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error("open failed.\n");
		return NULL;
	}

	/*get file size*/
	mmfile_seek(fp, 0L, MMFILE_SEEK_END);
	src_size = mmfile_tell(fp);
	mmfile_seek(fp, 0L, MMFILE_SEEK_SET);

	if (src_size <= 0) {
		debug_error("failed to get file size.\n");
		goto failed;
	}

	/*alloc read buffer*/
	if ((buf = mmfile_malloc(src_size)) == NULL) {
		debug_error("memory allocation failed.\n");
		goto failed;
	}

	/*read data*/
	if ((readed = mmfile_read(fp, buf, src_size)) != src_size) {
		debug_error("read error. size = %d\n", readed);
		goto failed;
	}

	*out_size = (int)src_size;
	mmfile_close(fp);

	return buf;

failed:
	if (buf) mmfile_free(buf);
	if (fp) mmfile_close(fp);
	return NULL;
}


static int
__is_good_imelody(unsigned char *src, unsigned int size)
{
	unsigned int i, j;
	int key_len;
	int is_found;
	unsigned char *p;
	unsigned int num = sizeof(g_imy_key_str) / sizeof(g_imy_key_str[0]);

	for (i = 0; i < num; i++) {
		key_len = strlen(g_imy_key_str[i]);
		p = src;
		is_found = 0;
		for (j = 0; j <= size - key_len; j++, p++) {
			if (memcmp(g_imy_key_str[i], p, key_len) == 0) {
				is_found = 1;
				break;
			}
		}
		if (is_found) continue;
		else return MMFILE_FORMAT_FAIL;
	}

	return MMFILE_FORMAT_SUCCESS;
}


static unsigned char *
__AvConvertIMelody2MIDI(char *pMelodyBuf, unsigned int *pBufLen)
{
	unsigned char *pConvertBuf;
	char *pStart;
	char *pMelodyStart;
	char noteBase[6];
	int octaveCount;
	int octaveValue;
	int count;
	int noteCount = 0;
	int MelodyCount;
	int number;
	int numberCount;
	char style = '0';
	int tempoData[3];
	int tempoValue;
	int trackSize;
	int repeat = 0;
	int repeatCount = 0;
	char vol = '%';
	int volInterval = 0;

	for (count = 0; count < AV_MIDI_NOTE_MAX; count++) {
		restSpec[count] = 0;
		durationSpec[count] = 0;
	}

	for (octaveCount = 0; octaveCount < AV_MIDI_NOTE_MAX; octaveCount++)
		octave[octaveCount] = '%';

	for (MelodyCount = 0; MelodyCount < AV_MIDI_NOTE_MAX; MelodyCount++) {
		Melody[MelodyCount].flat_sharp = '%';
		Melody[MelodyCount].note = '%';
		Melody[MelodyCount].duration = '%';
		Melody[MelodyCount].duration_specifier = '%';
		Melody[MelodyCount].rest = '%';
		Melody[MelodyCount].rest_specifier = '%';
		Melody[MelodyCount].vol = '%';
	}

	for (MelodyCount = 0; MelodyCount < AV_MIDI_NOTE_MAX; MelodyCount++) {
		noteData[MelodyCount].note = 0;
		noteData[MelodyCount].duration_on = 0;
		noteData[MelodyCount].duration_off = 0;
	}


	memset(midiData, 0, sizeof(midiData));
	memcpy(midiData, midiHeader, MIDI_HEADER_LENGTH);

	pStart = pMelodyBuf;

	midiData[49] = __AvMIDISetVolume(pMelodyBuf);

	pMelodyBuf = pStart;

	tempoValue = __AvMIDISetBeat(pMelodyBuf);

	for (number = 0; tempoValue != 0; number++) {
		tempoData[0] = tempoValue % 16;
		tempoValue = tempoValue / 16;

		tempoData[1] = tempoValue % 16;
		tempoValue = tempoValue / 16;

		tempoData[2] = tempoData[0] + tempoData[1] * 16;

		midiData[42 - number] = tempoData[2];
	}

	pMelodyBuf = pStart;

	while (!(*pMelodyBuf == '@' || (*pMelodyBuf == 'E' && *(pMelodyBuf + 2) == 'D')))
		pMelodyBuf++;

	pMelodyBuf++;

	if (*pMelodyBuf >= '1' && *pMelodyBuf <= '9') {
		repeat = *pMelodyBuf - '0';
		pMelodyBuf++;

	}

	repeat = 0;

	pMelodyBuf = pStart;

	pMelodyBuf = pMelodyBuf + 42;

	while (!(*pMelodyBuf == 'M' && *(pMelodyBuf + 5) == 'Y' && *(pMelodyBuf + 6) == ':')) /*2007-02-28 AVMS_Sound:k2bogus - UMTS200073205;imy play, [MELODY:] extract fix */
		pMelodyBuf++;

	pMelodyBuf = pMelodyBuf + 6;

	pMelodyStart = pMelodyBuf;

	/**@note if newline detected, stop reading
	 *       why? mobileBAE player stopped at newline.
	 *       2009/08/12
	 */
	while (!((*pMelodyBuf == 'E' && *(pMelodyBuf + 2) == 'D') || (*pMelodyBuf == '\n'))) {
		if (noteCount >= AV_MIDI_NOTE_MAX) {
			debug_warning("__AvConvertIMelody2MIDI : noteCount>=AV_MIDI_NOTE_MAX\n");
			break;
		}

		pMelodyBuf++;

		if (*pMelodyBuf == '*') {
			pMelodyBuf++;

			if (*pMelodyBuf >= '0' && *pMelodyBuf <= '8')
				octave[noteCount] = *pMelodyBuf;
		}

		if (*pMelodyBuf == '#' || *pMelodyBuf == '&')
			Melody[noteCount].flat_sharp = *pMelodyBuf;

		if (*pMelodyBuf == 'r') {
			pMelodyBuf++;

			if (*pMelodyBuf >= '0' && *pMelodyBuf <= '5') {
				Melody[noteCount].rest = *pMelodyBuf;
				pMelodyBuf++;

				if (*pMelodyBuf == '.' || *pMelodyBuf == ':' || *pMelodyBuf == ';')
					Melody[noteCount].rest_specifier = *pMelodyBuf;
			}
		}

		if (*pMelodyBuf == 'V') {
			pMelodyBuf++;

			if (*pMelodyBuf == '+' || *pMelodyBuf == '-')
				Melody[noteCount].vol = *pMelodyBuf;
		}

		if (*pMelodyBuf >= 'a' && *pMelodyBuf <= 'g') {
			Melody[noteCount].note = *pMelodyBuf;
			pMelodyBuf++;

			if (*pMelodyBuf >= '0' && *pMelodyBuf <= '5') {
				Melody[noteCount].duration = *pMelodyBuf;
				pMelodyBuf++;

				if (*pMelodyBuf == '.' || *pMelodyBuf == ':' || *pMelodyBuf == ';')
					Melody[noteCount].duration_specifier = *pMelodyBuf;

				else
					pMelodyBuf--;

				noteCount++;
			}
		}
	}

	for (octaveCount = 1; octaveCount < noteCount; octaveCount++) {
		if (octave[octaveCount] == '%')
			octave[octaveCount] = octave[octaveCount - 1];
	}

	for (number = 0; number < noteCount; number++) {
		if (octave[0] == '%' && octave[number] == '%')
			octaveValue = 4;

		else
			octaveValue = octave[number] - '0';

		octaveValue = octaveValue * 12;

		if (Melody[number].flat_sharp == '#') {
			switch (Melody[number].note) {
				case 'c':
					noteData[number].note = octaveValue + 1;
					break;
				case 'd':
					noteData[number].note = octaveValue + 3;
					break;
				case 'f':
					noteData[number].note = octaveValue + 6;
					break;
				case 'g':
					noteData[number].note = octaveValue + 8;
					break;
				case 'a':
					noteData[number].note = octaveValue + 10;
					break;
				default:
					break;
			}
		}

		else if (Melody[number].flat_sharp == '&') {
			switch (Melody[number].note) {
				case 'd':
					noteData[number].note = octaveValue + 1;
					break;
				case 'e':
					noteData[number].note = octaveValue + 3;
					break;
				case 'g':
					noteData[number].note = octaveValue + 6;
					break;
				case 'a':
					noteData[number].note = octaveValue + 8;
					break;
				case 'b':
					noteData[number].note = octaveValue + 10;
					break;
				default:
					break;
			}
		}

		else {
			switch (Melody[number].note) {
				case 'c':
					noteData[number].note = octaveValue;
					break;
				case 'd':
					noteData[number].note = octaveValue + 2;
					break;
				case 'e':
					noteData[number].note = octaveValue + 4;
					break;
				case 'f':
					noteData[number].note = octaveValue + 5;
					break;
				case 'g':
					noteData[number].note = octaveValue + 7;
					break;
				case 'a':
					noteData[number].note = octaveValue + 9;
					break;
				case 'b':
					noteData[number].note = octaveValue + 11;
					break;
				default:
					break;
			}
		}
	}

	pMelodyBuf = pMelodyStart;

	style = __AvMIDISetStyle(pMelodyBuf);

	for (number = 0; number < noteCount; number++) {
		if (style == '0') {
			switch (Melody[number].duration) {
				case '0':
					noteData[number].duration_on = 183;
					noteData[number].duration_off = 9;
					break;
				case '1':
					noteData[number].duration_on = 91;
					noteData[number].duration_off = 5;
					break;
				case '2':
					noteData[number].duration_on = 46;
					noteData[number].duration_off = 2;
					break;
				case '3':
					noteData[number].duration_on = 23;
					noteData[number].duration_off = 1;
					break;
				case '4':
					noteData[number].duration_on = 11;
					noteData[number].duration_off = 1;
					break;
				case '5':
					noteData[number].duration_on = 5;
					noteData[number].duration_off = 1;
					break;
				default:
					break;
			}
		}

		else if (style == '1') {
			switch (Melody[number].duration) {
				case '0':
					noteData[number].duration_on = 192;
					noteData[number].duration_off = 0;
					break;
				case '1':
					noteData[number].duration_on = 96;
					noteData[number].duration_off = 0;
					break;
				case '2':
					noteData[number].duration_on = 48;
					noteData[number].duration_off = 0;
					break;
				case '3':
					noteData[number].duration_on = 24;
					noteData[number].duration_off = 0;
					break;
				case '4':
					noteData[number].duration_on = 12;
					noteData[number].duration_off = 0;
					break;
				case '5':
					noteData[number].duration_on = 6;
					noteData[number].duration_off = 0;
					break;
				default:
					break;
			}
		}

		else {
			switch (Melody[number].duration) {
				case '0':
					noteData[number].duration_on = 96;
					noteData[number].duration_off = 96;
					break;
				case '1':
					noteData[number].duration_on = 48;
					noteData[number].duration_off = 48;
					break;
				case '2':
					noteData[number].duration_on = 24;
					noteData[number].duration_off = 24;
					break;
				case '3':
					noteData[number].duration_on = 12;
					noteData[number].duration_off = 12;
					break;
				case '4':
					noteData[number].duration_on = 6;
					noteData[number].duration_off = 6;
					break;
				case '5':
					noteData[number].duration_on = 3;
					noteData[number].duration_off = 3;
					break;
				default:
					break;
			}
		}

		switch (Melody[number].duration) {
			case '0':
				durationSpec[number] = 192;
				break;
			case '1':
				durationSpec[number] = 96;
				break;
			case '2':
				durationSpec[number] = 48;
				break;
			case '3':
				durationSpec[number] = 24;
				break;
			case '4':
				durationSpec[number] = 12;
				break;
			case '5':
				durationSpec[number] = 6;
				break;
			default:
				break;
		}

		if (Melody[number].duration_specifier != '%') {
			switch (Melody[number].duration_specifier) {
				case '.':
					noteData[number].duration_on += (durationSpec[number] / 2);
					break;
				case ':':
					noteData[number].duration_on += durationSpec[number];
					break;
				case ';':
					noteData[number].duration_on -= (durationSpec[number] / 3);
					break;
				default:
					break;
			}

			if (noteData[number].duration_on > MIDI_MAX)
				noteData[number].duration_on = MIDI_LIMIT;
		}
	}

	for (number = 1; number < noteCount; number++) {
		if (Melody[number].rest >= '0' && Melody[number].rest <= '5') {
			switch (Melody[number].rest) {
				case '0':
					noteData[number - 1].duration_off += 192;
					restSpec[number] = 192;
					break;
				case '1':
					noteData[number - 1].duration_off += 96;
					restSpec[number] = 96;
					break;
				case '2':
					noteData[number - 1].duration_off += 48;
					restSpec[number] = 48;
					break;
				case '3':
					noteData[number - 1].duration_off += 24;
					restSpec[number] = 24;
					break;
				case '4':
					noteData[number - 1].duration_off += 12;
					restSpec[number] = 12;
					break;
				case '5':
					noteData[number - 1].duration_off += 6;
					restSpec[number] = 6;
					break;
				default:
					break;
			}

			if (noteData[number - 1].duration_off > MIDI_MAX && Melody[number].rest_specifier == '%')
				noteData[number - 1].duration_off = MIDI_LIMIT;
		}

		if (Melody[number].rest_specifier != '%') {
			switch (Melody[number].rest_specifier) {
				case '.':
					noteData[number - 1].duration_off += (restSpec[number] / 2);
					break;
				case ':':
					noteData[number - 1].duration_off += restSpec[number];
					break;
				case ';':
					noteData[number - 1].duration_off -= (restSpec[number] / 3);
					break;
				default:
					break;
			}

			if (noteData[number - 1].duration_off > MIDI_MAX)
				noteData[number - 1].duration_off = MIDI_LIMIT;
		}
	}

	if (Melody[0].rest >= '0' && Melody[0].rest <= '5') {
		switch (Melody[0].rest) {
			case '0':
				midiData[50] += 192;
				restSpec[0] = 192;
				break;
			case '1':
				midiData[50] += 96;
				restSpec[0] = 96;
				break;
			case '2':
				midiData[50] += 48;
				restSpec[0] = 48;
				break;
			case '3':
				midiData[50] += 24;
				restSpec[0] = 24;
				break;
			case '4':
				midiData[50] += 12;
				restSpec[0] = 12;
				break;
			case '5':
				midiData[50] += 6;
				restSpec[0] = 6;
				break;
			default:
				break;
		}

		if (Melody[0].rest_specifier != '%') {
			switch (Melody[0].rest_specifier) {
				case '.':
					midiData[50] += (restSpec[0] / 2);
					break;
				case ':':
					midiData[50] += restSpec[0];
					break;
				case ';':
					midiData[50] -= (restSpec[0] / 3);
					break;
				default:
					break;
			}
		}

		if (midiData[50] > MIDI_LIMIT)
			midiData[50] = MIDI_LIMIT;

		if (Melody[0].rest == '0')
			midiData[50] = MIDI_LIMIT;
	}

	for (number = 0; number < noteCount; number++) {
		noteBase[0] = noteData[number].note;
		noteBase[2] = noteData[number].duration_on;
		noteBase[3] = noteData[number].note;
		noteBase[5] = noteData[number].duration_off;

		noteTotal[6 * number] = noteBase[0];
		noteTotal[6 * number + 2] = noteBase[2];
		noteTotal[6 * number + 3] = noteBase[3];
		noteTotal[6 * number + 5] = noteBase[5];
		noteTotal[6 * number + 4] = 0;

		if (noteTotal[6 * number + 2] > MIDI_LIMIT)
			noteTotal[6 * number + 2] = MIDI_LIMIT;

		if (noteTotal[6 * number + 5] > MIDI_LIMIT)
			noteTotal[6 * number + 5] = MIDI_LIMIT;
	}

	for (number = 1; number < noteCount; number++) {
		noteTotal[1] = 84;

		if (Melody[0].vol == '+')
			noteTotal[1] = 84 + VOL_INTERVAL;

		if (Melody[0].vol == '-')
			noteTotal[1] = 84 - VOL_INTERVAL;

		switch (Melody[number].vol) {
			case '+':
				noteTotal[6 * number + 1] = noteTotal[6 * (number - 1) + 1] + VOL_INTERVAL;
				break;
			case '-':
				noteTotal[6 * number + 1] = noteTotal[6 * (number - 1) + 1] - VOL_INTERVAL;
				break;
			default:
				break;
		}

		if (noteTotal[6 * number + 1] > MIDI_LIMIT)
			noteTotal[6 * number + 1] = MIDI_LIMIT;

		if ((noteTotal[6 * (number - 1) + 1] == 0 || noteTotal[6 * (number - 1) + 1] == 7) && Melody[number].vol == '-')
			noteTotal[6 * number + 1] = 0;

		if (Melody[number].vol == '%')
			noteTotal[6 * number + 1] = noteTotal[6 * (number - 1) + 1];
	}

	for (number = 0; number < 6 * noteCount; number++)
		midiData[52 + number] = noteTotal[number];

	for (number = 6 * noteCount; number < 6 * noteCount * (repeat + 1); number++) {
		noteTotal[number] = noteTotal[repeatCount];
		midiData[52 + number] = noteTotal[number];
		repeatCount++;

		if (repeatCount == 6 * noteCount)
			repeatCount = 0;
	}

	if (vol != '%') {
		switch (vol) {
			case '+':
				midiData[52 + (6 * noteCount + 1)] = midiData[52 + (6 * (noteCount - 1) + 1)] + VOL_INTERVAL;
				break;
			case '-':
				midiData[52 + (6 * noteCount + 1)] = midiData[52 + (6 * (noteCount - 1) + 1)] - VOL_INTERVAL;
				break;
			default:
				break;
		}

		if (Melody[0].vol != '%') {
			switch (Melody[0].vol) {
				case '+':
					midiData[52 + (6 * noteCount + 1)] += VOL_INTERVAL;
					break;
				case '-':
					midiData[52 + (6 * noteCount + 1)] -= VOL_INTERVAL;
					break;
				default:
					break;
			}
		}

		if (midiData[52 + (6 * noteCount + 1)] > MIDI_LIMIT)
			midiData[52 + (6 * noteCount + 1)] = MIDI_LIMIT;

		if ((midiData[52 + (6 * (noteCount - 1) + 1)] == 0 || midiData[52 + (6 * (noteCount - 1) + 1)] == 7) && vol == '-') {
			midiData[52 + (6 * noteCount + 1)] = 0;

			if (Melody[0].vol == '+')
				midiData[52 + (6 * noteCount + 1)] = 12;
		}

		if ((midiData[52 + (6 * (noteCount - 1) + 1)] == 12 || midiData[52 + (6 * (noteCount - 1) + 1)] == 19) && vol == '-' && Melody[0].vol == '-')
			midiData[52 + (6 * noteCount + 1)] = 0;
	}

	else if (Melody[0].vol != '%' && vol == '%' && repeat != 0) {
		switch (Melody[0].vol) {
			case '+':
				midiData[52 + (6 * noteCount + 1)] = midiData[52 + (6 * (noteCount - 1) + 1)] + VOL_INTERVAL;
				break;
			case '-':
				midiData[52 + (6 * noteCount + 1)] = midiData[52 + (6 * (noteCount - 1) + 1)] - VOL_INTERVAL;
				break;
			default:
				break;
		}

		if (midiData[52 + (6 * noteCount + 1)] > MIDI_LIMIT)
			midiData[52 + (6 * noteCount + 1)] = MIDI_LIMIT;

		if ((midiData[52 + (6 * (noteCount - 1) + 1)] == 0 || midiData[52 + (6 * (noteCount - 1) + 1)] == 7) && Melody[0].vol == '-')
			midiData[52 + (6 * noteCount + 1)] = 0;
	}

	else if (Melody[0].vol == '%' && vol == '%' && repeat != 0)
		midiData[52 + (6 * noteCount + 1)] = midiData[52 + (6 * (noteCount - 1) + 1)];

	volInterval = midiData[52 + (6 * noteCount + 1)] - midiData[53];

	for (repeatCount = 0; repeatCount < repeat; repeatCount++)
		for (number = 6 * noteCount * repeatCount + 1; number < 6 * noteCount * (repeatCount + 1); number = number + 6) {
			midiData[52 + (number + 6 * noteCount)] = midiData[52 + number] + volInterval;

			if (midiData[52 + number] + volInterval > MIDI_LIMIT)
				midiData[52 + (number + 6 * noteCount)] = MIDI_LIMIT;

			if (midiData[52 + number] < volInterval * (-1))
				midiData[52 + (number + 6 * noteCount)] = 0;
		}

	for (number = 1; number < 6 * noteCount * (repeat + 1); number = number + 6) {
		if (midiData[52 + number] > MIDI_LIMIT)
			midiData[52 + number] = MIDI_LIMIT;
	}

	pMelodyBuf = pMelodyStart;
	count = noteCount;

	if (repeat != 0) {
		while (*pMelodyBuf != '@')
			pMelodyBuf++;

		pMelodyBuf++;

		if (vol != '%')
			pMelodyBuf = pMelodyBuf + 2;

		while (!(*pMelodyBuf == '*' || *pMelodyBuf == '#' || *pMelodyBuf == '&' || *pMelodyBuf == 'r' || *pMelodyBuf == 'V' || *pMelodyBuf == 'E' || (*pMelodyBuf >= 'a' && *pMelodyBuf <= 'g')))
			pMelodyBuf++;
	}

	if (*pMelodyBuf != 'E' && *pMelodyBuf != ':') {
		pMelodyBuf--;

		while (*pMelodyBuf != 'E') {
			if (noteCount >= AV_MIDI_NOTE_MAX) {
				debug_warning("__AvConvertIMelody2MIDI : noteCount>=AV_MIDI_NOTE_MAX\n");
				break;
			}

			pMelodyBuf++;

			if (*pMelodyBuf == '*') {
				pMelodyBuf++;

				if (*pMelodyBuf >= '0' && *pMelodyBuf <= '8')
					octave[noteCount] = *pMelodyBuf;
			}

			if (*pMelodyBuf == '#' || *pMelodyBuf == '&')
				Melody[noteCount].flat_sharp = *pMelodyBuf;

			if (*pMelodyBuf == 'r') {
				pMelodyBuf++;

				if (*pMelodyBuf >= '0' && *pMelodyBuf <= '5') {
					Melody[noteCount].rest = *pMelodyBuf;
					pMelodyBuf++;

					if (*pMelodyBuf == '.' || *pMelodyBuf == ':' || *pMelodyBuf == ';')
						Melody[noteCount].rest_specifier = *pMelodyBuf;
				}
			}

			if (*pMelodyBuf == 'V') {
				pMelodyBuf++;

				if (*pMelodyBuf == '+' || *pMelodyBuf == '-')
					Melody[noteCount].vol = *pMelodyBuf;
			}

			if (*pMelodyBuf >= 'a' && *pMelodyBuf <= 'g') {
				Melody[noteCount].note = *pMelodyBuf;
				pMelodyBuf++;

				if (*pMelodyBuf >= '0' && *pMelodyBuf <= '5') {
					Melody[noteCount].duration = *pMelodyBuf;
					pMelodyBuf++;

					if (*pMelodyBuf == '.' || *pMelodyBuf == ':' || *pMelodyBuf == ';')
						Melody[noteCount].duration_specifier = *pMelodyBuf;

					else
						pMelodyBuf--;

					noteCount++;
				}
			}
		}

		for (octaveCount = count; octaveCount < noteCount && octaveCount < AV_MIDI_NOTE_MAX; octaveCount++) {
			if (octave[octaveCount] == '%')
				octave[octaveCount] = octave[octaveCount - 1];
		}

		for (number = count; number < noteCount && number < AV_MIDI_NOTE_MAX; number++) {
			octaveValue = octave[number] - '0';

			octaveValue = octaveValue * 12;

			if (Melody[number].flat_sharp == '#') {
				switch (Melody[number].note) {
					case 'c':
						noteData[number].note = octaveValue + 1;
						break;
					case 'd':
						noteData[number].note = octaveValue + 3;
						break;
					case 'f':
						noteData[number].note = octaveValue + 6;
						break;
					case 'g':
						noteData[number].note = octaveValue + 8;
						break;
					case 'a':
						noteData[number].note = octaveValue + 10;
						break;
					default:
						break;
				}
			}

			else if (Melody[number].flat_sharp == '&') {
				switch (Melody[number].note) {
					case 'd':
						noteData[number].note = octaveValue + 1;
						break;
					case 'e':
						noteData[number].note = octaveValue + 3;
						break;
					case 'g':
						noteData[number].note = octaveValue + 6;
						break;
					case 'a':
						noteData[number].note = octaveValue + 8;
						break;
					case 'b':
						noteData[number].note = octaveValue + 10;
						break;
					default:
						break;
				}
			}

			else {
				switch (Melody[number].note) {
					case 'c':
						noteData[number].note = octaveValue;
						break;
					case 'd':
						noteData[number].note = octaveValue + 2;
						break;
					case 'e':
						noteData[number].note = octaveValue + 4;
						break;
					case 'f':
						noteData[number].note = octaveValue + 5;
						break;
					case 'g':
						noteData[number].note = octaveValue + 7;
						break;
					case 'a':
						noteData[number].note = octaveValue + 9;
						break;
					case 'b':
						noteData[number].note = octaveValue + 11;
						break;
					default:
						break;
				}
			}


			if (style == '0') {
				switch (Melody[number].duration) {
					case '0':
						noteData[number].duration_on = 183;
						noteData[number].duration_off = 9;
						break;
					case '1':
						noteData[number].duration_on = 91;
						noteData[number].duration_off = 5;
						break;
					case '2':
						noteData[number].duration_on = 46;
						noteData[number].duration_off = 2;
						break;
					case '3':
						noteData[number].duration_on = 23;
						noteData[number].duration_off = 1;
						break;
					case '4':
						noteData[number].duration_on = 11;
						noteData[number].duration_off = 1;
						break;
					case '5':
						noteData[number].duration_on = 5;
						noteData[number].duration_off = 1;
						break;
					default:
						break;
				}
			}

			else if (style == '1') {
				switch (Melody[number].duration) {
					case '0':
						noteData[number].duration_on = 192;
						noteData[number].duration_off = 0;
						break;
					case '1':
						noteData[number].duration_on = 96;
						noteData[number].duration_off = 0;
						break;
					case '2':
						noteData[number].duration_on = 48;
						noteData[number].duration_off = 0;
						break;
					case '3':
						noteData[number].duration_on = 24;
						noteData[number].duration_off = 0;
						break;
					case '4':
						noteData[number].duration_on = 12;
						noteData[number].duration_off = 0;
						break;
					case '5':
						noteData[number].duration_on = 6;
						noteData[number].duration_off = 0;
						break;
					default:
						break;
				}
			}

			else {
				switch (Melody[number].duration) {
					case '0':
						noteData[number].duration_on = 96;
						noteData[number].duration_off = 96;
						break;
					case '1':
						noteData[number].duration_on = 48;
						noteData[number].duration_off = 48;
						break;
					case '2':
						noteData[number].duration_on = 24;
						noteData[number].duration_off = 24;
						break;
					case '3':
						noteData[number].duration_on = 12;
						noteData[number].duration_off = 12;
						break;
					case '4':
						noteData[number].duration_on = 6;
						noteData[number].duration_off = 6;
						break;
					case '5':
						noteData[number].duration_on = 3;
						noteData[number].duration_off = 3;
						break;
					default:
						break;
				}
			}

			switch (Melody[number].duration) {
				case '0':
					durationSpec[number] = 192;
					break;
				case '1':
					durationSpec[number] = 96;
					break;
				case '2':
					durationSpec[number] = 48;
					break;
				case '3':
					durationSpec[number] = 24;
					break;
				case '4':
					durationSpec[number] = 12;
					break;
				case '5':
					durationSpec[number] = 6;
					break;
				default:
					break;
			}

			if (Melody[number].duration_specifier != '%') {
				switch (Melody[number].duration_specifier) {
					case '.':
						noteData[number].duration_on += (durationSpec[number] / 2);
						break;
					case ':':
						noteData[number].duration_on += durationSpec[number];
						break;
					case ';':
						noteData[number].duration_on -= (durationSpec[number] / 3);
						break;
					default:
						break;
				}

				if (noteData[number].duration_on > MIDI_MAX)
					noteData[number].duration_on = MIDI_LIMIT;
			}
		}

		for (number = count + 1; number < noteCount && number < AV_MIDI_NOTE_MAX; number++) {
			if (Melody[number].rest >= '0' && Melody[number].rest <= '5') {
				switch (Melody[number].rest) {
					case '0':
						noteData[number - 1].duration_off += 192;
						restSpec[number] = 192;
						break;
					case '1':
						noteData[number - 1].duration_off += 96;
						restSpec[number] = 96;
						break;
					case '2':
						noteData[number - 1].duration_off += 48;
						restSpec[number] = 48;
						break;
					case '3':
						noteData[number - 1].duration_off += 24;
						restSpec[number] = 24;
						break;
					case '4':
						noteData[number - 1].duration_off += 12;
						restSpec[number] = 12;
						break;
					case '5':
						noteData[number - 1].duration_off += 6;
						restSpec[number] = 6;
						break;
					default:
						break;
				}

				if (noteData[number - 1].duration_off > MIDI_MAX && Melody[number].rest_specifier == '%')
					noteData[number - 1].duration_off = MIDI_LIMIT;
			}

			if (Melody[number].rest_specifier != '%') {
				switch (Melody[number].rest_specifier) {
					case '.':
						noteData[number - 1].duration_off += (restSpec[number] / 2);
						break;
					case ':':
						noteData[number - 1].duration_off += restSpec[number];
						break;
					case ';':
						noteData[number - 1].duration_off -= (restSpec[number] / 3);
						break;
					default:
						break;
				}

				if (noteData[number - 1].duration_off > MIDI_MAX)
					noteData[number - 1].duration_off = MIDI_LIMIT;
			}
		}

		if (Melody[count].rest >= '0' && Melody[count].rest <= '5') {
			switch (Melody[count].rest) {
				case '0':
					midiData[52 + (6 * count * (repeat + 1) - 1)] += 192;
					restSpec[count] = 192;
					break;
				case '1':
					midiData[52 + (6 * count * (repeat + 1) - 1)] += 96;
					restSpec[count] = 96;
					break;
				case '2':
					midiData[52 + (6 * count * (repeat + 1) - 1)] += 48;
					restSpec[count] = 48;
					break;
				case '3':
					midiData[52 + (6 * count * (repeat + 1) - 1)] += 24;
					restSpec[count] = 24;
					break;
				case '4':
					midiData[52 + (6 * count * (repeat + 1) - 1)] += 12;
					restSpec[count] = 12;
					break;
				case '5':
					midiData[52 + (6 * count * (repeat + 1) - 1)] += 6;
					restSpec[count] = 6;
					break;
				default:
					break;
			}

			if (Melody[count].rest_specifier != '%') {
				switch (Melody[count].rest_specifier) {
					case '.':
						midiData[52 + (6 * count * (repeat + 1) - 1)] += (restSpec[count] / 2);
						break;
					case ':':
						midiData[52 + (6 * count * (repeat + 1) - 1)] += restSpec[count];
						break;
					case ';':
						midiData[52 + (6 * count * (repeat + 1) - 1)] -= (restSpec[count] / 3);
						break;
					default:
						break;
				}
			}

			if (midiData[52 + (6 * count * (repeat + 1) - 1)] > MIDI_LIMIT)
				midiData[52 + (6 * count * (repeat + 1) - 1)] = MIDI_LIMIT;

			if (Melody[count].rest == '0')
				midiData[52 + (6 * count * (repeat + 1) - 1)] = MIDI_LIMIT;
		}

		for (number = count; number < noteCount; number++) {
			noteBase[0] = noteData[number].note;
			noteBase[2] = noteData[number].duration_on;
			noteBase[3] = noteData[number].note;
			noteBase[5] = noteData[number].duration_off;

			noteTotal[6 * number] = noteBase[0];
			noteTotal[6 * number + 2] = noteBase[2];
			noteTotal[6 * number + 3] = noteBase[3];
			noteTotal[6 * number + 5] = noteBase[5];
			noteTotal[6 * number + 4] = 0;

			if (noteTotal[6 * number + 2] > MIDI_LIMIT)
				noteTotal[6 * number + 2] = MIDI_LIMIT;

			if (noteTotal[6 * number + 5] > MIDI_LIMIT)
				noteTotal[6 * number + 5] = MIDI_LIMIT;
		}

		for (number = count + 1; number < noteCount; number++) {
			noteTotal[6 * count + 1] = midiData[52 + (6 * count * (repeat + 1) - 5)];

			if (Melody[count].vol == '+')
				noteTotal[6 * count + 1] = noteTotal[6 * count + 1] + VOL_INTERVAL;

			if (Melody[count].vol == '-' && (noteTotal[6 * count + 1] == 0 || noteTotal[6 * count + 1] == 7))
				noteTotal[6 * count + 1] = 0;

			if (Melody[count].vol == '-' && noteTotal[6 * count + 1] >= 12)
				noteTotal[6 * count + 1] = noteTotal[6 * count + 1] - VOL_INTERVAL;

			if (noteTotal[6 * count + 1] > MIDI_LIMIT)
				noteTotal[6 * count + 1] = MIDI_LIMIT;

			switch (Melody[number].vol) {
				case '+':
					noteTotal[6 * number + 1] = noteTotal[6 * (number - 1) + 1] + VOL_INTERVAL;
					break;
				case '-':
					noteTotal[6 * number + 1] = noteTotal[6 * (number - 1) + 1] - VOL_INTERVAL;
					break;
				default:
					break;
			}

			if (noteTotal[6 * number + 1] > MIDI_LIMIT)
				noteTotal[6 * number + 1] = MIDI_LIMIT;

			if ((noteTotal[6 * (number - 1) + 1] == 0 || noteTotal[6 * (number - 1) + 1] == 7) && Melody[number].vol == '-')
				noteTotal[6 * number + 1] = 0;

			if (Melody[number].vol == '%')
				noteTotal[6 * number + 1] = noteTotal[6 * (number - 1) + 1];
		}

		numberCount = 6 * count;

		for (number = 6 * count * (repeat + 1); number < 6 * (count * (repeat + 1) + (noteCount - count)); number++) {
			midiData[52 + number] = noteTotal[numberCount];
			numberCount++;
		}
	}

	noteTotal[6 * (count * (repeat + 1) + (noteCount - count))] = 0;				/*0x00 */
	noteTotal[6 * (count * (repeat + 1) + (noteCount - count)) + 1] = MIDI_MAX;		/*0xff */
	noteTotal[6 * (count * (repeat + 1) + (noteCount - count)) + 2] = 47;			/*0x2f */
	noteTotal[6 * (count * (repeat + 1) + (noteCount - count)) + 3] = 0;			/*0x00 */

	for (number = 6 * (count * (repeat + 1) + (noteCount - count)); number <= 6 * (count * (repeat + 1) + (noteCount - count)) + 3; number++)
		midiData[51 + number] = noteTotal[number];

	trackSize = (6 * (count * (repeat + 1) + (noteCount - count)) + 56) - 22 - 1 ;

	midiData[20] = (trackSize & 0xff00) >> 8;
	midiData[21] = (trackSize & 0x00ff);


	*pBufLen = 6 * (count * (repeat + 1) + (noteCount - count)) + 56 - 1;

	pConvertBuf = (unsigned char *) mmfile_malloc(*pBufLen);
	if (pConvertBuf == NULL) {
		debug_error("__AvConvertIMelody2MIDI: malloc failed!\n");
		return NULL;
	}

	memcpy(pConvertBuf, midiData, *pBufLen);

	return pConvertBuf;
}

static unsigned char
__AvMIDISetVolume(char *pMelodyBuf)
{
	unsigned char midiVol;

	pMelodyBuf = pMelodyBuf + 42;

	while (!((*pMelodyBuf == 'V' && (*(pMelodyBuf + 1) < 'a' || *(pMelodyBuf + 1) > 'z')) || (*pMelodyBuf == 'M' && *(pMelodyBuf + 5) == 'Y' && *(pMelodyBuf + 6) == ':'))) /*2007-02-28 AVMS_Sound:k2bogus - UMTS200073205;imy play, [MELODY:] extract fix */
		pMelodyBuf++;

	if (*pMelodyBuf != 'V')
		midiVol = AV_MIDI_VOL_MAX;

	else {
		pMelodyBuf = pMelodyBuf + 5;

		if (*pMelodyBuf == 'E')
			pMelodyBuf = pMelodyBuf + 3;

		else
			pMelodyBuf = pMelodyBuf - 4;

		if (*pMelodyBuf == '1') {
			pMelodyBuf++;

			switch (*pMelodyBuf) {
				case '0':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '1':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '2':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '3':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '4':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '5':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				default:
					midiVol = AV_MIDI_VOL_MAX;
					break;
			}
		}

		else
			switch (*pMelodyBuf) {
				case '0':
					midiVol = 0;
					break;
				case '2':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '3':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '4':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '5':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '6':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '7':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '8':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				case '9':
					midiVol = AV_MIDI_VOL_MAX;
					break;
				default:
					midiVol = AV_MIDI_VOL_MAX;
					break;
			}
	}

	return midiVol;
}

static int
__AvMIDISetBeat(char *pMelodyBuf)
{
	int bpmValue[4] = {0};
	int beatValue;

	pMelodyBuf = pMelodyBuf + 42;

	while (!((*pMelodyBuf == 'B' && (*(pMelodyBuf + 1) < 'a' || *(pMelodyBuf + 1) > 'z')) || (*pMelodyBuf == 'M' && *(pMelodyBuf + 5) == 'Y' && *(pMelodyBuf + 6) == ':'))) /*2007-02-28 AVMS_Sound:k2bogus - UMTS200073205;imy play, [MELODY:] extract fix */
		pMelodyBuf++;

	if (*pMelodyBuf != 'B')
		bpmValue[3] = 120;

	else {
		pMelodyBuf = pMelodyBuf + 4;

		if (*pMelodyBuf == ':') {
			pMelodyBuf++;

			if (*pMelodyBuf >= '1' && *pMelodyBuf <= '9') {
				bpmValue[0] = *pMelodyBuf - '0';
				pMelodyBuf++;

				if (*pMelodyBuf >= '0' && *pMelodyBuf <= '9') {
					bpmValue[1] = *pMelodyBuf - '0';
					pMelodyBuf++;

					if (*pMelodyBuf >= '0' && *pMelodyBuf <= '9') {
						bpmValue[2] = *pMelodyBuf - '0';

						bpmValue[0] = bpmValue[0] * 100;
						bpmValue[1] = bpmValue[1] * 10;
						pMelodyBuf++;
					}

					else
						bpmValue[0] = bpmValue[0] * 10;
				}
			}

			bpmValue[3] = bpmValue[0] + bpmValue[1] + bpmValue[2];
		}
	}

	if (bpmValue[3] < 25 || bpmValue[3] > 900)
		bpmValue[3] = 120;

	if (*pMelodyBuf >= '0' && *pMelodyBuf <= '9')
		bpmValue[3] = 120;

	beatValue = 1000000 * 60 / bpmValue[3];

#ifdef __MMFILE_TEST_MODE__
	debug_msg("beat: %d = 1000000 * 60 / %d\n", beatValue, bpmValue[3]);
#endif

	return beatValue;
}

static char
__AvMIDISetStyle(char *pMelodyBuf)
{
	char styleValue = '0';

	while (*pMelodyBuf != 'S')
		pMelodyBuf--;

	pMelodyBuf++;

	if (*pMelodyBuf >= '0' && *pMelodyBuf <= '2')
		pMelodyBuf++;

	if (*pMelodyBuf != '.') {
		pMelodyBuf--;
		styleValue = *pMelodyBuf;
	}

	if (styleValue < '0' || styleValue > '2') {
		debug_warning("unknown style. use default(S0)\n");
		styleValue = '0';
	}

#ifdef __MMFILE_TEST_MODE__
	debug_msg("style: '%c'\n", styleValue);
#endif

	return styleValue;
}


