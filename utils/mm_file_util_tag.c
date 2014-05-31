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
#include <ctype.h>
#include <vconf.h>

#include "mm_debug.h"
#include "mm_file_utils.h"

#define ENABLE_ITUNES_META		//All itunes metadata extracted by ffmpeg. see mov_read_udta_string() but Some cover art not support.

typedef struct _mmfilemp4basicboxheader {
	unsigned int size;
	unsigned int type;
	long long start_offset;	/*huge file*/
} MMFILE_MP4_BASIC_BOX_HEADER;

typedef struct _mmfilemp4ftpybox {
	unsigned int major_brand;
	unsigned short major_version;
	unsigned short minor_version;
	unsigned int *compatiable_brands;
} MMFILE_MP4_FTYPBOX;

typedef enum {
	eMMFILE_3GP_TAG_TITLE = 0x01,
	eMMFILE_3GP_TAG_CAPTION = 0x02,
	eMMFILE_3GP_TAG_COPYRIGHT = 0x03,
	eMMFILE_3GP_TAG_PERFORMER = 0x04,
	eMMFILE_3GP_TAG_AUTHOR = 0x05,
	eMMFILE_3GP_TAG_GENRE = 0x06,
} eMMFILE_3GP_TEXT_TAG;

typedef struct _mmfile3gptagtextbox {
	unsigned char version;
	unsigned char flag[3];
	unsigned char language[2];
	unsigned char *text;
} MMFILE_3GP_TEXT_TAGBOX;

typedef struct _mmfile3gptagyearbox {
	unsigned char version;
	unsigned char flag[3];
	unsigned short year;
} MMFILE_3GP_YEAR_TAGBOX;

typedef struct _mmfile3gptagalbumbox {
	unsigned char version;
	unsigned char flag[3];
	unsigned char language[2];
	unsigned char *albumtile;
	unsigned char trackNumber;
} MMFILE_3GP_ALBUM_TAGBOX;

typedef struct _mmfile3gpratingbox {
	unsigned char version;
	unsigned char flag[3];
	unsigned int  ratingEntity;
	unsigned int  ratingCriteria;
	unsigned char language[2];
	unsigned char *ratingInfo;
} MMFILE_3GP_RATING_TAGBOX;

typedef struct _mmfile3gpclsfbox {
	unsigned char version;
	unsigned char flag[3];
	unsigned int  classificationEntity;
	unsigned int  classificationTable;
	unsigned char language[2];
	unsigned char *classificationInfo;
} MMFILE_3GP_CLASSIFICATION_TAGBOX;

typedef struct _mmfile3gphdlrbox {
	unsigned int  version;
	unsigned int  pre_defined;
	unsigned int  handler_type;
	unsigned int  reserved[3];
	unsigned char *boxname;
} MMFILE_3GP_HANDLER_BOX;

typedef struct _mmfileidv2box {
	unsigned char version;
	unsigned char flag[3];
	unsigned char language[2];
	unsigned char *id3v2Data;
} MMFILE_3GP_ID3V2_BOX;

typedef struct _mmfilelocibox {
	unsigned char version;
	unsigned char flag[3];
	unsigned char language[2];
	unsigned char *name;
	unsigned char role;
	float         longitude;
	float         latitude;
	float         altitude;
	unsigned char *astronomical_body;
	unsigned char *additional_notes;
} MMFILE_3GP_LOCATION_TAGBOX;

typedef struct _mmfilesmtabox {
	unsigned int tmp;
	unsigned int length;
	unsigned char saut[4];
	unsigned int value;
} MMFILE_M4A_SMTA_TAGBOX;

#define MMFILE_MP4_BASIC_BOX_HEADER_LEN 8
#define MMFILE_MP4_MOVIE_HEADER_BOX_LEN 96
#define MMFILE_MP4_HDLR_BOX_LEN         24
#define MMFILE_MP4_STSZ_BOX_LEN         20
#define MMFILE_MP4_MP4VBOX_LEN          80
#define MMFILE_MP4_MP4ABOX_LEN          28
#define MMFILE_3GP_TEXT_TAGBOX_LEN      6
#define MMFILE_3GP_YEAR_TAGBOX_LEN      6
#define MMFILE_3GP_ALBUM_TAGBOX_LEN     6
#define MMFILE_3GP_RATING_TAGBOX_LEN    14
#define MMFILE_3GP_CLASS_TAGBOX_LEN     14
#define MMFILE_3GP_HANDLER_BOX_LEN      24
#define MMFILE_3GP_ID3V2_BOX_LEN        6
#define MMFILE_SYNC_LYRIC_INFO_MIN_LEN        5


#define FOURCC(a,b,c,d) ((a) + ((b) << 8) + ((c) << 16) + ((d) << 24))

#define	MIN(a,b) (((a)<(b))?(a):(b))

#define GENRE_COUNT	149

static char* MpegAudio_Genre[GENRE_COUNT] = {"Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk", "Grunge", "Hip-Hop", "Jazz", "Metal",
"New Age", "Oldies", "Other", "Pop", "R&B", "Rap", "Reggae", "Rock", "Techno", "Industrial",
"Alternative", "Ska", "Death Metal", "Pranks", "Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
"Fusion", "Trance", "Classical", "Instrumental", "Acid", "House", "Game", "Sound Clip", "Gospel", "Noise",
"AlternRock", "Bass", "Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock", "Ethnic", "Gothic",
"Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk", "Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta",
"Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American", "Cabaret", "New Wave", "Psychadelic", "Rave", "Showtunes",
"Trailer", "Lo-Fi", "Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical", "Rock & Roll", "Hard Rock",
"Folk", "Folk-Rock", "National Folk", "Swing", "Fast Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass",
"Avantgarde", "Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band", "Chorus", "Easy Listening", "Acoustic",
"Humour", "Speech", "Chanson", "Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus", "Porn Groove",
"Satire", "Slow Jam", "Club", "Tango", "Samba", "Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle",
"Duet", "Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall", "Goa", "Drum & Bass", "Club-House", "Hardcore",
"Terror", "Indie", "BritPop", "Negerpunk", "Polsk Punk", "Beat", "Christian", "Heavy Metal", "Black Metal", "Crossover",
"Contemporary", "Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop", "Synthpop", "Unknown" };


static int GetStringFromTextTagBox (MMFileFormatContext *formatContext, MMFileIOHandle *fp, MMFILE_MP4_BASIC_BOX_HEADER *basic_header, eMMFILE_3GP_TEXT_TAG eTag)
{
	int ret = MMFILE_UTIL_FAIL;    /*fail*/
	MMFILE_3GP_TEXT_TAGBOX texttag = {0,};
	int readed = 0;
	int textBytes = 0;
	char *temp_text = NULL;

	if (!formatContext || !fp || !basic_header) {
		debug_error ("invalid param\n");
		return MMFILE_UTIL_FAIL;
	}

	textBytes = basic_header->size - MMFILE_MP4_BASIC_BOX_HEADER_LEN - MMFILE_3GP_TEXT_TAGBOX_LEN;

	readed = mmfile_read (fp, (unsigned char *)&texttag, MMFILE_3GP_TEXT_TAGBOX_LEN);
	if (readed != MMFILE_3GP_TEXT_TAGBOX_LEN) {
		debug_error ("read text tag header fail\n");
		ret = MMFILE_UTIL_FAIL;
		goto exception;
	}

	if (textBytes <= 1) /* there exist only 00 (null) */
	{
		debug_error ("Text is NULL\n");
		goto exception;
	}

	texttag.text = mmfile_malloc (textBytes);
	if (!texttag.text) {
		debug_error ("malloc fail for text box\n");
		ret = MMFILE_UTIL_FAIL;
		goto exception;
	}

	readed = mmfile_read (fp, (unsigned char *)texttag.text, textBytes);
	if (readed != textBytes) {
		debug_error ("read text fail\n");
		ret = MMFILE_UTIL_FAIL;
		goto exception;
	}

	/* check BOM char */
	if ( (texttag.text[0] == 0xFE) && (texttag.text[1] == 0xFF)) {
		/* this char is UTF-16 */
		unsigned int bytes_written = 0;
		temp_text = mmfile_string_convert ((const char *)&texttag.text[2], readed-2, "UTF-8", "UTF-16",NULL, (unsigned int*)&bytes_written);
	} else {
		temp_text = mmfile_strdup ((const char *)texttag.text);
	}

	switch (eTag) {
		case eMMFILE_3GP_TAG_TITLE: {
			if (!formatContext->title) {
				formatContext->title = temp_text;
			}
			break;
		}
		case eMMFILE_3GP_TAG_CAPTION: {
			if (!formatContext->description) {
				formatContext->description = temp_text;
			}
			break;
		}
		case eMMFILE_3GP_TAG_COPYRIGHT: {
			if (!formatContext->copyright) {
				formatContext->copyright = temp_text;
			}
			break;
		}
		case eMMFILE_3GP_TAG_PERFORMER: {
			if (!formatContext->artist) {
				formatContext->artist = temp_text;
			}
			break;
		}
		case eMMFILE_3GP_TAG_AUTHOR: {
			if (!formatContext->author) {
				formatContext->author = temp_text;
			}
			break;
		}
		case eMMFILE_3GP_TAG_GENRE: {
			if (!formatContext->genre) {
				formatContext->genre = temp_text;
			}
			break;
		}
		default: {
			debug_warning("Not supported Text Tag type[%d]\n", eTag);
			break;
		}
	}

	mmfile_free (texttag.text);
	texttag.text = NULL;
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);

	return MMFILE_UTIL_SUCCESS;

exception:
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);
	if (texttag.text) {
		mmfile_free(texttag.text);
	}
	return ret;
}

static int GetYearFromYearTagBox (MMFileFormatContext *formatContext, MMFileIOHandle *fp, MMFILE_MP4_BASIC_BOX_HEADER *basic_header)
{
#define MAX_YEAR_BUFFER 10
	int readed = 0;
	MMFILE_3GP_YEAR_TAGBOX yearbox = {0,};
	char temp_year[MAX_YEAR_BUFFER] = {0,};

	if (!formatContext || !fp || !basic_header) {
		debug_error ("invalid param\n");
		return MMFILE_UTIL_FAIL;
	}

	readed = mmfile_read (fp, (unsigned char *)&yearbox, MMFILE_3GP_YEAR_TAGBOX_LEN);
	if (readed != MMFILE_3GP_YEAR_TAGBOX_LEN) {
		debug_error ("read yeartag header fail\n");
		goto exception;
	}

	if (!formatContext->year) {
		yearbox.year = mmfile_io_be_int16 (yearbox.year);
		snprintf (temp_year, MAX_YEAR_BUFFER, "%d", yearbox.year);
		temp_year[MAX_YEAR_BUFFER-1] = '\0';
		formatContext->year = mmfile_strdup ((const char*)temp_year);
	}

	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);
	return MMFILE_UTIL_SUCCESS;

exception:
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);

	return MMFILE_UTIL_FAIL;
}

static int GetAlbumFromAlbumTagBox (MMFileFormatContext *formatContext, MMFileIOHandle *fp, MMFILE_MP4_BASIC_BOX_HEADER *basic_header)
{
	int albumTitleLen = 0;
	char *temp_text = NULL;
	int readed = 0;
	int trackFlags = 0;
	MMFILE_3GP_ALBUM_TAGBOX albumbox = {0,};

	if (!formatContext || !fp || !basic_header) {
		debug_error ("invalid param\n");
		return MMFILE_UTIL_FAIL;
	}

	readed = mmfile_read (fp, (unsigned char *)&albumbox, MMFILE_3GP_ALBUM_TAGBOX_LEN);
	if (readed != MMFILE_3GP_ALBUM_TAGBOX_LEN) {
		debug_error ("read albumtag header fail\n");
		goto exception;
	}

	albumTitleLen = basic_header->size - MMFILE_MP4_BASIC_BOX_HEADER_LEN - MMFILE_3GP_ALBUM_TAGBOX_LEN - 1; /* 1: track number */
	if (albumTitleLen > 1) /* there exist only 00 (null) */
	{
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("albumTitleLen=%d\n", albumTitleLen);
		#endif

		albumbox.albumtile = mmfile_malloc (albumTitleLen + 1); /* 1: for null char */
		if (!albumbox.albumtile) {
			debug_error ("malloc fail for album title text\n");
			goto exception;
		}

		readed = mmfile_read (fp, (unsigned char *)albumbox.albumtile, albumTitleLen);
		if (readed != albumTitleLen) {
			debug_error ("read album title fail\n");
			goto exception;
		}

		if (albumbox.albumtile[albumTitleLen-1] == '\0') /* there exist track number */
		{
			trackFlags = 1;
		}
		else
		{
			trackFlags = 0;
			readed = mmfile_read (fp, (unsigned char *)&(albumbox.albumtile[albumTitleLen]), 1);
			if (readed != 1) {
				debug_error ("read album title fail\n");
				goto exception;
			}            
			albumbox.albumtile[albumTitleLen] = '\0';
		}

		/* check BOM char */
		if ( (albumbox.albumtile[0] == 0xFE) && (albumbox.albumtile[1] == 0xFF)) {
			/* this char is UTF-16 */
			unsigned int bytes_written = 0;
			temp_text = mmfile_string_convert ((const char*)&albumbox.albumtile[2], readed-2, "UTF-8", "UTF-16", NULL, (unsigned int*)&bytes_written);
		} else {
			temp_text = mmfile_strdup ((const char*)albumbox.albumtile);
		}

		if (!formatContext->album) {
			formatContext->album = temp_text;
		}

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("formatContext->album=%s, strlen=%d\n", formatContext->album, strlen(formatContext->album));
		#endif
	}

	if (trackFlags) {
		readed = mmfile_read (fp, (unsigned char *)&albumbox.trackNumber, 1);
		if (readed != 1) {
			debug_error ("read track number fail\n");
			goto exception;
		}

		if (formatContext->tagTrackNum == 0) {
			char tracknum[10] = {0,};
			snprintf (tracknum, 10, "%d", albumbox.trackNumber);
			tracknum[9] = '\0';
			formatContext->tagTrackNum = mmfile_strdup ((const char*)tracknum);
		}
	}

	mmfile_free (albumbox.albumtile);
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);

	return MMFILE_UTIL_SUCCESS;

exception:
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);
	mmfile_free (albumbox.albumtile);

	return MMFILE_UTIL_FAIL;
}

static int GetRatingFromRatingTagBox (MMFileFormatContext *formatContext, MMFileIOHandle *fp, MMFILE_MP4_BASIC_BOX_HEADER *basic_header)
{
	int readed = 0;
	int  ratinginfoLen = 0;
	char *temp_text = NULL;

	MMFILE_3GP_RATING_TAGBOX ratingTag = {0,};

	if (!formatContext || !fp || !basic_header) {
		debug_error ("invalid param\n");
		return MMFILE_UTIL_FAIL;
	}

	readed = mmfile_read (fp, (unsigned char *)&ratingTag, MMFILE_3GP_RATING_TAGBOX_LEN);
	if (readed != MMFILE_3GP_RATING_TAGBOX_LEN) {
		debug_error ("read rating tag header fail\n");
		goto exception;
	}

	ratinginfoLen = basic_header->size - MMFILE_MP4_BASIC_BOX_HEADER_LEN - MMFILE_3GP_RATING_TAGBOX_LEN;

	if (ratinginfoLen == 1) {
		debug_error ("Rating Text is NULL\n");
		goto exception;
	}

	ratingTag.ratingInfo = mmfile_malloc (ratinginfoLen);
	if (!ratingTag.ratingInfo) {
		debug_error ("rating info error\n");
		goto exception;
	}

	readed = mmfile_read (fp, (unsigned char *)ratingTag.ratingInfo, ratinginfoLen);
	if (readed != ratinginfoLen) {
		debug_error ("read rating info string fail\n");
		goto exception;
	}

	/* check BOM char */
	if ( (ratingTag.ratingInfo[0] == 0xFE) && (ratingTag.ratingInfo[1] == 0xFF)) {
		/* this char is UTF-16 */
		unsigned int bytes_written = 0;
		temp_text = mmfile_string_convert ((const char*)&ratingTag.ratingInfo[2], readed-2, "UTF-8", "UTF-16", NULL, (unsigned int*)&bytes_written);
	} else {
		temp_text = mmfile_strdup ((const char*)ratingTag.ratingInfo);
	}

	if (!formatContext->rating) {
		formatContext->rating = temp_text;
	} else {
		mmfile_free (temp_text);
	}

	mmfile_free (ratingTag.ratingInfo);
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);

	return MMFILE_UTIL_SUCCESS;

exception:
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);
	mmfile_free (ratingTag.ratingInfo);

	return MMFILE_UTIL_FAIL;
}


static int GetClassficationFromClsfTagBox (MMFileFormatContext *formatContext, MMFileIOHandle *fp, MMFILE_MP4_BASIC_BOX_HEADER *basic_header)
{
	int classinfoLen = 0;
	int readed = 0;
	char *temp_text = NULL;
	MMFILE_3GP_CLASSIFICATION_TAGBOX classTag = {0,};

	if (!formatContext || !fp || !basic_header) {
		debug_error ("invalid param\n");
		return MMFILE_UTIL_FAIL;
	}

	readed = mmfile_read (fp, (unsigned char *)&classTag, MMFILE_3GP_CLASS_TAGBOX_LEN);
	if (readed != MMFILE_3GP_CLASS_TAGBOX_LEN) {
		debug_error ("read classification tag header fail\n");
		goto exception;
	}


	classinfoLen = basic_header->size - MMFILE_MP4_BASIC_BOX_HEADER_LEN - MMFILE_3GP_CLASS_TAGBOX_LEN;
	if (classinfoLen == 1) {
		debug_error ("Classification Text is NULL\n");
		goto exception;
	}

	classTag.classificationInfo = mmfile_malloc (classinfoLen);
	if (!classTag.classificationInfo) {
		debug_error ("class info error\n");
		goto exception;
	}

	readed = mmfile_read (fp, (unsigned char *)classTag.classificationInfo, classinfoLen);
	if (readed != classinfoLen) {
		debug_error ("read class info string fail\n");
		goto exception;
	}

	/* check BOM char */
	if ( (classTag.classificationInfo[0] == 0xFE) && (classTag.classificationInfo[1] == 0xFF)) {
		/* this char is UTF-16 */
		unsigned int bytes_written = 0;
		temp_text = mmfile_string_convert ((const char*)&classTag.classificationInfo[2], readed-2, "UTF-8", "UTF-16", NULL, (unsigned int*)&bytes_written);
	} else {
		temp_text = mmfile_strdup ((const char*)classTag.classificationInfo);
	}

	if (!formatContext->classification) {
		formatContext->classification = temp_text;
	} else {
		mmfile_free (temp_text);
	}

	mmfile_free (classTag.classificationInfo);
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);

	return MMFILE_UTIL_SUCCESS;

exception:
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);
	mmfile_free (classTag.classificationInfo);

	return MMFILE_UTIL_FAIL;
}

/**
 * The Location Information box
 * --------------------+-------------------+-----------------------------------+------
 * Field				Type 				Details								Value
 * --------------------+-------------------+-----------------------------------+------
 * BoxHeader.Size		Unsigned int(32)
 * BoxHeader.Type		Unsigned int(32)										'loci'
 * BoxHeader.Version	Unsigned int(8)											0
 * BoxHeader.Flags		Bit(24)													0
 * Pad					Bit(1)													0
 * Language				Unsigned int(5)[3]	Packed ISO-639-2/T language code
 * Name					String				Text of place name
 * Role					Unsigned int(8)		Non-negative value indicating role
 * 											 of location
 * Longitude			Unsigned int(32)	Fixed-point value of the longitude
 * Latitude				Unsigned int(32)	Fixed-point value of the latitude
 * Altitude				Unsigned int(32)	Fixed-point value of the Altitude
 * Astronomical_body	String				Text of astronomical body
 * Additional_notes		String				Text of additional location-related
 * 											 information
 * --------------------+-------------------+-----------------------------------+------
 */
static int _get_char_position (unsigned char *src, char ch, int max)
{
	int i;
	for (i = 0; i < max; i++) {
		if ( *(src + i) == ch)
			return i;
	}

	return -1;
} 
 
static int GetLocationFromLociTagBox (MMFileFormatContext *formatContext, MMFileIOHandle *fp, MMFILE_MP4_BASIC_BOX_HEADER *basic_header)
{

	MMFILE_3GP_LOCATION_TAGBOX lociTag = {0,};
	int readed = 0;
	int bufferLen = 0;
	unsigned char *buffer = NULL;
	unsigned char *p = NULL;
	int pos = 0;
	unsigned int bytes_written = 0;
	unsigned int name_sz = 0;
	unsigned int astro_sz = 0;

	int ilong, ilati, ialti;
	float flong, flati, falti;


	if (!formatContext || !fp || !basic_header) {
		debug_error ("invalid param\n");
		return MMFILE_UTIL_FAIL;
	}

	readed = mmfile_read (fp, (unsigned char *)&lociTag, 6); /*6 = version + flag + pad + language */
	if (readed != 6) {
		debug_error ("read location tag header fail\n");
		goto exception;
	}

	/*buffer len = name + role + ... + additional notes length */
	bufferLen = basic_header->size - MMFILE_MP4_BASIC_BOX_HEADER_LEN - 6;
	if (bufferLen < 1) {
		debug_error ("too small buffer\n");
		goto exception;
	}

	buffer = mmfile_malloc (bufferLen);
	if (!buffer) {
		debug_error ("buffer malloc error\n");
		goto exception;
	}

	readed = mmfile_read (fp, (unsigned char *)buffer, bufferLen);
	if (readed != bufferLen) {
		debug_error ("read location tag fail\n");
		goto exception;
	}
	p = buffer;

	/*name*/
	pos = _get_char_position (p, '\0', readed - (1+4+4+4+2));
	if (pos >= 0) {
		if (p[0] == 0xFE && p[1] == 0xFF) {
			lociTag.name = (unsigned char*)mmfile_string_convert ((const char*)(p + 2), pos - 2, "UTF-8", "UTF-16", NULL, (unsigned int*)&bytes_written);
		} else {
			lociTag.name = (unsigned char*)mmfile_strdup ((const char*)p);
		}
	} else {
		goto exception;
	}
	name_sz = pos + 1;
	p += (pos + 1);

	/*role*/
	lociTag.role = *p;
	p++;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("long: 0x%02X 0x%02X 0x%02X 0x%02X \n", *(p+0), *(p+1), *(p+2), *(p+3));
	debug_msg ("lati: 0x%02X 0x%02X 0x%02X 0x%02X \n", *(p+4), *(p+5), *(p+6), *(p+7));
	debug_msg ("alti: 0x%02X 0x%02X 0x%02X 0x%02X \n", *(p+8), *(p+9), *(p+10), *(p+11));
	#endif

	ilong = mmfile_io_be_uint32 (*(unsigned int*)p);
	ilati = mmfile_io_be_uint32 (*(unsigned int*)(p+4));
	ialti = mmfile_io_be_uint32 (*(unsigned int*)(p+8));

	flong = (float)ilong / (1<<16);
	flati = (float)ilati / (1<<16);
	falti = (float)ialti / (1<<16);

	/*longitude*/
	lociTag.longitude = flong;
	/*latitude*/
	lociTag.latitude = flati;
	/*altitude*/
	lociTag.altitude = falti;
	
	p += 12;

	/*astronomical body*/
	pos = _get_char_position (p, '\0', readed - (name_sz+1+4+4+4+1));
	if (pos >= 0) {
		if (p[0] == 0xFE && p[1] == 0xFF) {
			lociTag.astronomical_body = (unsigned char*)mmfile_string_convert ((const char*)(p + 2), pos - 2, "UTF-8", "UTF-16", NULL, (unsigned int*)&bytes_written);
		} else {
			lociTag.astronomical_body = (unsigned char*)mmfile_strdup ((const char*)p);
		}
	} else {
		goto exception;
	}
	astro_sz = pos + 1;
	p += (pos + 1);

	/*additional notes*/
	pos = _get_char_position (p, '\0', readed - (name_sz+1+4+4+4+astro_sz));
	if (pos >= 0) {
		if (p[0] == 0xFE && p[1] == 0xFF) {
			lociTag.additional_notes = (unsigned char*)mmfile_string_convert ((const char*)(p + 2), pos - 2, "UTF-8", "UTF-16", NULL, (unsigned int*)&bytes_written);
		} else {
			lociTag.additional_notes = (unsigned char*)mmfile_strdup ((const char*)p);
		}
	} else {
		goto exception;
	}

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("** Location Information **\n");
	debug_msg ("Name             : %s\n", lociTag.name);
	debug_msg ("Role             : %d (0: shooting, 1: real, 2: fictional, other: reserved)\n", lociTag.role);
	debug_msg ("Longitude        : %16.16f\n", lociTag.longitude);
	debug_msg ("Latitude         : %16.16f\n", lociTag.latitude);
	debug_msg ("Altitude         : %16.16f\n", lociTag.altitude);
	debug_msg ("Astronomical body: %s\n", lociTag.astronomical_body);
	debug_msg ("Additional notes : %s\n", lociTag.additional_notes);
	#endif

	formatContext->longitude = lociTag.longitude;
	formatContext->latitude = lociTag.latitude;
	formatContext->altitude = lociTag.altitude;

	mmfile_free (buffer);
	mmfile_free (lociTag.name);
	mmfile_free (lociTag.astronomical_body);
	mmfile_free (lociTag.additional_notes);

	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);
	return MMFILE_UTIL_SUCCESS;
	
exception:
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);
	mmfile_free (buffer);
	mmfile_free (lociTag.name);
	mmfile_free (lociTag.astronomical_body);
	mmfile_free (lociTag.additional_notes);

	return MMFILE_UTIL_FAIL;
}

static int GetSAUTInfoFromSMTATagBox(MMFileFormatContext *formatContext, MMFileIOHandle *fp, MMFILE_MP4_BASIC_BOX_HEADER *basic_header)
{
	MMFILE_M4A_SMTA_TAGBOX smtaTag = {0,};
	int readed = 0;

	if (!formatContext || !fp || !basic_header) {
		debug_error ("invalid param\n");
		return MMFILE_UTIL_FAIL;
	}

	readed = mmfile_read (fp, (unsigned char *)&smtaTag, sizeof(MMFILE_M4A_SMTA_TAGBOX));
	if (readed != sizeof(MMFILE_M4A_SMTA_TAGBOX)) {
		debug_error ("read smta tag header fail\n");
		goto exception;
	}

	smtaTag.length = mmfile_io_be_uint32(smtaTag.length);
	smtaTag.value = mmfile_io_be_uint32(smtaTag.value);
#ifdef __MMFILE_TEST_MODE__
	debug_msg("Len : 0x%x", smtaTag.length);
	debug_msg("Saut : 0x%x 0x%x 0x%x 0x%x", smtaTag.saut[0], smtaTag.saut[1], smtaTag.saut[2], smtaTag.saut[3]);
	debug_msg("Value : 0x%x", smtaTag.value);
#endif

	if (smtaTag.saut[0] == 's'
		&& smtaTag.saut[1] == 'a'
		&& smtaTag.saut[2] == 'u'
		&& smtaTag.saut[3] == 't') {
		if (smtaTag.value == 0x01) {
			#ifdef __MMFILE_TEST_MODE__
			debug_msg("This has saut tag and valid value");
			#endif
			formatContext->smta = 1;
		} else {
			debug_error("This has saut tag and but invalid value");
			goto exception;
		}
	} else {
		debug_error("This hasn't saut tag and valid value");
		goto exception;
	}

	return MMFILE_UTIL_SUCCESS;

exception:
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);

	return MMFILE_UTIL_FAIL;
}

static int GetValueFromCDISTagBox(MMFileFormatContext *formatContext, MMFileIOHandle *fp, MMFILE_MP4_BASIC_BOX_HEADER *basic_header)
{
	unsigned int value = 0;
	int readed = 0;

	if (!formatContext || !fp || !basic_header) {
		debug_error ("invalid param\n");
		return MMFILE_UTIL_FAIL;
	}

	readed = mmfile_read (fp, (unsigned char *)&value, sizeof(unsigned int));
	if (readed != sizeof(unsigned int)) {
		debug_error ("read cdis tag header fail\n");
		goto exception;
	}

	value = mmfile_io_be_uint32(value);

#ifdef __MMFILE_TEST_MODE__
	debug_msg("Value : 0x%x", value);
#endif

	if (value == 0x01) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg("This has cdis tag and valid value");
		#endif
		formatContext->cdis = 1;
	} else {
		debug_error("This has cdis tag and but invalid value");
		goto exception;
	}

	return MMFILE_UTIL_SUCCESS;

exception:
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);

	return MMFILE_UTIL_FAIL;
}

static int GetTagFromMetaBox (MMFileFormatContext *formatContext, MMFileIOHandle *fp, MMFILE_MP4_BASIC_BOX_HEADER *basic_header)
{
	int readed = 0;
	MMFILE_MP4_BASIC_BOX_HEADER hdlrBoxHeader = {0,};
	MMFILE_MP4_BASIC_BOX_HEADER id3v2BoxHeader = {0,};
	MMFILE_3GP_ID3V2_BOX id3v2Box = {0,};
	AvFileContentInfo tagInfo = {0,};
	unsigned char tagVersion = 0;
	bool versionCheck = false;
	int id3v2Len = 0;
	unsigned int meta_version = 0;
	MMFILE_3GP_HANDLER_BOX hdlrBox = {0,};
	int encSize = 0;
	int id3_meta = 0;
#ifdef ENABLE_ITUNES_META /* We don't support itunes meta now. so this is not defined yet */
	int iTunes_meta = 0;
#endif

	/* meta box */
	readed = mmfile_read (fp, (unsigned char *)&meta_version, 4);
	if (readed != 4) {
		debug_error ("read meta box version\n");
		goto exception;
	}

	/* hdlr box */
	readed = mmfile_read (fp, (unsigned char *)&hdlrBoxHeader, MMFILE_MP4_BASIC_BOX_HEADER_LEN);
	if (readed != MMFILE_MP4_BASIC_BOX_HEADER_LEN) {
		debug_error ("read hdlr box header\n");
		goto exception;
	}

	if (hdlrBoxHeader.type != FOURCC ('h', 'd', 'l', 'r')) {
		debug_warning ("meta type is not hdlr\n");
		goto exception;
	}

	hdlrBoxHeader.size = mmfile_io_be_uint32 (hdlrBoxHeader.size);
	hdlrBoxHeader.type = mmfile_io_le_uint32 (hdlrBoxHeader.type);

	readed = mmfile_read (fp, (unsigned char *)&hdlrBox, MMFILE_3GP_HANDLER_BOX_LEN);
	if (readed != MMFILE_3GP_HANDLER_BOX_LEN) {
		debug_error ("read hdlr box\n");
		goto exception;
	}

	hdlrBox.handler_type = mmfile_io_le_uint32 (hdlrBox.handler_type);

	/**
	 * check tag type (ID3v2 or iTunes)
	 */
	if (hdlrBox.handler_type == FOURCC ('I', 'D', '3', '2')) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("ID3v2 tag detected.\n");
		#endif

		id3_meta = 1;
#ifdef ENABLE_ITUNES_META
		iTunes_meta = 0;
#endif
	} else if (hdlrBox.handler_type == FOURCC ('m', 'd', 'i', 'r') &&
				mmfile_io_le_uint32 (hdlrBox.reserved[0]) == FOURCC ('a', 'p', 'p', 'l')) {

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("Apple iTunes tag detected by mdir.\n");
		#endif

#ifdef ENABLE_ITUNES_META
		iTunes_meta = 1;
#endif
	} else {
		debug_warning ("unknown meta type. 4CC:[%c%c%c%c]\n", ((char*)&hdlrBox.handler_type)[0],
																((char*)&hdlrBox.handler_type)[1],
																((char*)&hdlrBox.handler_type)[2],
																((char*)&hdlrBox.handler_type)[3]);
		//goto exception;
	}

#ifdef ENABLE_ITUNES_META
	if(!id3_meta && !iTunes_meta) {
		/*Check ilst.
		APPLE meta data for iTunes reader = 'mdir.' so if handler type is 'mdir', this content may has itunes meta.
		most of contents has 'mdir' + 'appl'. but some contents just has 'mdir'
		but 'ilst' is meta for iTunes. so find 'ilst' is more correct to check if this contents has iTunes meta or not.*/

		char *ilst_box = "ilst";
		int buf_size = strlen(ilst_box);

		unsigned char read_buf[buf_size+1];
		memset(read_buf, 0x00, buf_size+1);

		/* skip hdlr box */
		mmfile_seek (fp, hdlrBoxHeader.size - MMFILE_MP4_BASIC_BOX_HEADER_LEN - MMFILE_3GP_HANDLER_BOX_LEN +4, SEEK_CUR);	//+4 is hdlr size field

		readed = mmfile_read (fp, read_buf, buf_size);	// to find 'ilst'
		if (readed != buf_size) {
			debug_error ("read fail [%d]\n", readed);
			goto exception;
		}

		if(read_buf[0] == 'i' && read_buf[1] == 'l' && read_buf[2] == 's' && read_buf[3] == 't') {
			#ifdef __MMFILE_TEST_MODE__
				debug_msg ("Apple iTunes tag detected by ilst.\n");
			#endif

			iTunes_meta = 1;
		}
	}
#endif

#ifdef ENABLE_ITUNES_META
	if (iTunes_meta) {
		/**
		 * iTunes (Cover[?ovr] & Track[trkn] only extract!) + Genre/Artist : Added 2010.10.27,28
		 *
		 *  4cc   : 4byte
		 *        : 4byte	size
		 * 'data' : 4byte
		 *        : 4byte	type
		 *        : 4byte	unknown
		 */
#define _ITUNES_READ_BUF_SZ		20
#define _ITUNES_TRACK_NUM_SZ	4
#define _ITUNES_GENRE_NUM_SZ	4
#define _ITUNES_COVER_TYPE_JPEG	13
#define _ITUNES_COVER_TYPE_PNG		14

		unsigned char read_buf[_ITUNES_READ_BUF_SZ];
		int i = 0;
		int cover_sz = 0, cover_type = 0, cover_found = 0;
		int track_num = 0, track_found = 0;
		int genre_index = 0, genre_found = 0;
		int artist_sz = 0, artist_found = 0;
		int limit = basic_header->size - hdlrBoxHeader.size;
		long long cover_offset = 0, track_offset =0 , genre_offset = 0, artist_offset = 0;

		for (i = 0, cover_found = 0, track_found = 0, genre_found = 0, artist_found = 0; i < limit && (cover_found == 0 || track_found == 0 || genre_found == 0 || artist_found == 0) ; i++) {
			readed = mmfile_read (fp, read_buf, _ITUNES_READ_BUF_SZ);
			if (readed != _ITUNES_READ_BUF_SZ)
				goto exception;

/*ffmpeg extract artist, tracknum, genre and cover image. see mov_read_udta_string().
but ffmpeg does not support strange cover image.
only support covr type 0xd(JPEG), 0xe(PNG), 0x1b(BMP). but we support other type*/
#if 0
			/**
			 * Artist : Added 2010.10.28
			 */
			if (artist_found == 0 &&
				read_buf[0] == 0xa9 && read_buf[1] == 'A' && read_buf[2] == 'R' && read_buf[3] == 'T' &&
				read_buf[8] == 'd' && read_buf[9] == 'a' && read_buf[10] == 't' && read_buf[11] == 'a') {

				artist_found = 1;
				artist_offset = mmfile_tell (fp);
				artist_sz = mmfile_io_be_uint32 (*(int*)(read_buf + 4)) - 16; /* atom len(4)+data(4)+atom verion(1)+flag(3)+null(4) = 16 */

				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("----------------------------------- artist found, offset=[%lld], size=[%d]\n", artist_offset, artist_sz);
				#endif
			}

			/**
			 * Track number
			 */
			if (track_found == 0 &&
				read_buf[0] == 't' && read_buf[1] == 'r' && read_buf[2] == 'k' && read_buf[3] == 'n' &&
				read_buf[8] == 'd' && read_buf[9] == 'a' && read_buf[10] == 't' && read_buf[11] == 'a') {

				track_found = 1;
				track_offset = mmfile_tell (fp);

				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("----------------------------------- Track found, offset=[%lld]\n", track_offset);
				#endif
			}

			/**
			 * Genre : Added 2010.10.27
			 */
			/*ffmpeg extract genre but only (0xa9,'g','e','n'). see mov_read_udta_string()*/
			if (genre_found == 0 &&
				read_buf[0] == 'g' && read_buf[1] == 'n' && read_buf[2] == 'r' && read_buf[3] == 'e' &&
				read_buf[8] == 'd' && read_buf[9] == 'a' && read_buf[10] == 't' && read_buf[11] == 'a') {

				genre_found = 1;
				genre_offset = mmfile_tell (fp);

				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("----------------------------------- genre found, offset=[%lld]\n", genre_offset);
				#endif
			}
#endif

			/**
			 * Cover image
			 */

			if (cover_found == 0 &&
				read_buf[0] == 'c' && read_buf[1] == 'o' && read_buf[2] == 'v' && read_buf[3] == 'r' &&
				read_buf[8] == 'd' && read_buf[9] == 'a' && read_buf[10] == 't' && read_buf[11] == 'a') {

				cover_found = 1;
				cover_sz = mmfile_io_be_uint32 (*(int*)(read_buf + 4)) - 12;
				cover_type = mmfile_io_be_uint32 (*(int*)(read_buf + 12));

				cover_offset = mmfile_tell (fp);

				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("----------------------------------- cover_found found,  offset=[%lld]\n", cover_offset);
				#endif
			}

			mmfile_seek (fp, -(_ITUNES_READ_BUF_SZ - 1), SEEK_CUR);	/*FIXME: poor search*/
		} /*loop*/

/*ffmpeg extract artist, tracknum, excep cover image. see mov_read_udta_string()*/
#if 0
		if (artist_found) {
			if (artist_sz > 0) {
				mmfile_seek (fp, artist_offset, SEEK_SET);

				if (formatContext->artist) {
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ("----------------------------------- previous artist was [%s] \n", formatContext->artist);
					#endif
					free (formatContext->artist);
				}
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("----------------------------------- new artist will be allocated with size (len+1) [%d] \n", artist_sz+1);
				#endif
				formatContext->artist = mmfile_malloc (artist_sz+1);

				if (formatContext->artist) {
					readed = mmfile_read (fp, (unsigned char*)formatContext->artist, artist_sz);
					formatContext->artist[artist_sz] = '\0';

					#ifdef __MMFILE_TEST_MODE__
					debug_msg ("----------------------------------- new artist is [%s] \n", formatContext->artist);
					#endif

					if (readed != artist_sz) {
						debug_error ("failed to read. ret = %d, in = %d\n", readed, artist_sz);
						mmfile_free (formatContext->artist);
					}
				}
			}
		}

		if (track_found) {
			mmfile_seek (fp, track_offset, SEEK_SET);
			readed = mmfile_read (fp, read_buf, _ITUNES_TRACK_NUM_SZ);
			if (readed != _ITUNES_TRACK_NUM_SZ) {
				debug_error ("failed to read. ret = %d, in = %d\n", readed, _ITUNES_TRACK_NUM_SZ);
			} else {
				track_num = mmfile_io_be_uint32 (*(int*)read_buf);
				if (!formatContext->tagTrackNum) {
					memset (read_buf, 0x00, _ITUNES_READ_BUF_SZ);
					snprintf ((char*)read_buf, sizeof(read_buf), "%d", track_num);
					formatContext->tagTrackNum = mmfile_strdup ((const char*)read_buf);
				}
			}
		}

		if (genre_found) {
			mmfile_seek (fp, genre_offset, SEEK_SET);
			readed = mmfile_read (fp, read_buf, _ITUNES_GENRE_NUM_SZ);
			if (readed != _ITUNES_GENRE_NUM_SZ) {
				debug_error ("failed to read. ret = %d, in = %d\n", readed, _ITUNES_GENRE_NUM_SZ);
			} else {
				genre_index = mmfile_io_be_uint16 (*(int*)read_buf);
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("genre index=[%d] \n", genre_index);
				#endif
				if (genre_index > 0 && genre_index < GENRE_COUNT)	{
					if (!formatContext->genre) {
						memset (read_buf, 0x00, _ITUNES_READ_BUF_SZ);
						snprintf ((char*)read_buf, sizeof(read_buf),"%s", MpegAudio_Genre[genre_index-1]);
						#ifdef __MMFILE_TEST_MODE__
						debug_msg ("genre string=[%s] \n", read_buf);
						#endif
						formatContext->genre = mmfile_strdup ((const char*)read_buf);
					}
				}
			}
		}
#endif

/*
	1) below spec is in "iTunes Package Asset Specification 4.3" published by apple.
	Music Cover Art Image Profile
	- TIFF with ".tif" extension (32-bit uncompressed), JPEG with ".jpg" extension (quality unconstrained), or PNG with ".png" extension
	- RGB (screen standard)
	- Minimum size of 600 x 600 pixels
	- Images must be at least 72 dpi

	2)I found below info from google.
	cover image flag : JPEG (13, 0xd), PNG (14, 0xe)

	3)So, FIXME when cover image format is tif!
*/
		if (cover_found) {
			if (cover_sz > 0) {
				mmfile_seek (fp, cover_offset, SEEK_SET);

				formatContext->artwork = mmfile_malloc (cover_sz);
				formatContext->artworkSize = cover_sz;
				if(cover_type == _ITUNES_COVER_TYPE_JPEG) {
					formatContext->artworkMime = mmfile_strdup("image/jpeg");
				} else if(cover_type == _ITUNES_COVER_TYPE_PNG) {
					formatContext->artworkMime = mmfile_strdup("image/png");
				/*} else if(cover_type == _ITUNES_COVER_TYPE_TIF) {
					formatContext->artworkMime = mmfile_strdup("image/tif");*/
				} else {
					debug_warning("Not proper cover image type, but set to jpeg. cover_type[%d]", cover_type);
					formatContext->artworkMime = mmfile_strdup("image/jpeg");
				}

				if (formatContext->artwork) {
					readed = mmfile_read (fp, formatContext->artwork, cover_sz);
					if (readed != cover_sz) {
						debug_error ("failed to read. ret = %d, in = %d\n", readed, cover_sz);
						mmfile_free (formatContext->artwork);
						formatContext->artworkSize = 0;
						mmfile_free (formatContext->artworkMime);
					}
				}
			}
		}

		/*reset seek position*/
		mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);

		return MMFILE_UTIL_SUCCESS;

	} else
#endif
	if(id3_meta) {
		/**
		 * ID3v2
		 */
		/* skip hdlr box name */
		mmfile_seek (fp, hdlrBoxHeader.size - MMFILE_MP4_BASIC_BOX_HEADER_LEN - MMFILE_3GP_HANDLER_BOX_LEN, SEEK_CUR);

		/* id3 tag box */
		readed = mmfile_read (fp, (unsigned char *)&id3v2BoxHeader, MMFILE_MP4_BASIC_BOX_HEADER_LEN);
		if (readed != MMFILE_MP4_BASIC_BOX_HEADER_LEN)
		{
			debug_error ("read id3v2 box header\n");
			goto exception;
		}

		id3v2BoxHeader.size = mmfile_io_be_uint32 (id3v2BoxHeader.size);
		id3v2BoxHeader.type = mmfile_io_le_uint32 (id3v2BoxHeader.type);

		if (id3v2BoxHeader.type != FOURCC ('I', 'D', '3', '2'))
		{
			debug_warning ("meta type is not id3v2\n");
			goto exception;
		}

		readed = mmfile_read (fp, (unsigned char *)&id3v2Box, MMFILE_3GP_ID3V2_BOX_LEN);
		if (readed != MMFILE_3GP_ID3V2_BOX_LEN)
		{
			debug_error ("read id3v2 box\n");
			goto exception;
		}

		id3v2Len = id3v2BoxHeader.size - MMFILE_MP4_BASIC_BOX_HEADER_LEN - MMFILE_3GP_ID3V2_BOX_LEN;

		id3v2Box.id3v2Data = mmfile_malloc (id3v2Len);
		if (!id3v2Box.id3v2Data)
		{
			debug_error ("malloc id3tag data error\n");
			goto exception;
		}

		readed = mmfile_read (fp, (unsigned char *)id3v2Box.id3v2Data, id3v2Len);
		if (readed != id3v2Len)
		{
			debug_error ("read id3tag data error\n");
			goto exception;
		}

		/* check id3v2 */
		if (!IS_ID3V2_TAG(id3v2Box.id3v2Data))
		{
			debug_error ("it is not id3tag\n");
			goto exception;
		}

		if (id3v2Box.id3v2Data[3] == 0xFF ||  id3v2Box.id3v2Data[4] == 0xFF ||
			id3v2Box.id3v2Data[6] >= 0x80 ||  id3v2Box.id3v2Data[7] >= 0x80 ||
		   id3v2Box.id3v2Data[8] >= 0x80 ||  id3v2Box.id3v2Data[9] >= 0x80)
		{
			debug_error ("it is not valid id3tag\n");
			goto exception;
		}

		tagVersion = id3v2Box.id3v2Data[3];
		if ( tagVersion > 4)
		{
			debug_error ("tag vesion is too high\n");
			goto exception;
		}

		encSize = mmfile_io_le_uint32(&id3v2Box.id3v2Data[6]);
		tagInfo.tagV2Info.tagLen = MP3_TAGv2_HEADER_LEN;
		tagInfo.tagV2Info.tagLen += (((encSize & 0x0000007F) >> 0) | ((encSize & 0x00007F00) >> 1) | ((encSize & 0x007F0000) >> 2) | ((encSize & 0x7F000000) >> 3));
		tagInfo.tagV2Info.tagVersion = tagVersion;
		tagInfo.fileLen = id3v2Len;

		/* set id3v2 data to formatContext */
		switch (tagVersion)
		{
			case 2:
			{
				versionCheck = mm_file_id3tag_parse_v222(&tagInfo, id3v2Box.id3v2Data);
				break;
			}
			case 3:
			{
				versionCheck = mm_file_id3tag_parse_v223(&tagInfo, id3v2Box.id3v2Data);
				break;
			}
			case 4:
			{
				versionCheck = mm_file_id3tag_parse_v224(&tagInfo, id3v2Box.id3v2Data);
				break;
			}
			case 1:
			default:
			{
				debug_error ("tag vesion is not support\n");
				versionCheck = false;
				break;
			}
		}

		if (versionCheck == false)
		{
			debug_error ("tag parsing is fail\n");
			goto exception;
		}

		if (!formatContext->title)          formatContext->title = mmfile_strdup((const char*)tagInfo.pTitle);
		if (!formatContext->artist)         formatContext->artist = mmfile_strdup((const char*)tagInfo.pArtist);
		if (!formatContext->author)         formatContext->author = mmfile_strdup((const char*)tagInfo.pAuthor);
		if (!formatContext->copyright)      formatContext->copyright = mmfile_strdup((const char*)tagInfo.pCopyright);
		if (!formatContext->comment)        formatContext->comment = mmfile_strdup((const char*)tagInfo.pComment);
		if (!formatContext->album)          formatContext->album = mmfile_strdup((const char*)tagInfo.pAlbum);
		if (!formatContext->album_artist)   formatContext->album_artist = mmfile_strdup((const char*)tagInfo.pAlbum_Artist);
		if (!formatContext->year)           formatContext->year = mmfile_strdup((const char*)tagInfo.pYear);
		if (!formatContext->genre)          formatContext->genre = mmfile_strdup((const char*)tagInfo.pGenre);
		if (!formatContext->tagTrackNum)    formatContext->tagTrackNum = mmfile_strdup((const char*)tagInfo.pTrackNum);
		if (!formatContext->composer)       formatContext->composer = mmfile_strdup((const char*)tagInfo.pComposer);
		if (!formatContext->classification) formatContext->classification = mmfile_strdup((const char*)tagInfo.pContentGroup);
		if (!formatContext->conductor)      formatContext->conductor = mmfile_strdup((const char*)tagInfo.pConductor);

		formatContext->artwork = mmfile_malloc (tagInfo.imageInfo.imageLen);
		if ((tagInfo.imageInfo.imageLen > 0) && formatContext->artwork)
		{
			formatContext->artworkSize = tagInfo.imageInfo.imageLen;
			memcpy (formatContext->artwork, tagInfo.imageInfo.pImageBuf, tagInfo.imageInfo.imageLen);
		}

		mm_file_free_AvFileContentInfo (&tagInfo);
		mmfile_free (id3v2Box.id3v2Data);

		/*reset seek position*/
		mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);

		return MMFILE_UTIL_SUCCESS;

	}


exception:
	mmfile_seek (fp, basic_header->start_offset + basic_header->size, SEEK_SET);
	mmfile_free (id3v2Box.id3v2Data);

	return MMFILE_UTIL_FAIL;
}


EXPORT_API int MMFileUtilGetMetaDataFromMP4 (MMFileFormatContext * formatContext)
{
	MMFileIOHandle *fp = NULL;
	int ret = 0;
	int readed;

	ret = mmfile_open (&fp, formatContext->uriFileName, MMFILE_RDONLY);
	if(ret == MMFILE_UTIL_FAIL) {
		debug_error ("error: mmfile_open\n");
		goto exit;
	}

	MMFILE_MP4_BASIC_BOX_HEADER basic_header = {0,};
	basic_header.start_offset = mmfile_tell (fp);

	while ( (ret != MMFILE_UTIL_FAIL) && ((readed = mmfile_read (fp, (unsigned char *)&basic_header, MMFILE_MP4_BASIC_BOX_HEADER_LEN)) > 0 ) ) {
		basic_header.size = mmfile_io_be_uint32 (basic_header.size);
		basic_header.type = mmfile_io_le_uint32 (basic_header.type);

		if (basic_header.size == 0) {
			debug_warning ("header is invalid.\n");
			basic_header.size = readed;
			basic_header.type = 0;
		}

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("START_OFFSET:[%lld] SIZE:[%d Byte] 4CC:[%c%c%c%c]\n",
					basic_header.start_offset, basic_header.size,
					((char*)&basic_header.type)[0], ((char*)&basic_header.type)[1],
					((char*)&basic_header.type)[2], ((char*)&basic_header.type)[3]);
		#endif

		switch (basic_header.type) {
			case FOURCC ('m', 'o', 'o', 'v'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [moov] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				break;
			}
			case FOURCC ('u', 'd', 't', 'a'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [udat] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				break;
			}
			/////////////////////////////////////////////////////////////////
			//                  Extracting Tag Data                        //
			/////////////////////////////////////////////////////////////////
			case FOURCC ('t', 'i', 't', 'l'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [titl] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetStringFromTextTagBox (formatContext, fp, &basic_header, eMMFILE_3GP_TAG_TITLE);
				break;
			}
			case FOURCC ('d', 's', 'c', 'p'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [dscp] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetStringFromTextTagBox (formatContext, fp, &basic_header, eMMFILE_3GP_TAG_CAPTION);
				break;
			}
			case FOURCC ('c', 'p', 'r', 't'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [cprt] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetStringFromTextTagBox (formatContext, fp, &basic_header, eMMFILE_3GP_TAG_COPYRIGHT);
				break;
			}
			case FOURCC ('p', 'e', 'r', 'f'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [perf] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetStringFromTextTagBox (formatContext, fp, &basic_header, eMMFILE_3GP_TAG_PERFORMER );
				break;
			}
			case FOURCC ('a', 'u', 't', 'h'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [auth] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetStringFromTextTagBox (formatContext, fp, &basic_header, eMMFILE_3GP_TAG_AUTHOR);
				break;
			}
			case FOURCC ('g', 'n', 'r', 'e'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [gnre] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetStringFromTextTagBox (formatContext, fp, &basic_header, eMMFILE_3GP_TAG_GENRE);
				break;
			}
			case FOURCC ('a', 'l', 'b', 'm'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [albm] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetAlbumFromAlbumTagBox (formatContext, fp, &basic_header);
				break;
			}
			case FOURCC ('y', 'r', 'r', 'c'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [yrrc] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetYearFromYearTagBox (formatContext, fp, &basic_header);
				break;
			}
			case FOURCC ('r', 't', 'n', 'g'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [rtng] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetRatingFromRatingTagBox (formatContext, fp, &basic_header); /* not use */
				break;
			}
			case FOURCC ('c', 'l', 's', 'f'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [clsf] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetClassficationFromClsfTagBox (formatContext, fp, &basic_header);
				break;
			}
			case FOURCC ('k', 'y', 'w', 'd'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [kywd] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				ret = mmfile_seek (fp, basic_header.start_offset + basic_header.size, SEEK_SET);
				break;
			}
			case FOURCC ('l', 'o', 'c', 'i'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [loci] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetLocationFromLociTagBox (formatContext, fp, &basic_header);
				break;
			}
			/* Check smta in user data field to be compatible with android */
			case FOURCC ('s', 'm', 't', 'a'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [smta] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetSAUTInfoFromSMTATagBox (formatContext, fp, &basic_header);
				break;
			}
			/* Check smta in user data field to be compatible with android */
			case FOURCC ('c', 'd', 'i', 's'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [smta] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetValueFromCDISTagBox (formatContext, fp, &basic_header);
				break;
			}
			/////////////////////////////////////////////////////////////////
			//                  Extracting ID3 Tag Data                    //
			/////////////////////////////////////////////////////////////////
			case FOURCC ('m', 'e', 't', 'a'): {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("MPEG4: [meta] SIZE: [%d]Byte\n", basic_header.size);
				#endif
				GetTagFromMetaBox (formatContext, fp, &basic_header);
				break;
			}
			default: {
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ("4CC: Not Support.. so skip it\n");
				#endif
				ret = mmfile_seek (fp, basic_header.start_offset + basic_header.size, SEEK_SET);
				break;
			}
		}

		if (ret == MMFILE_UTIL_FAIL) {
			debug_error("mmfile operation is error\n");
			ret = -1;
			goto exit;
		}

		basic_header.start_offset = mmfile_tell (fp);
	}

exit:
	mmfile_close (fp);
	return ret;
}

static char * get_string(const char *buf, int buf_size, int *bytes_written)
{
	int i = 0, c = 0;
	char *q = NULL;
	char str[512] = {0, };

	q = str;
	for(i = 0; i < buf_size; i++) {
		c = buf[i];
		if (c == '\0')
			break;
		if ((q - str) >= sizeof(str) - 1)
		break;
		*q++ = c;
	}
	*q = '\0';

	if(strlen(str) > 0) {
		*bytes_written = strlen(str);
		return strdup(str);
	} else {
		*bytes_written = 0;
		return NULL;
	}
}

static bool is_numeric(const char *buf, int buf_size)
{
	int idx = 0;
	bool is_num = true;

	for(idx = 0; idx < buf_size; idx++) {
		if(isdigit((int)buf[idx])) {
			continue;
		}
		else {
			is_num = false;
			break;
		}
	}

	return is_num;
}

static bool make_characterset_array(char ***charset_array)
{
	char *locale = MMFileUtilGetLocale (NULL);

	*charset_array = malloc(sizeof(char*) * AV_ID3V2_MAX);
	memset(*charset_array, 0x0, sizeof(charset_array));
	if (locale != NULL) {
		(*charset_array)[AV_ID3V2_ISO_8859] = strdup(locale);
	} else {
		debug_error("get locale failed");
		(*charset_array)[AV_ID3V2_ISO_8859] = NULL;
	}

	(*charset_array)[AV_ID3V2_UTF16] = strdup("UCS2");
	(*charset_array)[AV_ID3V2_UTF16_BE] = strdup("UTF16-BE");
	(*charset_array)[AV_ID3V2_UTF8] = strdup("UTF-8");

	return true;
}

static bool release_characterset_array(char **charset_array)
{
	int i = 0;

	for (i = 0; i < AV_ID3V2_MAX; i++) {
		if (charset_array[i] != NULL)
			free(charset_array[i]);
	}

	free(charset_array);

	return true;
}

static void init_content_info(AvFileContentInfo* pInfo)
{
	pInfo->tagV2Info.bTitleMarked = false;
	pInfo->tagV2Info.bArtistMarked= false;
	pInfo->tagV2Info.bAlbumMarked= false;
	pInfo->tagV2Info.bAlbum_ArtistMarked= false;
	pInfo->tagV2Info.bYearMarked= false;
	pInfo->tagV2Info.bDescriptionMarked= false;
	pInfo->tagV2Info.bGenreMarked= false;
	pInfo->tagV2Info.bTrackNumMarked= false;
	pInfo->tagV2Info.bEncByMarked= false;
	pInfo->tagV2Info.bURLMarked= false;
	pInfo->tagV2Info.bCopyRightMarked= false;
	pInfo->tagV2Info.bOriginArtistMarked= false;
	pInfo->tagV2Info.bComposerMarked= false;
	pInfo->tagV2Info.bImageMarked= false;

	pInfo->tagV2Info.bRecDateMarked= false;
	pInfo->tagV2Info.bContentGroupMarked= false;

	pInfo->tagV2Info.bUnsyncLyricsMarked = false;
	pInfo->tagV2Info.bSyncLyricsMarked = false;
	pInfo->tagV2Info.bConductorMarked = false;
	pInfo->tagV2Info.bGenreUTF16 = false;

	pInfo->imageInfo.bURLInfo = false;
	pInfo->imageInfo.pImageBuf = NULL;
	pInfo->imageInfo.imageLen = 0;
}

EXPORT_API
bool mm_file_id3tag_parse_v110(AvFileContentInfo* pInfo,  unsigned char *buffer)
{
	const char *locale = MMFileUtilGetLocale (NULL);

#ifdef __MMFILE_TEST_MODE__
	debug_msg ("ID3tag v110--------------------------------------------------------------\n");
#endif

	if(pInfo->tagV2Info.bTitleMarked == false) {
		pInfo->pTitle = mmfile_string_convert ((const char*)&buffer[3], MP3_ID3_TITLE_LENGTH, "UTF-8", locale, NULL, (unsigned int*)&pInfo->titleLen);
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "pInfo->pTitle returned =(%s), pInfo->titleLen(%d)\n", pInfo->pTitle, pInfo->titleLen);
		#endif

		if(pInfo->pTitle == NULL) {	/*Use same logic with ffmpeg*/
			pInfo->pTitle = get_string((const char*)&buffer[3], MP3_ID3_TITLE_LENGTH, (int*)&pInfo->titleLen);
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ( "pInfo->pTitle returned =(%s), pInfo->titleLen(%d)\n", pInfo->pTitle, pInfo->titleLen);
			#endif
		}
	}

	if(pInfo->tagV2Info.bArtistMarked == false) {
		pInfo->pArtist = mmfile_string_convert ((const char*)&buffer[33], MP3_ID3_ARTIST_LENGTH, "UTF-8", locale, NULL, (unsigned int*)&pInfo->artistLen);
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "pInfo->pArtist returned =(%s), pInfo->artistLen(%d)\n", pInfo->pArtist, pInfo->artistLen);
		#endif

		if(pInfo->pArtist == NULL) {	/*Use same logic with ffmpeg*/
			pInfo->pArtist = get_string((const char*)&buffer[33], MP3_ID3_ARTIST_LENGTH, (int*)&pInfo->artistLen);
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ( "pInfo->pArtist returned =(%s), pInfo->artistLen(%d)\n", pInfo->pArtist, pInfo->artistLen);
			#endif
		}
	}

	if(pInfo->tagV2Info.bAlbumMarked == false) {
		pInfo->pAlbum = mmfile_string_convert ((const char*)&buffer[63], MP3_ID3_ALBUM_LENGTH, "UTF-8", locale, NULL, (unsigned int*)&pInfo->albumLen);
		#ifdef __MMFILE_TEST_MODE__
		debug_msg (  "pInfo->pAlbum returned =(%s), pInfo->albumLen(%d)\n", pInfo->pAlbum, pInfo->albumLen);
		#endif

		if(pInfo->pAlbum == NULL) {	/*Use same logic with ffmpeg*/
			pInfo->pAlbum = get_string((const char*)&buffer[63], MP3_ID3_ALBUM_LENGTH, (int*)&pInfo->albumLen);
			#ifdef __MMFILE_TEST_MODE__
			debug_msg (  "pInfo->pAlbum returned =(%s), pInfo->albumLen(%d)\n", pInfo->pAlbum, pInfo->albumLen);
			#endif
		}
	}

	if(pInfo->tagV2Info.bYearMarked == false) {

		pInfo->pYear = mmfile_string_convert ((const char*)&buffer[93], MP3_ID3_YEAR_LENGTH, "UTF-8", locale, NULL, (unsigned int*)&pInfo->yearLen);
		#ifdef __MMFILE_TEST_MODE__
		debug_msg (  "pInfo->pYear returned =(%s), pInfo->yearLen(%d)\n", pInfo->pYear, pInfo->yearLen);
		#endif

		if(pInfo->pYear == NULL) {	/*Use same logic with ffmpeg*/
			pInfo->pYear = get_string((const char*)&buffer[93], MP3_ID3_YEAR_LENGTH, (int*)&pInfo->yearLen);
			#ifdef __MMFILE_TEST_MODE__
			debug_msg (  "pInfo->pYear returned =(%s), pInfo->yearLen(%d)\n", pInfo->pYear, pInfo->yearLen);
			#endif
		}
	}

	if(pInfo->tagV2Info.bDescriptionMarked == false) {
		pInfo->pComment = mmfile_string_convert ((const char*)&buffer[97], MP3_ID3_DESCRIPTION_LENGTH, "UTF-8", locale, NULL, (unsigned int*)&pInfo->commentLen);
		#ifdef __MMFILE_TEST_MODE__
		debug_msg (  "pInfo->pComment returned =(%s), pInfo->commentLen(%d)\n", pInfo->pComment, pInfo->commentLen);
		#endif

		if(pInfo->pComment == NULL) {	/*Use same logic with ffmpeg*/
			pInfo->pComment = get_string((const char*)&buffer[97], MP3_ID3_DESCRIPTION_LENGTH, (int*)&pInfo->commentLen);
			#ifdef __MMFILE_TEST_MODE__
			debug_msg (  "pInfo->pComment returned =(%s), pInfo->commentLen(%d)\n", pInfo->pComment, pInfo->commentLen);
			#endif
		}
	}

	if(pInfo->tagV2Info.bTrackNumMarked== false) {
		pInfo->pTrackNum = mmfile_malloc (5);
		pInfo->pTrackNum[4] = 0;
		snprintf(pInfo->pTrackNum, 4, "%04d", (int)buffer[126]);
		pInfo->tracknumLen = strlen(pInfo->pTrackNum);
		#ifdef __MMFILE_TEST_MODE__
		debug_msg (  "pInfo->pTrackNum returned =(%s), pInfo->tracknumLen(%d)\n", pInfo->pTrackNum, pInfo->tracknumLen);
		#endif
	}

	if(pInfo->tagV2Info.bGenreMarked == false) {
		pInfo->genre = buffer[127];
		#ifdef __MMFILE_TEST_MODE__
		debug_msg (  "pInfo->genre returned genre number (%d)\n", pInfo->genre);
		#endif
	}

	return true;
}

EXPORT_API
bool mm_file_id3tag_parse_v222(AvFileContentInfo* pInfo, unsigned char *buffer)
{
	unsigned long taglen = 0;
	unsigned long needToloopv2taglen;
	unsigned long oneFrameLen = 0;
	unsigned long v2numOfFrames = 0;
	unsigned long curPos = 0;
	char CompTmp[4];
	char *pExtContent = NULL;
	unsigned long purelyFramelen = 0;
	int inx=0, encodingOffSet=0, realCpyFrameNum=0,
		checkImgMimeTypeMax=0, checkImgExtMax=0,
		imgstartOffset=0, tmp = 0;

	int textEncodingType = 0;

	char **charset_array = NULL;

	make_characterset_array(&charset_array);

	init_content_info(pInfo);

	taglen = pInfo->tagV2Info.tagLen;
	needToloopv2taglen = taglen - MP3_TAGv2_HEADER_LEN;
	curPos = MP3_TAGv2_HEADER_LEN;
	
#ifdef __MMFILE_TEST_MODE__
	debug_msg ("ID3tag v222--------------------------------------------------------------\n");
#endif
	if(needToloopv2taglen -MP3_TAGv2_22_TXT_HEADER_LEN > MP3_TAGv2_22_TXT_HEADER_LEN) {
		v2numOfFrames = 1;
		while(needToloopv2taglen > MP3_TAGv2_22_TXT_HEADER_LEN)
		{
			if((buffer[curPos] < '0' || buffer[curPos] > 'Z') || (buffer[curPos+1] < '0' || buffer[curPos+1] > 'Z')
				|| (buffer[curPos+2] < '0' || buffer[curPos+2] > 'Z'))
				break;

			memcpy(CompTmp, &buffer[curPos], 3);

			CompTmp[3] = 0;
			oneFrameLen = MP3_TAGv2_22_TXT_HEADER_LEN;
			oneFrameLen += (unsigned long)buffer[3+curPos] << 16 | (unsigned long)buffer[4+curPos] << 8
				| (unsigned long)buffer[5+curPos];
			if(oneFrameLen > taglen-curPos)
				break;
			purelyFramelen = oneFrameLen - MP3_TAGv2_22_TXT_HEADER_LEN;
			curPos +=MP3_TAGv2_22_TXT_HEADER_LEN;

			if(oneFrameLen > MP3_TAGv2_22_TXT_HEADER_LEN && purelyFramelen <= taglen-curPos)
			{
				curPos += purelyFramelen;

				if(buffer[curPos-purelyFramelen] == 0x00)
				{
					encodingOffSet = 1;
					textEncodingType = AV_ID3V2_ISO_8859;
				}
				else if(buffer[curPos-purelyFramelen] == 0x01)
				{
					encodingOffSet = 1;
					textEncodingType = AV_ID3V2_UTF16;
				}

				//in order to deliver valid string to MP
				while((buffer[curPos-purelyFramelen+encodingOffSet] < 0x20) && (encodingOffSet < purelyFramelen))
					encodingOffSet++;

				if(encodingOffSet < purelyFramelen)
				{
					realCpyFrameNum = purelyFramelen - encodingOffSet;
					pExtContent = mmfile_malloc (realCpyFrameNum+3);
					memset(pExtContent, '\0', realCpyFrameNum+3);

					memcpy(pExtContent, &buffer[curPos-purelyFramelen+encodingOffSet], purelyFramelen-encodingOffSet);

					if(realCpyFrameNum > 0)
					{
						if(strncmp((char *)CompTmp, "TT2", 3) == 0 && pInfo->tagV2Info.bTitleMarked == false)
						{
							pInfo->pTitle = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->titleLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pTitle returned = (%s), pInfo->titleLen(%d)\n", pInfo->pTitle, pInfo->titleLen);
							#endif
							pInfo->tagV2Info.bTitleMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TP1", 3) == 0 && pInfo->tagV2Info.bArtistMarked == false)
						{
							pInfo->pArtist = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->artistLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pArtist returned = (%s), pInfo->artistLen(%d)\n", pInfo->pArtist, pInfo->artistLen);
							#endif
							pInfo->tagV2Info.bArtistMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TP2", 3) == 0 && pInfo->tagV2Info.bAlbum_ArtistMarked == false)
						{
							pInfo->pAlbum_Artist = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->album_artistLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pAlbum_Artist returned = (%s), pInfo->album_artistLen(%d)\n", pInfo->pAlbum_Artist, pInfo->album_artistLen);
							#endif
							pInfo->tagV2Info.bAlbum_ArtistMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TP3", 3) == 0 && pInfo->tagV2Info.bConductorMarked == false)
						{
							pInfo->pConductor = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->conductorLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pConductor returned = (%s), pInfo->conductorLen(%d)\n", pInfo->pConductor, pInfo->conductorLen);
							#endif
							pInfo->tagV2Info.bConductorMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TAL", 3) == 0 && pInfo->tagV2Info.bAlbumMarked == false)
						{
							pInfo->pAlbum = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->albumLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pAlbum returned = (%s), pInfo->albumLen(%d)\n", pInfo->pAlbum, pInfo->albumLen);
							#endif
							pInfo->tagV2Info.bAlbumMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TYE", 3) == 0 && pInfo->tagV2Info.bYearMarked == false)
						{
							pInfo->pYear = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->yearLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pYear returned = (%s), pInfo->yearLen(%d)\n", pInfo->pYear, pInfo->yearLen);
							#endif
							pInfo->tagV2Info.bYearMarked = true;
						}
						else if(strncmp((char *)CompTmp, "COM", 3) == 0 && pInfo->tagV2Info.bDescriptionMarked == false)
						{
							//skip language data!
							if(realCpyFrameNum > 4)
							{
								realCpyFrameNum -= 4;
								tmp = 4;

								//pExtContent[tmp+1] value should't have encoding value
								if(pExtContent[tmp] > 0x20 && (pExtContent[tmp - 1] == 0x00 || pExtContent[tmp - 1] == 0x01))
								{
									if(pExtContent[tmp - 1] == 0x00)
										textEncodingType = AV_ID3V2_ISO_8859;
									else
										textEncodingType = AV_ID3V2_UTF16;

									pInfo->pComment = mmfile_string_convert ((char*)&pExtContent[tmp], realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->commentLen);

									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "pInfo->pComment returned = (%s), pInfo->commentLen(%d)\n", pInfo->pComment, pInfo->commentLen);
									#endif
									pInfo->tagV2Info.bDescriptionMarked = true;
								}
								else
								{
									#ifdef __MMFILE_TEST_MODE__
									debug_msg (  "mmf_file_id3tag_parse_v222: failed to get Comment Info tmp(%d), purelyFramelen - encodingOffSet(%d)\n", tmp, purelyFramelen - encodingOffSet );
									#endif
								}
							}
							else
							{
								#ifdef __MMFILE_TEST_MODE__
								debug_msg (  "mmf_file_id3tag_parse_v222: Description info too small to parse realCpyFrameNum(%d)\n", realCpyFrameNum );
								#endif
							}
							tmp = 0;

						}
						else if(strncmp((char *)CompTmp, "TCO", 3) == 0 && pInfo->tagV2Info.bGenreMarked == false)
						{
							pInfo->pGenre = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->genreLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pGenre returned = (%s), pInfo->genreLen(%d)\n", pInfo->pGenre, pInfo->genreLen);
							#endif

							if((pInfo->pGenre != NULL) && (pInfo->genreLen > 0)) {
								bool ret = FALSE;
								int int_genre = -1;

								ret = is_numeric(pInfo->pGenre, pInfo->genreLen);

								if(ret == TRUE) {
									sscanf( pInfo->pGenre, "%d", &int_genre);
									#ifdef __MMFILE_TEST_MODE__
									debug_msg("genre information is inteager [%d]\n", int_genre);
									#endif

									//Change int to string
									if((0 <= int_genre) && (int_genre < GENRE_COUNT-1)) {
										/*save genreinfo like "(123)". mm_file_id3tag_restore_content_info convert it to string*/
										char tmp_genre[6] = {0,};	/*ex. "(123)+NULL"*/
										int tmp_genre_len = 0;

										memset (tmp_genre, 0, 6);
										snprintf(tmp_genre, sizeof(tmp_genre), "(%d)", int_genre);

										tmp_genre_len = strlen(tmp_genre);
										if(tmp_genre_len > 0) {
											if(pInfo->pGenre) _FREE_EX(pInfo->pGenre);
											pInfo->pGenre = mmfile_malloc (sizeof(char) * (tmp_genre_len + 1) );
											if (pInfo->pGenre)
											{
												strncpy(pInfo->pGenre, tmp_genre, tmp_genre_len);
												pInfo->pGenre[tmp_genre_len] = 0;
											}
										}
									}
								}
							}

							pInfo->tagV2Info.bGenreMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TRK", 3) == 0 && pInfo->tagV2Info.bTrackNumMarked == false)
						{
							pInfo->pTrackNum = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->tracknumLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pTrackNum returned = (%s), pInfo->tracknumLen(%d)\n", pInfo->pTrackNum, pInfo->tracknumLen);
							#endif
							pInfo->tagV2Info.bTrackNumMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TEN", 3) == 0 && pInfo->tagV2Info.bEncByMarked == false)
						{
							pInfo->pEncBy = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->encbyLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pEncBy returned = (%s), pInfo->encbyLen(%d)\n", pInfo->pEncBy, pInfo->encbyLen);
							#endif
							pInfo->tagV2Info.bEncByMarked = true;
						}
						else if(strncmp((char *)CompTmp, "WXX", 3) == 0 && pInfo->tagV2Info.bURLMarked == false)
						{
							if(realCpyFrameNum > 4)
							{
								//skip language data!
								realCpyFrameNum -= 4;
								tmp = 4;

								//pExtContent[tmp+1] value should't have null value
								if(pExtContent[tmp] > 0x20 && (pExtContent[tmp - 1] == 0x00 || pExtContent[tmp - 1] == 0x01))
								{
									if(pExtContent[tmp - 1] == 0x00)
										textEncodingType = AV_ID3V2_ISO_8859;
									else
										textEncodingType = AV_ID3V2_UTF16;

									pInfo->pURL = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->urlLen);

									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "pInfo->pURL returned = (%s), pInfo->urlLen(%d)\n", pInfo->pURL, pInfo->urlLen);
									#endif
									pInfo->tagV2Info.bURLMarked = true;
								}
								else
								{
									#ifdef __MMFILE_TEST_MODE__
									debug_msg (  "mmf_file_id3tag_parse_v222: failed to get URL Info tmp(%d), purelyFramelen - encodingOffSet(%d)\n", tmp, purelyFramelen - encodingOffSet );
									#endif
								}
							}
							else
							{
								#ifdef __MMFILE_TEST_MODE__
								debug_msg (  "mmf_file_id3tag_parse_v222: URL info too small to parse realCpyFrameNum(%d)\n", realCpyFrameNum );
								#endif
							}
							tmp = 0;
						}
						else if(strncmp((char *)CompTmp, "TCR", 3) == 0 && pInfo->tagV2Info.bCopyRightMarked == false)
						{
							pInfo->pCopyright = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->copyrightLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pCopyright returned = (%s), pInfo->copyrightLen(%d)\n", pInfo->pCopyright, pInfo->copyrightLen);
							#endif
							pInfo->tagV2Info.bCopyRightMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TOA", 3) == 0 && pInfo->tagV2Info.bOriginArtistMarked == false)
						{
							pInfo->pOriginArtist = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->originartistLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pOriginArtist returned = (%s), pInfo->originartistLen(%d)\n", pInfo->pOriginArtist, pInfo->originartistLen);
							#endif
							pInfo->tagV2Info.bOriginArtistMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TCM", 3) == 0 && pInfo->tagV2Info.bComposerMarked == false)
						{
							pInfo->pComposer = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->composerLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pComposer returned = (%s), pInfo->originartistLen(%d)\n", pInfo->pComposer, pInfo->composerLen);
							#endif
							pInfo->tagV2Info.bComposerMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TRD", 3) == 0 && pInfo->tagV2Info.bRecDateMarked== false)
						{
							pInfo->pRecDate = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->recdateLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pRecDate returned = (%s), pInfo->recdateLen(%d)\n", pInfo->pRecDate, pInfo->recdateLen);
							#endif
							pInfo->tagV2Info.bRecDateMarked = true;
						}
						else if(strncmp((char *)CompTmp, "PIC", 3) == 0 && pInfo->tagV2Info.bImageMarked == false && realCpyFrameNum <= 2000000)
						{
							if(pExtContent[0] != 0)
							{
								for(inx = 0; inx < MP3_ID3_IMAGE_EXT_MAX_LENGTH; inx++)
									pInfo->imageInfo.imageExt[inx] = '\0';//ini mimetype variable

								while((checkImgExtMax < MP3_ID3_IMAGE_EXT_MAX_LENGTH-1) && pExtContent[checkImgExtMax] != '\0')
								{
									pInfo->imageInfo.imageExt[checkImgExtMax] = pExtContent[checkImgExtMax];
									checkImgExtMax++;
								}
							}
							else
							{
								#ifdef __MMFILE_TEST_MODE__
								 debug_msg ( "mmf_file_id3tag_parse_v222: PIC image's not included to image Extention\n");
								#endif
							}

							imgstartOffset += checkImgExtMax;

							if(pExtContent[imgstartOffset] < AV_ID3V2_PICTURE_TYPE_MAX)
							{
								pInfo->imageInfo.pictureType = pExtContent[imgstartOffset];
							}
							imgstartOffset++;//PictureType(1byte)

							if(pExtContent[imgstartOffset] != 0x0)
							{
								int cur_pos = 0;
								int dis_len = 0;
								int new_dis_len = 0;
								char jpg_sign[3] = {0xff, 0xd8, 0xff};
								char png_sign[8] = {0x80, 0x50, 0x4e, 0x47,0x0d, 0x0a, 0x1a, 0x0a};
								char *tmp_desc = NULL;

								while (1) {
									if (pExtContent[imgstartOffset + cur_pos] == '\0') {
										if (realCpyFrameNum < imgstartOffset + cur_pos) {
												debug_error("End of APIC Tag %d %d %d\n", realCpyFrameNum, imgstartOffset, cur_pos);
												break;
										}
										/*check end of image description*/
										if ((pExtContent[imgstartOffset + cur_pos + 1] == jpg_sign[0]) ||
											(pExtContent[imgstartOffset + cur_pos + 1] == png_sign[0])) {
											#ifdef __MMFILE_TEST_MODE__
											debug_msg ( "length of description (%d)", cur_pos);
											#endif

											break;
										}
									}
									cur_pos ++;
								}

								dis_len = cur_pos + 1;

								tmp_desc = mmfile_malloc(sizeof(char) * dis_len);
								memcpy(tmp_desc, pExtContent + imgstartOffset, dis_len);

								/*convert description*/
								pInfo->imageInfo.imageDescription = mmfile_string_convert (tmp_desc, dis_len, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&new_dis_len);
								mmfile_free(tmp_desc);

								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "new_desc %s(%d)\n", pInfo->imageInfo.imageDescription, new_dis_len);
								#endif
								pInfo->imageInfo.imgDesLen = new_dis_len; /**/
								imgstartOffset += cur_pos;
							}
							else
							{
								pInfo->imageInfo.imgDesLen= 0;
							}

							if((pExtContent[imgstartOffset] == '\0') && (realCpyFrameNum - imgstartOffset > 0))
							{
								imgstartOffset ++; // endofDesceriptionType(1byte)

								while(pExtContent[imgstartOffset] == '\0')	//some content has useless '\0' in front of picture data
								{
									imgstartOffset ++;
								}

								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "after scaning imgDescription imgstartOffset(%d) value!\n", imgstartOffset);
								#endif

								if(realCpyFrameNum - imgstartOffset > 0)
								{
									pInfo->imageInfo.imageLen = realCpyFrameNum - imgstartOffset;
									pInfo->imageInfo.pImageBuf= mmfile_malloc (pInfo->imageInfo.imageLen + 1);
									memcpy(pInfo->imageInfo.pImageBuf, pExtContent+ imgstartOffset, pInfo->imageInfo.imageLen);
									pInfo->imageInfo.pImageBuf[pInfo->imageInfo.imageLen] = 0;

									if(IS_INCLUDE_URL(pInfo->imageInfo.imageMIMEType))
										pInfo->imageInfo.bURLInfo = true; //if mimetype is "-->", image date has an URL
								}
								else
								{
									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "No APIC image!! realCpyFrameNum(%d) - imgstartOffset(%d)\n", realCpyFrameNum, imgstartOffset);
									#endif
								}
							}

							checkImgMimeTypeMax = 0;
							checkImgExtMax = 0;
							inx = 0;
							imgstartOffset = 0;
							pInfo->tagV2Info.bImageMarked= true;

						}
					}

				}
			}
			else
			{
				curPos += purelyFramelen;
				if(purelyFramelen != 0)
					needToloopv2taglen = MP3_TAGv2_22_TXT_HEADER_LEN;
			}

			if(pExtContent)	_FREE_EX(pExtContent);
			memset(CompTmp, 0, 4);
			if(curPos < taglen)
			{
				needToloopv2taglen -= oneFrameLen;
				v2numOfFrames++;
			}
			else
				needToloopv2taglen = MP3_TAGv2_22_TXT_HEADER_LEN;
			oneFrameLen = 0;
			encodingOffSet = 0;
			realCpyFrameNum = 0;
			textEncodingType = 0;
			purelyFramelen = 0;

		}
	}

	release_characterset_array(charset_array);

	if(taglen) {
		return true;
	} else {
		return false;
	}
}

EXPORT_API
bool mm_file_id3tag_parse_v223(AvFileContentInfo* pInfo, unsigned char *buffer)
{
	unsigned long taglen = 0;
	unsigned long needToloopv2taglen;
	unsigned long oneFrameLen = 0;
	unsigned long v2numOfFrames = 0;
	unsigned long curPos = 0;
	char CompTmp[5];
	unsigned char *pExtContent = NULL;
	unsigned long purelyFramelen = 0;
	int inx=0, encodingOffSet=0, realCpyFrameNum=0, checkImgMimeTypeMax=0, imgstartOffset=0,  tmp = 0;
	int textEncodingType = 0;
	char **charset_array = NULL;

	make_characterset_array(&charset_array);

	init_content_info(pInfo);

	taglen = pInfo->tagV2Info.tagLen;
	needToloopv2taglen = taglen - MP3_TAGv2_HEADER_LEN;
	curPos = MP3_TAGv2_HEADER_LEN;

#ifdef __MMFILE_TEST_MODE__
	debug_msg ("ID3tag v223--------------------------------------------------------------\n");
#endif

	if(needToloopv2taglen -MP3_TAGv2_23_TXT_HEADER_LEN > MP3_TAGv2_23_TXT_HEADER_LEN)
	{
		v2numOfFrames = 1;
		while(needToloopv2taglen > MP3_TAGv2_23_TXT_HEADER_LEN)
		{
			if((buffer[curPos] < '0' || buffer[curPos] > 'Z') || (buffer[curPos+1] < '0' || buffer[curPos+1] > 'Z')
				|| (buffer[curPos+2] < '0' || buffer[curPos+2] > 'Z')|| (buffer[curPos+3] < '0' || buffer[curPos+3] > 'Z'))
				break;

			memcpy(CompTmp, &buffer[curPos], 4);

			CompTmp[4] = 0;
			oneFrameLen = MP3_TAGv2_23_TXT_HEADER_LEN;
			oneFrameLen += (unsigned long)buffer[4+curPos] << 24 | (unsigned long)buffer[5+curPos] << 16
				| (unsigned long)buffer[6+curPos] << 8 | (unsigned long)buffer[7+curPos];
			
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ("----------------------------------------------------------------------------------------------------\n");
			#endif

			if(oneFrameLen > taglen-curPos)
				break;

			purelyFramelen = oneFrameLen - MP3_TAGv2_23_TXT_HEADER_LEN;
			curPos +=MP3_TAGv2_23_TXT_HEADER_LEN;

			if(oneFrameLen > MP3_TAGv2_23_TXT_HEADER_LEN && purelyFramelen <= taglen-curPos)
			{
				curPos += purelyFramelen;

				if( IS_ENCODEDBY_UTF16(buffer+(curPos-purelyFramelen)) )
				{
					encodingOffSet = 2;
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "this text string(%s) encoded by UTF16 encodingOffSet(%d)\n", CompTmp, encodingOffSet);
					#endif
					textEncodingType = AV_ID3V2_UTF16;
				}
				else if ( IS_ENCODEDBY_UTF16_R(buffer+(curPos-purelyFramelen)) )
				{
					encodingOffSet = 2;
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "this text string(%s) encoded by UTF16 encodingOffSet(%d)\n", CompTmp, encodingOffSet);
					#endif
					textEncodingType = AV_ID3V2_UTF16_BE;
				}
				else if( IS_ENCODEDBY_UTF16(buffer+(curPos-purelyFramelen+1)) )
				{
					encodingOffSet = 3;
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "this text string(%s) encoded by UTF16 encodingOffSet(%d)\n", CompTmp, encodingOffSet);
					#endif
					textEncodingType = AV_ID3V2_UTF16;
				}
				else if ( IS_ENCODEDBY_UTF16_R(buffer+(curPos-purelyFramelen+1)) )
				{
					encodingOffSet = 3;
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "this text string(%s) encoded by UTF16 encodingOffSet(%d)\n", CompTmp, encodingOffSet);
					#endif
					textEncodingType = AV_ID3V2_UTF16_BE;
				}
				else
				{
					if (buffer[curPos-purelyFramelen+encodingOffSet] == 0x00)
					{
						#ifdef __MMFILE_TEST_MODE__
						debug_msg ("encodingOffset will be set to 1\n");
						#endif

						encodingOffSet=1;
					}
					else
					{
						#ifdef __MMFILE_TEST_MODE__
						debug_msg ("Finding encodingOffset\n");
						#endif

						while((buffer[curPos-purelyFramelen+encodingOffSet] < 0x20) && (encodingOffSet < purelyFramelen)) // text string encoded by ISO-8859-1
							encodingOffSet++;
					}
					textEncodingType = AV_ID3V2_ISO_8859;
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "this text string(%s) encoded by ISO-8859-1 encodingOffSet(%d)\n", CompTmp, encodingOffSet);
					#endif
				}

				if(encodingOffSet < purelyFramelen)
				{
					realCpyFrameNum = purelyFramelen - encodingOffSet;
					pExtContent = mmfile_malloc (realCpyFrameNum+3);
					memset(pExtContent, '\0', realCpyFrameNum+3);

					if (textEncodingType != AV_ID3V2_UTF16 && textEncodingType != AV_ID3V2_UTF16_BE) {
						if (CompTmp[0] == 'T' ||(strcmp(CompTmp, "APIC")==0)) {
							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "get the new text ecoding type\n");
							#endif
							textEncodingType = buffer[curPos-purelyFramelen+encodingOffSet -1];
						}
					}

					memcpy(pExtContent, &buffer[curPos-purelyFramelen+encodingOffSet], purelyFramelen-encodingOffSet);
					if(realCpyFrameNum > 0)
					{
						if(strncmp((char *)CompTmp, "TIT2", 4) == 0 && pInfo->tagV2Info.bTitleMarked == false)
						{
							pInfo->pTitle = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->titleLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pTitle returned = (%s), pInfo->titleLen(%d)\n", pInfo->pTitle, pInfo->titleLen);
							#endif
							pInfo->tagV2Info.bTitleMarked = true;

						}
						else if(strncmp((char *)CompTmp, "TPE1", 4) == 0 && pInfo->tagV2Info.bArtistMarked == false)
						{
							pInfo->pArtist = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->artistLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pArtist returned = (%s), pInfo->artistLen(%d)\n", pInfo->pArtist, pInfo->artistLen);
							#endif
							pInfo->tagV2Info.bArtistMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TPE2", 4) == 0 && pInfo->tagV2Info.bAlbum_ArtistMarked == false)
						{
							pInfo->pAlbum_Artist = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->album_artistLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pAlbum_Artist returned = (%s), pInfo->album_artistLen(%d)\n", pInfo->pAlbum_Artist, pInfo->album_artistLen);
							#endif
							pInfo->tagV2Info.bAlbum_ArtistMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TPE3", 4) == 0 && pInfo->tagV2Info.bConductorMarked == false)
						{
							pInfo->pConductor = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->conductorLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pConductor returned = (%s), pInfo->conductorLen(%d)\n", pInfo->pConductor, pInfo->conductorLen);
							#endif
							pInfo->tagV2Info.bConductorMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TALB", 4) == 0 && pInfo->tagV2Info.bAlbumMarked == false)
						{
							pInfo->pAlbum = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->albumLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pAlbum returned = (%s), pInfo->albumLen(%d)\n", pInfo->pAlbum, pInfo->albumLen);
							#endif
							pInfo->tagV2Info.bAlbumMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TYER", 4) == 0 && pInfo->tagV2Info.bYearMarked == false)
						{
							pInfo->pYear = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->yearLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pYear returned = (%s), pInfo->yearLen(%d)\n", pInfo->pYear, pInfo->yearLen);
							#endif
							pInfo->tagV2Info.bYearMarked = true;
						}
						else if(strncmp((char *)CompTmp, "COMM", 4) == 0 && pInfo->tagV2Info.bDescriptionMarked == false)
						{
							if(realCpyFrameNum > 3)
							{
								realCpyFrameNum -= 3;
								tmp = 3;

								//pExtContent[tmp+1] value should't have encoding value
								if(pExtContent[tmp] == 0x00 || pExtContent[tmp] == 0xFF|| pExtContent[tmp] == 0xFE)
								{
									if((IS_ENCODEDBY_UTF16(pExtContent+tmp) || IS_ENCODEDBY_UTF16_R(pExtContent+tmp)) && realCpyFrameNum > 2)
									{
										while((NEWLINE_OF_UTF16(pExtContent + tmp) || NEWLINE_OF_UTF16_R(pExtContent + tmp))&& realCpyFrameNum > 4)
										{
											realCpyFrameNum -= 4;
											tmp += 4;
										}

										if(IS_ENCODEDBY_UTF16(pExtContent+tmp) && (realCpyFrameNum > 2))
										{
											realCpyFrameNum -= 2;
											tmp += 2;
											textEncodingType = AV_ID3V2_UTF16;
										}
										else if (IS_ENCODEDBY_UTF16_R(pExtContent+tmp) && (realCpyFrameNum > 2))
										{
											realCpyFrameNum -= 2;
											tmp += 2;
											textEncodingType = AV_ID3V2_UTF16_BE;
										}
										else if(IS_ENCODEDBY_UTF16(pExtContent+tmp+1) && (realCpyFrameNum > 3))
										{
											realCpyFrameNum -= 3;
											tmp += 3;
											textEncodingType = AV_ID3V2_UTF16;
										}
										else if (IS_ENCODEDBY_UTF16_R(pExtContent+tmp+1)  && (realCpyFrameNum > 3))
										{
											realCpyFrameNum -= 3;
											tmp += 3;
											textEncodingType = AV_ID3V2_UTF16_BE;
										}
										else
										{
											#ifdef __MMFILE_TEST_MODE__
											debug_msg ( "pInfo->pComment Never Get Here!!\n");
											#endif
										}
									}
									else
									{
										while((pExtContent[tmp] < 0x20) && (tmp < realCpyFrameNum)) // text string encoded by ISO-8859-1
										{
											realCpyFrameNum --;
											tmp++;
										}
										textEncodingType = AV_ID3V2_ISO_8859;
									}

									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "tmp(%d) textEncodingType(%d), realCpyFrameNum(%d)\n", tmp, textEncodingType, realCpyFrameNum);
									#endif

									pInfo->pComment = mmfile_string_convert ((const char*)&pExtContent[tmp], realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->commentLen);
								}
								else
								{
									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "failed to get Comment Info tmp(%d), purelyFramelen - encodingOffSet(%d)\n", tmp, purelyFramelen - encodingOffSet);
									#endif
									pInfo->commentLen = 0;
								}
							}
							else
							{
								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "Description info too small to parse realCpyFrameNum(%d)\n", realCpyFrameNum);
								#endif
								pInfo->commentLen = 0;
							}
							tmp = 0;

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pComment returned = (%s), pInfo->commentLen(%d)\n", pInfo->pComment, pInfo->commentLen);
							#endif
							pInfo->tagV2Info.bDescriptionMarked = true;
						}
						else if(strncmp((char *)CompTmp, "SYLT", 4) == 0 && pInfo->tagV2Info.bSyncLyricsMarked == false)
						{
							int idx = 0;
							int copy_len = 0;
							int copy_start_pos = tmp;
							AvSynclyricsInfo *synclyrics_info = NULL;
							GList *synclyrics_info_list = NULL;
									
							if(realCpyFrameNum > 5)
							{
								realCpyFrameNum -= 5;
								tmp = 5;

								//pExtContent[tmp+1] value should't have encoding value
								if(pExtContent[tmp] == 0x00 || pExtContent[tmp] == 0xFF|| pExtContent[tmp] == 0xFE)
								{
									if((IS_ENCODEDBY_UTF16(pExtContent+tmp) || IS_ENCODEDBY_UTF16_R(pExtContent+tmp)) && realCpyFrameNum > 2)
									{
										while((NEWLINE_OF_UTF16(pExtContent + tmp) || NEWLINE_OF_UTF16_R(pExtContent + tmp))&& realCpyFrameNum > 4)
										{
											realCpyFrameNum -= 4;
											tmp += 4;
										}

										if(IS_ENCODEDBY_UTF16(pExtContent+tmp) && (realCpyFrameNum > 2))
										{
											realCpyFrameNum -= 2;
											tmp += 2;
											textEncodingType = AV_ID3V2_UTF16;
										}
										else if (IS_ENCODEDBY_UTF16_R(pExtContent+tmp) && (realCpyFrameNum > 2))
										{
											realCpyFrameNum -= 2;
											tmp += 2;
											textEncodingType = AV_ID3V2_UTF16_BE;
										}
										else if(IS_ENCODEDBY_UTF16(pExtContent+tmp+1) && (realCpyFrameNum > 3))
										{
											realCpyFrameNum -= 3;
											tmp += 3;
											textEncodingType = AV_ID3V2_UTF16;
										}
										else if (IS_ENCODEDBY_UTF16_R(pExtContent+tmp+1)  && (realCpyFrameNum > 3))
										{
											realCpyFrameNum -= 3;
											tmp += 3;
											textEncodingType = AV_ID3V2_UTF16_BE;
										}
										else
										{
											#ifdef __MMFILE_TEST_MODE__
											debug_msg ( "pInfo->pSyncLyrics Never Get Here!!\n");
											#endif
										}
									}
									else
									{
										while((pExtContent[tmp] < 0x20) && (tmp < realCpyFrameNum)) // text string encoded by ISO-8859-1
										{
											realCpyFrameNum --;
											tmp++;
										}
										textEncodingType = AV_ID3V2_ISO_8859;
									}

									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "tmp(%d) textEncodingType(%d), realCpyFrameNum(%d)\n", tmp, textEncodingType, realCpyFrameNum);
									#endif

									if(realCpyFrameNum < MMFILE_SYNC_LYRIC_INFO_MIN_LEN)
									{
										#ifdef __MMFILE_TEST_MODE__
										debug_msg ( "failed to get Synchronised lyrics Info realCpyFramNum(%d)\n", realCpyFrameNum);
										#endif
										pInfo->syncLyricsNum = 0;
									}
									else
									{
										if(textEncodingType == AV_ID3V2_UTF16)
										{
											debug_warning ("[AV_ID3V2_UTF16] not implemented\n");
										}
										else if (textEncodingType == AV_ID3V2_UTF16_BE)
										{
											debug_warning ("[AV_ID3V2_UTF16_BE] not implemented\n");
										}
										else
										{
											for(idx = 0; idx < realCpyFrameNum; idx++)
											{
												if(pExtContent[tmp+idx] == 0x00)
												{
													synclyrics_info = (AvSynclyricsInfo *)malloc(sizeof(AvSynclyricsInfo));

													if(textEncodingType == AV_ID3V2_UTF8) {
														synclyrics_info->lyric_info= mmfile_malloc(copy_len+1);
														memset(synclyrics_info->lyric_info, 0, copy_len+1);
														memcpy(synclyrics_info->lyric_info, pExtContent+copy_start_pos, copy_len);
														synclyrics_info->lyric_info[copy_len+1] = '\0';
													}
													else {
														synclyrics_info->lyric_info = mmfile_string_convert ((const char*)&pExtContent[copy_start_pos], copy_len, "UTF-8", charset_array[AV_ID3V2_ISO_8859], NULL, NULL);
													}

													synclyrics_info->time_info= (unsigned long)pExtContent[tmp+idx+1] << 24 | (unsigned long)pExtContent[tmp+idx+2] << 16 | (unsigned long)pExtContent[tmp+idx+3] << 8  | (unsigned long)pExtContent[tmp+idx+4];
													idx += 4;
													copy_start_pos = tmp + idx + 1;
													#ifdef __MMFILE_TEST_MODE__
													debug_msg("[%d][%s] idx[%d], copy_len[%d] copy_start_pos[%d]", synclyrics_info->time_info, synclyrics_info->lyric_info, idx, copy_len, copy_start_pos);
													#endif
													copy_len = 0;
													synclyrics_info_list = g_list_append(synclyrics_info_list, synclyrics_info);
												}
												copy_len ++;
											}
											pInfo->pSyncLyrics = synclyrics_info_list;
											pInfo->syncLyricsNum = g_list_length(pInfo->pSyncLyrics);
										}
									}
								}
								else
								{
									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "failed to get Synchronised lyrics Info tmp(%d), purelyFramelen - encodingOffSet(%d)\n", tmp, purelyFramelen - encodingOffSet);
									#endif
									pInfo->syncLyricsNum = 0;
								}
							}
							else
							{
								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "Synchronised lyrics too small to parse realCpyFrameNum(%d)\n", realCpyFrameNum);
								#endif
								pInfo->syncLyricsNum = 0;
							}
							tmp = 0;

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pSyncLyrics returned = (%s), pInfo->syncLyricsNum(%d)\n", pInfo->pSyncLyrics, pInfo->syncLyricsNum);
							#endif
							pInfo->tagV2Info.bSyncLyricsMarked = true;
						}
						else if(strncmp((char *)CompTmp, "USLT", 4) == 0 && pInfo->tagV2Info.bUnsyncLyricsMarked == false)
						{
							char *lang_info = strndup((char*)pExtContent, 3);

							if(realCpyFrameNum > 3)
							{
								realCpyFrameNum -= 3;
								tmp = 3;

								/*find start of lyrics */
								while(1) {
									if (pExtContent[tmp] == 0x00) {
										if (pExtContent[tmp+1] == 0x00) {
											realCpyFrameNum -=2;
											tmp +=2;
										}
										break;
									} else {
										realCpyFrameNum--;
										tmp++;
									}
								}

								//pExtContent[tmp+1] value should't have encoding value
								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "tpExtContent[%d] %x\n", tmp, pExtContent[tmp]);
								#endif
								if(pExtContent[tmp] == 0x00 || pExtContent[tmp] == 0xFF|| pExtContent[tmp] == 0xFE)
								{
									if((IS_ENCODEDBY_UTF16(pExtContent+tmp) || IS_ENCODEDBY_UTF16_R(pExtContent+tmp)) && realCpyFrameNum > 2)
									{
										while((NEWLINE_OF_UTF16(pExtContent + tmp) || NEWLINE_OF_UTF16_R(pExtContent + tmp))&& realCpyFrameNum > 4)
										{
											realCpyFrameNum -= 4;
											tmp += 4;
										}

										if(IS_ENCODEDBY_UTF16(pExtContent+tmp) && (realCpyFrameNum > 2))
										{
											realCpyFrameNum -= 2;
											tmp += 2;
											textEncodingType = AV_ID3V2_UTF16;
										}
										else if (IS_ENCODEDBY_UTF16_R(pExtContent+tmp) && (realCpyFrameNum > 2))
										{
											realCpyFrameNum -= 2;
											tmp += 2;
											textEncodingType = AV_ID3V2_UTF16_BE;
										}
										else if(IS_ENCODEDBY_UTF16(pExtContent+tmp+1) && (realCpyFrameNum > 3))
										{
											realCpyFrameNum -= 3;
											tmp += 3;
											textEncodingType = AV_ID3V2_UTF16;
										}
										else if (IS_ENCODEDBY_UTF16_R(pExtContent+tmp+1)  && (realCpyFrameNum > 3))
										{
											realCpyFrameNum -= 3;
											tmp += 3;
											textEncodingType = AV_ID3V2_UTF16_BE;
										}
										else
										{
											#ifdef __MMFILE_TEST_MODE__
											debug_msg ( "pInfo->pUnsyncLyrics Never Get Here!!\n");
											#endif
										}
									}
									else
									{
										while((pExtContent[tmp] < 0x20) && (tmp < realCpyFrameNum)) // text string encoded by ISO-8859-1
										{
											realCpyFrameNum --;
											tmp++;
										}
										textEncodingType = AV_ID3V2_ISO_8859;
									}

									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "tmp(%d) textEncodingType(%d), realCpyFrameNum(%d)\n", tmp, textEncodingType, realCpyFrameNum);
									#endif

									char *char_set = NULL;
									if (textEncodingType == AV_ID3V2_ISO_8859) {
										if (!strcasecmp(lang_info, "KOR")) {
											char_set = strdup("EUC-KR");
										} else {
											char_set = mmfile_get_charset((const char*)&pExtContent[tmp]);
										}
									}

									if (char_set == NULL) {
										pInfo->pUnsyncLyrics = mmfile_string_convert ((const char*)&pExtContent[tmp], realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->unsynclyricsLen);
									} else {
										pInfo->pUnsyncLyrics = mmfile_string_convert ((const char*)&pExtContent[tmp], realCpyFrameNum, "UTF-8", char_set, NULL, (unsigned int*)&pInfo->unsynclyricsLen);
										_FREE_EX(char_set);
									}
								}
								else
								{
									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "failed to get Unsynchronised lyrics Info tmp(%d), purelyFramelen - encodingOffSet(%d)\n", tmp, purelyFramelen - encodingOffSet);
									#endif
									pInfo->unsynclyricsLen = 0;
								}
							}
							else
							{
								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "Unsynchronised lyrics too small to parse realCpyFrameNum(%d)\n", realCpyFrameNum);
								#endif
								pInfo->unsynclyricsLen = 0;
							}
							tmp = 0;

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pUnsyncLyrics returned = (%s), pInfo->unsynclyricsLen(%d)\n", pInfo->pUnsyncLyrics, pInfo->unsynclyricsLen);
							#endif
							pInfo->tagV2Info.bUnsyncLyricsMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TCON", 4) == 0 && pInfo->tagV2Info.bGenreMarked == false)
						{
							pInfo->pGenre = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->genreLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pGenre returned = (%s), pInfo->genreLen(%d)\n", pInfo->pGenre, pInfo->genreLen);
							#endif

							if((pInfo->pGenre != NULL) && (pInfo->genreLen > 0)) {
								bool ret = FALSE;
								int int_genre = -1;

								ret = is_numeric(pInfo->pGenre, pInfo->genreLen);

								if(ret == TRUE) {
									sscanf( pInfo->pGenre, "%d", &int_genre);
									#ifdef __MMFILE_TEST_MODE__
									debug_msg("genre information is inteager [%d]\n", int_genre);
									#endif

									//Change int to string
									if((0 <= int_genre) && (int_genre < GENRE_COUNT-1)) {
										/*save genreinfo like "(123)". mm_file_id3tag_restore_content_info convert it to string*/
										char tmp_genre[6] = {0,};	/*ex. "(123)+NULL"*/
										int tmp_genre_len = 0;

										memset (tmp_genre, 0, 6);
										snprintf(tmp_genre, sizeof(tmp_genre), "(%d)", int_genre);

										tmp_genre_len = strlen(tmp_genre);
										if(tmp_genre_len > 0) {
											if(pInfo->pGenre) _FREE_EX(pInfo->pGenre);
											pInfo->pGenre = mmfile_malloc (sizeof(char) * (tmp_genre_len + 1) );
											if (pInfo->pGenre)
											{
												strncpy(pInfo->pGenre, tmp_genre, tmp_genre_len);
												pInfo->pGenre[tmp_genre_len] = 0;
											}
										}
									}
								}
							}

							pInfo->tagV2Info.bGenreMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TRCK", 4) == 0 && pInfo->tagV2Info.bTrackNumMarked == false)
						{
							pInfo->pTrackNum = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->tracknumLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pTrackNum returned = (%s), pInfo->tracknumLen(%d)\n", pInfo->pTrackNum, pInfo->tracknumLen);
							#endif
							pInfo->tagV2Info.bTrackNumMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TENC", 4) == 0 && pInfo->tagV2Info.bEncByMarked == false)
						{
							pInfo->pEncBy = mmfile_string_convert ((char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->encbyLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pEncBy returned = (%s), pInfo->encbyLen(%d)\n", pInfo->pEncBy, pInfo->encbyLen);
							#endif
							pInfo->tagV2Info.bEncByMarked = true;
						}
						else if(strncmp((char *)CompTmp, "WXXX", 4) == 0 && pInfo->tagV2Info.bURLMarked == false)
						{
							pInfo->pURL = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->urlLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pURL returned = (%s), pInfo->urlLen(%d)\n", pInfo->pURL, pInfo->urlLen);
							#endif
							pInfo->tagV2Info.bURLMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TCOP", 4) == 0 && pInfo->tagV2Info.bCopyRightMarked == false)
						{
							pInfo->pCopyright = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->copyrightLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pCopyright returned = (%s), pInfo->copyrightLen(%d)\n", pInfo->pCopyright, pInfo->copyrightLen);
							#endif
							pInfo->tagV2Info.bCopyRightMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TOPE", 4) == 0 && pInfo->tagV2Info.bOriginArtistMarked == false)
						{
							pInfo->pOriginArtist = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->originartistLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pOriginArtist returned = (%s), pInfo->originartistLen(%d)\n", pInfo->pOriginArtist, pInfo->originartistLen);
							#endif
							pInfo->tagV2Info.bOriginArtistMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TCOM", 4) == 0 && pInfo->tagV2Info.bComposerMarked == false)
						{
							pInfo->pComposer = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->composerLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pComposer returned = (%s), pInfo->composerLen(%d)\n", pInfo->pComposer, pInfo->composerLen);
							#endif
							pInfo->tagV2Info.bComposerMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TRDA", 4) == 0 && pInfo->tagV2Info.bRecDateMarked== false)
						{
							pInfo->pRecDate = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->recdateLen);

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pRecDate returned = (%s), pInfo->recdateLen(%d)\n", pInfo->pRecDate, pInfo->recdateLen);
							#endif
							pInfo->tagV2Info.bRecDateMarked = true;
						}
						else if(strncmp((char *)CompTmp, "APIC", 4) == 0 && pInfo->tagV2Info.bImageMarked == false && realCpyFrameNum <= 2000000)
						{
							debug_msg ( "text encoding %d \n", textEncodingType);

							if(pExtContent[0] != '\0')
							{
								for(inx = 0; inx < MP3_ID3_IMAGE_MIME_TYPE_MAX_LENGTH-1; inx++)
									pInfo->imageInfo.imageMIMEType[inx] = '\0';//ini mimetype variable

								while((checkImgMimeTypeMax < MP3_ID3_IMAGE_MIME_TYPE_MAX_LENGTH-1) && pExtContent[checkImgMimeTypeMax] != '\0')
								{
									pInfo->imageInfo.imageMIMEType[checkImgMimeTypeMax] = pExtContent[checkImgMimeTypeMax];
									checkImgMimeTypeMax++;
								}
								pInfo->imageInfo.imgMimetypeLen = checkImgMimeTypeMax;
							}
							else
							{
								pInfo->imageInfo.imgMimetypeLen = 0;
								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "APIC image's not included to MIME type\n");
								#endif
							}

							imgstartOffset += checkImgMimeTypeMax;

							if((pExtContent[imgstartOffset] == '\0') && (realCpyFrameNum - imgstartOffset > 0))
							{
								imgstartOffset++;//endofMIME(1byte)
								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "after scaning Mime type imgstartOffset(%d) value!\n", imgstartOffset);
								#endif

								if(pExtContent[imgstartOffset] < AV_ID3V2_PICTURE_TYPE_MAX)
								{
									pInfo->imageInfo.pictureType = pExtContent[imgstartOffset];
								}
								else
								{
									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "APIC image has invalid picture type(0x%x)\n", pExtContent[imgstartOffset]);
									#endif
								}
								imgstartOffset++;//PictureType(1byte)
								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "after scaning PictureType imgstartOffset(%d) value!\n", imgstartOffset);
								#endif

								if(pExtContent[imgstartOffset] != 0x0)
								{
									int cur_pos = 0;
									int dis_len = 0;
									int new_dis_len = 0;
									char jpg_sign[3] = {0xff, 0xd8, 0xff};
									char png_sign[8] = {0x80, 0x50, 0x4e, 0x47,0x0d, 0x0a, 0x1a, 0x0a};
									char *tmp_desc = NULL;

									while (1) {
										if (pExtContent[imgstartOffset + cur_pos] == '\0') {
											if (realCpyFrameNum < imgstartOffset + cur_pos) {
												debug_error("End of APIC Tag %d %d %d\n", realCpyFrameNum, imgstartOffset, cur_pos);
												break;
											}
											/*check end of image description*/
											if ((pExtContent[imgstartOffset + cur_pos + 1] == jpg_sign[0]) ||
												(pExtContent[imgstartOffset + cur_pos + 1] == png_sign[0])) {
												#ifdef __MMFILE_TEST_MODE__
												debug_msg ( "length of description (%d)", cur_pos);
												#endif

												break;
											}
										}
										cur_pos ++;
									}

									dis_len = cur_pos + 1;

									tmp_desc = mmfile_malloc(sizeof(char) * dis_len);
									memcpy(tmp_desc, pExtContent + imgstartOffset, dis_len);

									/*convert description*/
									pInfo->imageInfo.imageDescription = mmfile_string_convert (tmp_desc, dis_len, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&new_dis_len);
									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "new_desc %s(%d)\n", pInfo->imageInfo.imageDescription, new_dis_len);
									#endif
									mmfile_free(tmp_desc);

									pInfo->imageInfo.imgDesLen = new_dis_len; /**/
									imgstartOffset += cur_pos;
								}
								else
								{
									pInfo->imageInfo.imgDesLen= 0;
									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "APIC image's not included to Description!!!\n");
									#endif
								}

								if((pExtContent[imgstartOffset] == '\0') && (realCpyFrameNum - imgstartOffset > 0))
								{
									imgstartOffset ++; // endofDesceriptionType(1byte)

									while(pExtContent[imgstartOffset] == '\0')	//some content has useless '\0' in front of picture data
									{
										imgstartOffset ++;
									}

									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "after scaning imgDescription imgstartOffset(%d) value!\n", imgstartOffset);
									#endif

									if(realCpyFrameNum - imgstartOffset > 0)
									{
										pInfo->imageInfo.imageLen = realCpyFrameNum - imgstartOffset;
										pInfo->imageInfo.pImageBuf = mmfile_malloc (pInfo->imageInfo.imageLen + 1);
										memcpy(pInfo->imageInfo.pImageBuf, pExtContent+ imgstartOffset, pInfo->imageInfo.imageLen);
										pInfo->imageInfo.pImageBuf[pInfo->imageInfo.imageLen] = 0;
										if(IS_INCLUDE_URL(pInfo->imageInfo.imageMIMEType))
											pInfo->imageInfo.bURLInfo = true; //if mimetype is "-->", image date has an URL
									}
									else
									{
										#ifdef __MMFILE_TEST_MODE__
										debug_msg ( "No APIC image!! realCpyFrameNum(%d) - imgstartOffset(%d)\n", realCpyFrameNum, imgstartOffset);
										#endif
									}
									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "pInfo->imageInfo.imageLen(%d), imgstartOffset(%d)!\n", pInfo->imageInfo.imageLen, imgstartOffset);
									#endif
								}
								else
								{
									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "pExtContent[imgstartOffset](%d) value should setted NULL value for end of description! realCpyFrameNum - imgstartOffset(%d)\n",
												pExtContent[imgstartOffset], realCpyFrameNum - imgstartOffset);
									#endif
								}
							}
							else
							{
								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "pExtContent[imgstartOffset](%d) value should setted NULL value for end of mimetype! realCpyFrameNum - imgstartOffset(%d)\n",
										pExtContent[imgstartOffset], realCpyFrameNum - imgstartOffset);
								#endif
							}

							checkImgMimeTypeMax = 0;
							inx = 0;
							imgstartOffset = 0;
							pInfo->tagV2Info.bImageMarked= true;

						}
						else
						{
							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "CompTmp(%s) This Frame ID currently not Supports!!\n", CompTmp);
							#endif
						}
					}

				}
				else
				{
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "All of the pExtContent Values are NULL\n");
					#endif
				}
			}
			else
			{
				curPos += purelyFramelen;
				if(purelyFramelen != 0)
					needToloopv2taglen = MP3_TAGv2_23_TXT_HEADER_LEN;
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ( "This Frame's size is Zero! purelyFramelen(%d)\n", purelyFramelen);
				#endif
			}

			if(pExtContent)	_FREE_EX(pExtContent);
			memset(CompTmp, 0, 4);

			if(curPos < taglen)
			{
				needToloopv2taglen -= oneFrameLen;
				v2numOfFrames++;
			}
			else
				needToloopv2taglen = MP3_TAGv2_23_TXT_HEADER_LEN;
			oneFrameLen = 0;
			encodingOffSet = 0;
			realCpyFrameNum = 0;
			textEncodingType = 0;
			purelyFramelen = 0;

		}
	}

	release_characterset_array(charset_array);

	if(taglen)
		return true;
	else
		return false;

}

EXPORT_API
bool mm_file_id3tag_parse_v224(AvFileContentInfo* pInfo, unsigned char *buffer)
{
	unsigned long taglen = 0;
	unsigned long needToloopv2taglen;
	unsigned long oneFrameLen = 0;
	unsigned long v2numOfFrames = 0;
	unsigned long curPos = 0;
	char CompTmp[5];
	unsigned char *pExtContent = NULL;
	unsigned long purelyFramelen = 0;
	int inx=0, encodingOffSet=0, realCpyFrameNum=0, checkImgMimeTypeMax=0, imgstartOffset=0,  tmp = 0;
	int textEncodingType = 0;
	char **charset_array = NULL;

	make_characterset_array(&charset_array);

	init_content_info(pInfo);

	taglen = pInfo->tagV2Info.tagLen;
	needToloopv2taglen = taglen - MP3_TAGv2_HEADER_LEN;
	curPos = MP3_TAGv2_HEADER_LEN;

#ifdef __MMFILE_TEST_MODE__
	debug_msg ("ID3tag v224--------------------------------------------------------------\n");
#endif

	/* check Extended Header */
	if (buffer[5] & 0x40)
	{
		/* if extended header exists, skip it*/
		int extendedHeaderLen = (unsigned long)buffer[10] << 21 | (unsigned long)buffer[11] << 14 | (unsigned long)buffer[12] << 7  | (unsigned long)buffer[13];

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("--------------- extendedHeaderLen = %d\n", extendedHeaderLen);
		#endif 

		curPos += extendedHeaderLen;
	}

	if(needToloopv2taglen -MP3_TAGv2_23_TXT_HEADER_LEN > MP3_TAGv2_23_TXT_HEADER_LEN)
	{
		v2numOfFrames = 1;
		while(needToloopv2taglen > MP3_TAGv2_23_TXT_HEADER_LEN)
		{
			if((buffer[curPos] < '0' || buffer[curPos] > 'Z') || (buffer[curPos+1] < '0' || buffer[curPos+1] > 'Z')
				|| (buffer[curPos+2] < '0' || buffer[curPos+2] > 'Z')|| (buffer[curPos+3] < '0' || buffer[curPos+3] > 'Z'))
				break;

			memcpy(CompTmp, &buffer[curPos], 4);

			CompTmp[4] = 0;
			oneFrameLen = MP3_TAGv2_23_TXT_HEADER_LEN;
			oneFrameLen += (unsigned long)buffer[4+curPos] << 21 | (unsigned long)buffer[5+curPos] << 14
				| (unsigned long)buffer[6+curPos] << 7 | (unsigned long)buffer[7+curPos];
			if(oneFrameLen > taglen-curPos)
				break;

			purelyFramelen = oneFrameLen - MP3_TAGv2_23_TXT_HEADER_LEN;
			curPos +=MP3_TAGv2_23_TXT_HEADER_LEN;

#ifdef __MMFILE_TEST_MODE__
			debug_msg ("-----------------------------------------------------------------------------------\n");
#endif

			if(oneFrameLen > MP3_TAGv2_23_TXT_HEADER_LEN && purelyFramelen <= taglen-curPos)
			{
				curPos += purelyFramelen;

				//in case of UTF 16 encoding
				//buffer+(curPos-purelyFramelen) data should '0x01' but in order to expansion, we don't accurately check the value.
				if(IS_ENCODEDBY_UTF16(buffer+(curPos-purelyFramelen)))
				{
					encodingOffSet = 2;
					textEncodingType = AV_ID3V2_UTF16;
				}
				else if (IS_ENCODEDBY_UTF16_R(buffer+(curPos-purelyFramelen)))
				{
					encodingOffSet = 2;
					textEncodingType = AV_ID3V2_UTF16_BE;
				}
				else if(IS_ENCODEDBY_UTF16(buffer+(curPos-purelyFramelen+1)))
				{
					encodingOffSet = 3;
					textEncodingType = AV_ID3V2_UTF16;
				}
				else if(IS_ENCODEDBY_UTF16_R(buffer+(curPos-purelyFramelen+1)))
				{
					encodingOffSet = 3;
					textEncodingType = AV_ID3V2_UTF16_BE;
				}
				else
				{
					//in case of UTF-16 BE encoding
					if(buffer[curPos-purelyFramelen] == 0x02)
					{
						encodingOffSet = 1;
						while((buffer[curPos-purelyFramelen+encodingOffSet] == '\0') && (encodingOffSet < purelyFramelen))
							encodingOffSet++;//null skip!
						textEncodingType = AV_ID3V2_UTF16_BE;
					}
					//in case of UTF8 encoding
					else if (buffer[curPos-purelyFramelen] == 0x03)
					{
						encodingOffSet = 1;
						while((buffer[curPos-purelyFramelen+encodingOffSet] == '\0') && (encodingOffSet < purelyFramelen))
							encodingOffSet++;//null skip!
						textEncodingType = AV_ID3V2_UTF8;
					}
					//in case of ISO-8859-1 encoding
					else
					{
						//buffer+(curPos-purelyFramelen) data should 0x00 but in order to expansion, we don't accurately check the value.
						encodingOffSet = 1;
						while((buffer[curPos-purelyFramelen+encodingOffSet] < 0x20) && (encodingOffSet < purelyFramelen))
							encodingOffSet++;//less than 0x20 value skip!
						textEncodingType = AV_ID3V2_ISO_8859;
					}
				}

				if(encodingOffSet < purelyFramelen)
				{
					realCpyFrameNum = purelyFramelen - encodingOffSet;
					pExtContent = mmfile_malloc (realCpyFrameNum+3);
					memset(pExtContent, '\0', realCpyFrameNum+3);

					if (textEncodingType != AV_ID3V2_UTF16 && textEncodingType != AV_ID3V2_UTF16_BE) {
						if (CompTmp[0] == 'T' ||(strcmp(CompTmp, "APIC")==0)) {
							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "get the new text ecoding type\n");
							#endif
							textEncodingType = buffer[curPos-purelyFramelen+encodingOffSet -1];
						}
					}

					memcpy(pExtContent, &buffer[curPos-purelyFramelen+encodingOffSet], purelyFramelen-encodingOffSet);

					if(realCpyFrameNum > 0)
					{
						if(strncmp((char *)CompTmp, "TIT2", 4) == 0 && pInfo->tagV2Info.bTitleMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pTitle= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pTitle, pExtContent, realCpyFrameNum);
								pInfo->pTitle[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->titleLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pTitle, pExtContent, pInfo->titleLen);
							}
							else
							{
								pInfo->pTitle = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->titleLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pTitle returned = (%s), pInfo->titleLen(%d)\n", pInfo->pTitle, pInfo->titleLen);
							#endif
							pInfo->tagV2Info.bTitleMarked = true;

						}
						else if(strncmp((char *)CompTmp, "TPE1", 4) == 0 && pInfo->tagV2Info.bArtistMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pArtist= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pArtist, pExtContent, realCpyFrameNum);
								pInfo->pArtist[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->artistLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pArtist, pExtContent, pInfo->artistLen);
							}
							else
							{
								pInfo->pArtist = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->artistLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pArtist returned = (%s), pInfo->artistLen(%d)\n", pInfo->pArtist, pInfo->artistLen);
							#endif
							pInfo->tagV2Info.bArtistMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TPE2", 4) == 0 && pInfo->tagV2Info.bAlbum_ArtistMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pAlbum_Artist= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pAlbum_Artist, pExtContent, realCpyFrameNum);
								pInfo->pAlbum_Artist[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->album_artistLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pAlbum_Artist, pExtContent, pInfo->album_artistLen);
							}
							else
							{
								pInfo->pAlbum_Artist = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->album_artistLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pAlbum_Artist returned = (%s), pInfo->album_artistLen(%d)\n", pInfo->pAlbum_Artist, pInfo->album_artistLen);
							#endif
							pInfo->tagV2Info.bAlbum_ArtistMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TPE3", 4) == 0 && pInfo->tagV2Info.bConductorMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pConductor= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pConductor, pExtContent, realCpyFrameNum);
								pInfo->pConductor[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->conductorLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pConductor, pExtContent, pInfo->conductorLen);
							}
							else
							{
								pInfo->pConductor = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->conductorLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pConductor returned = (%s), pInfo->conductorLen(%d)\n", pInfo->pConductor, pInfo->conductorLen);
							#endif
							pInfo->tagV2Info.bConductorMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TALB", 4) == 0 && pInfo->tagV2Info.bAlbumMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pAlbum= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pAlbum, pExtContent, realCpyFrameNum);
								pInfo->pAlbum[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->albumLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pAlbum, pExtContent, pInfo->albumLen);
							}
							else
							{
								pInfo->pAlbum = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->albumLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pAlbum returned = (%s), pInfo->albumLen(%d)\n", pInfo->pAlbum, pInfo->albumLen);
							#endif
							pInfo->tagV2Info.bAlbumMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TYER", 4) == 0 && pInfo->tagV2Info.bYearMarked == false)	//TODO. TYER is replaced by the TDRC. but many files use TYER in v2.4
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pYear= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pYear, pExtContent, realCpyFrameNum);
								pInfo->pYear[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->yearLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pYear, pExtContent, pInfo->yearLen);
							}
							else
							{
								pInfo->pYear = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->yearLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pYear returned = (%s), pInfo->yearLen(%d)\n", pInfo->pYear, pInfo->yearLen);
							#endif
							pInfo->tagV2Info.bYearMarked = true;
						}
						else if(strncmp((char *)CompTmp, "COMM", 4) == 0 && pInfo->tagV2Info.bDescriptionMarked == false)
						{
							if(realCpyFrameNum > 3)
							{
								realCpyFrameNum -= 3;
								tmp = 3;

								if(textEncodingType == AV_ID3V2_UTF16 || textEncodingType == AV_ID3V2_UTF16_BE)
								{
									while((NEWLINE_OF_UTF16(pExtContent + tmp) || NEWLINE_OF_UTF16_R(pExtContent + tmp))&& realCpyFrameNum > 4)
									{
										realCpyFrameNum -= 4;
										tmp += 4;
									}

									if((IS_ENCODEDBY_UTF16(pExtContent+tmp) || IS_ENCODEDBY_UTF16_R(pExtContent+tmp)) && realCpyFrameNum > 2)
									{
										realCpyFrameNum -= 2;
										tmp += 2;
										textEncodingType = AV_ID3V2_UTF16;
									}
									else
									{
										#ifdef __MMFILE_TEST_MODE__
										debug_msg ( "pInfo->pComment Never Get Here!!\n");
										#endif
									}
								}
								else if(textEncodingType == AV_ID3V2_UTF8)
								{
									while(pExtContent[tmp] < 0x20 && (tmp < realCpyFrameNum)) // text string encoded by ISO-8859-1
									{
										realCpyFrameNum --;
										tmp++;
									}
									textEncodingType = AV_ID3V2_UTF8;
								}
								else
								{
									while(pExtContent[tmp] < 0x20 && (tmp < realCpyFrameNum)) // text string encoded by ISO-8859-1
									{
										realCpyFrameNum --;
										tmp++;
									}
									textEncodingType = AV_ID3V2_ISO_8859;
								}

								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "tmp(%d) textEncodingType(%d), realCpyFrameNum(%d)\n", tmp, textEncodingType, realCpyFrameNum);
								#endif

								if(textEncodingType == AV_ID3V2_UTF8)
								{
									pInfo->pComment= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
									memset(pInfo->pComment, 0, (realCpyFrameNum+2));
									memcpy(pInfo->pComment, pExtContent+tmp, realCpyFrameNum);
									pInfo->pComment[realCpyFrameNum] = '\0';
									/*string copy with '\0'*/
									pInfo->commentLen = realCpyFrameNum;
									_STRNCPY_EX (pInfo->pComment, pExtContent, pInfo->commentLen);
								}
								else
								{
									pInfo->pComment = mmfile_string_convert ((const char*)&pExtContent[tmp], realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->commentLen);
								}
							}
							else
							{
								#ifdef __MMFILE_TEST_MODE__
								debug_msg (  "Description info too small to parse realCpyFrameNum(%d)\n", realCpyFrameNum);
								#endif
							}

							tmp = 0;

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pComment returned = (%s), pInfo->commentLen(%d)\n", pInfo->pComment, pInfo->commentLen);
							#endif
							pInfo->tagV2Info.bDescriptionMarked = true;
						}
						else if(strncmp((char *)CompTmp, "SYLT", 4) == 0 && pInfo->tagV2Info.bSyncLyricsMarked == false)
						{
							int idx = 0;
							int copy_len = 0;
							int copy_start_pos = tmp;
							AvSynclyricsInfo *synclyrics_info = NULL;
							GList *synclyrics_info_list = NULL;
							
							if(realCpyFrameNum > 5)
							{
								realCpyFrameNum -= 5;
								tmp = 5;

								if(textEncodingType == AV_ID3V2_UTF16 || textEncodingType == AV_ID3V2_UTF16_BE)
								{
									while((NEWLINE_OF_UTF16(pExtContent + tmp) || NEWLINE_OF_UTF16_R(pExtContent + tmp))&& realCpyFrameNum > 4)
									{
										realCpyFrameNum -= 4;
										tmp += 4;
									}

									if((IS_ENCODEDBY_UTF16(pExtContent+tmp) || IS_ENCODEDBY_UTF16_R(pExtContent+tmp)) && realCpyFrameNum > 2)
									{
										realCpyFrameNum -= 2;
										tmp += 2;
										textEncodingType = AV_ID3V2_UTF16;
									}
									else
									{
										#ifdef __MMFILE_TEST_MODE__
										debug_msg ( "pInfo->pSyncLyrics Never Get Here!!\n");
										#endif
									}
								}
								else if(textEncodingType == AV_ID3V2_UTF8)
								{
									while(pExtContent[tmp] < 0x20 && (tmp < realCpyFrameNum)) // text string encoded by ISO-8859-1
									{
										realCpyFrameNum --;
										tmp++;
									}
									textEncodingType = AV_ID3V2_UTF8;
								}
								else
								{
									while(pExtContent[tmp] < 0x20 && (tmp < realCpyFrameNum)) // text string encoded by ISO-8859-1
									{
										realCpyFrameNum --;
										tmp++;
									}
									textEncodingType = AV_ID3V2_ISO_8859;
								}

								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "tmp(%d) textEncodingType(%d), realCpyFrameNum(%d)\n", tmp, textEncodingType, realCpyFrameNum);
								#endif

								if(realCpyFrameNum < MMFILE_SYNC_LYRIC_INFO_MIN_LEN)
								{
									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "failed to get Synchronised lyrics Info realCpyFramNum(%d)\n", realCpyFrameNum);
									#endif
									pInfo->syncLyricsNum = 0;
								}
								else
								{
									if(textEncodingType == AV_ID3V2_UTF16)
									{
										debug_warning ("[AV_ID3V2_UTF16] not implemented\n");
									}
									else if(textEncodingType == AV_ID3V2_UTF16_BE)
									{
										debug_warning ("[AV_ID3V2_UTF16_BE] not implemented\n");
									}
									else
									{
										for(idx = 0; idx < realCpyFrameNum; idx++)
										{
											if(pExtContent[tmp+idx] == 0x00)
											{
												synclyrics_info = (AvSynclyricsInfo *)malloc(sizeof(AvSynclyricsInfo));

												if(textEncodingType == AV_ID3V2_UTF8) {
													synclyrics_info->lyric_info= mmfile_malloc(copy_len+1);
													memset(synclyrics_info->lyric_info, 0, copy_len+1);
													memcpy(synclyrics_info->lyric_info, pExtContent+copy_start_pos, copy_len);
													synclyrics_info->lyric_info[copy_len+1] = '\0';
												}
												else {
													synclyrics_info->lyric_info = mmfile_string_convert ((const char*)&pExtContent[copy_start_pos], copy_len, "UTF-8", charset_array[textEncodingType], NULL, NULL);
												}

												synclyrics_info->time_info= (unsigned long)pExtContent[tmp+idx+1] << 24 | (unsigned long)pExtContent[tmp+idx+2] << 16 | (unsigned long)pExtContent[tmp+idx+3] << 8  | (unsigned long)pExtContent[tmp+idx+4];
												idx += 4;
												copy_start_pos = tmp + idx + 1;
												#ifdef __MMFILE_TEST_MODE__
												debug_msg("[%d][%s] idx[%d], copy_len[%d] copy_start_pos[%d]", synclyrics_info->time_info, synclyrics_info->lyric_info, idx, copy_len, copy_start_pos);
												#endif
												copy_len = 0;
												synclyrics_info_list = g_list_append(synclyrics_info_list, synclyrics_info);
											}
											copy_len ++;
										}
										pInfo->pSyncLyrics = synclyrics_info_list;
										pInfo->syncLyricsNum = g_list_length(pInfo->pSyncLyrics);
									}
								}
							}
							else
							{
								#ifdef __MMFILE_TEST_MODE__
								debug_msg (  "SyncLyrics info too small to parse realCpyFrameNum(%d)\n", realCpyFrameNum);
								#endif
							}

							tmp = 0;
							pInfo->tagV2Info.bSyncLyricsMarked = true;
						}
						else if(strncmp((char *)CompTmp, "USLT", 4) == 0 && pInfo->tagV2Info.bUnsyncLyricsMarked == false)
						{
							if(realCpyFrameNum > 3)
							{
								realCpyFrameNum -= 3;
								tmp = 3;

								if(textEncodingType == AV_ID3V2_UTF16 || textEncodingType == AV_ID3V2_UTF16_BE)
								{
									while((NEWLINE_OF_UTF16(pExtContent + tmp) || NEWLINE_OF_UTF16_R(pExtContent + tmp))&& realCpyFrameNum > 4)
									{
										realCpyFrameNum -= 4;
										tmp += 4;
									}

									if((IS_ENCODEDBY_UTF16(pExtContent+tmp) || IS_ENCODEDBY_UTF16_R(pExtContent+tmp)) && realCpyFrameNum > 2)
									{
										realCpyFrameNum -= 2;
										tmp += 2;
										textEncodingType = AV_ID3V2_UTF16;
									}
									else
									{
										#ifdef __MMFILE_TEST_MODE__
										debug_msg ( "pInfo->pUnsyncLyrics Never Get Here!!\n");
										#endif
									}
								}
								else if(textEncodingType == AV_ID3V2_UTF8)
								{
									while(pExtContent[tmp] < 0x20 && (tmp < realCpyFrameNum)) // text string encoded by ISO-8859-1
									{
										realCpyFrameNum --;
										tmp++;
									}
									textEncodingType = AV_ID3V2_UTF8;
								}
								else
								{
									while(pExtContent[tmp] < 0x20 && (tmp < realCpyFrameNum)) // text string encoded by ISO-8859-1
									{
										realCpyFrameNum --;
										tmp++;
									}
									textEncodingType = AV_ID3V2_ISO_8859;
								}

								#ifdef __MMFILE_TEST_MODE__
								debug_msg ( "tmp(%d) textEncodingType(%d), realCpyFrameNum(%d)\n", tmp, textEncodingType, realCpyFrameNum);
								#endif

								if(textEncodingType == AV_ID3V2_UTF8)
								{
									pInfo->pUnsyncLyrics= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
									memset(pInfo->pUnsyncLyrics, 0, (realCpyFrameNum+2));
									memcpy(pInfo->pUnsyncLyrics, pExtContent+tmp, realCpyFrameNum);
									pInfo->pUnsyncLyrics[realCpyFrameNum] = '\0';
									/*string copy with '\0'*/
									pInfo->unsynclyricsLen = realCpyFrameNum;
									_STRNCPY_EX (pInfo->pUnsyncLyrics, pExtContent, pInfo->unsynclyricsLen);
								}
								else
								{
									pInfo->pUnsyncLyrics = mmfile_string_convert ((const char*)&pExtContent[tmp], realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->unsynclyricsLen);
								}
							}
							else
							{
								#ifdef __MMFILE_TEST_MODE__
								debug_msg (  "Description info too small to parse realCpyFrameNum(%d)\n", realCpyFrameNum);
								#endif
							}

							tmp = 0;

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pUnsyncLyrics returned = (%s), pInfo->unsynclyricsLen(%d)\n", pInfo->pUnsyncLyrics, pInfo->unsynclyricsLen);
							#endif
							pInfo->tagV2Info.bDescriptionMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TCON", 4) == 0 && pInfo->tagV2Info.bGenreMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pGenre= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pGenre, pExtContent, realCpyFrameNum);
								pInfo->pGenre[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->genreLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pGenre, pExtContent, pInfo->genreLen);
							}
							else
							{
								pInfo->pGenre = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->genreLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pGenre returned = (%s), pInfo->genreLen(%d)\n", pInfo->pGenre, pInfo->genreLen);
							#endif

							if((pInfo->pGenre != NULL) && (pInfo->genreLen > 0)) {
								bool ret = FALSE;
								int int_genre = -1;

								ret = is_numeric(pInfo->pGenre, pInfo->genreLen);

								if(ret == TRUE) {
									sscanf( pInfo->pGenre, "%d", &int_genre);
									#ifdef __MMFILE_TEST_MODE__
									debug_msg("genre information is inteager [%d]\n", int_genre);
									#endif

									//Change int to string
									if((0 <= int_genre) && (int_genre < GENRE_COUNT-1)) {
										/*save genreinfo like "(123)". mm_file_id3tag_restore_content_info convert it to string*/
										char tmp_genre[6] = {0,};	/*ex. "(123)+NULL"*/
										int tmp_genre_len = 0;

										memset (tmp_genre, 0, 6);
										snprintf(tmp_genre, sizeof(tmp_genre), "(%d)", int_genre);

										tmp_genre_len = strlen(tmp_genre);
										if(tmp_genre_len > 0) {
											if(pInfo->pGenre) _FREE_EX(pInfo->pGenre);
											pInfo->pGenre = mmfile_malloc (sizeof(char) * (tmp_genre_len + 1) );
											if (pInfo->pGenre)
											{
												strncpy(pInfo->pGenre, tmp_genre, tmp_genre_len);
												pInfo->pGenre[tmp_genre_len] = 0;
											}
										}
									}
								}
							}

							pInfo->tagV2Info.bGenreMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TRCK", 4) == 0 && pInfo->tagV2Info.bTrackNumMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pTrackNum= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pTrackNum, pExtContent, realCpyFrameNum);
								pInfo->pTrackNum[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->tracknumLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pTrackNum, pExtContent, pInfo->tracknumLen);
							}
							else
							{
								pInfo->pTrackNum = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->tracknumLen);
							}


							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pTrackNum returned = (%s), pInfo->tracknumLen(%d)\n", pInfo->pTrackNum, pInfo->tracknumLen);
							#endif
							pInfo->tagV2Info.bTrackNumMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TENC", 4) == 0 && pInfo->tagV2Info.bEncByMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pEncBy= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pEncBy, pExtContent, realCpyFrameNum);
								pInfo->pEncBy[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->encbyLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pEncBy, pExtContent, pInfo->encbyLen);
							}
							else
							{
								pInfo->pEncBy = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->encbyLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pEncBy returned = (%s), pInfo->encbyLen(%d)\n", pInfo->pEncBy, pInfo->encbyLen);
							#endif
							pInfo->tagV2Info.bEncByMarked = true;
						}
						else if(strncmp((char *)CompTmp, "WXXX", 4) == 0 && pInfo->tagV2Info.bURLMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pURL= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pURL, pExtContent, realCpyFrameNum);
								pInfo->pURL[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->urlLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pURL, pExtContent, pInfo->urlLen);
							}
							else
							{
								pInfo->pURL = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->urlLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pURL returned = (%s), pInfo->urlLen(%d)\n", pInfo->pURL, pInfo->urlLen);
							#endif
							pInfo->tagV2Info.bURLMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TCOP", 4) == 0 && pInfo->tagV2Info.bCopyRightMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pCopyright= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pCopyright, pExtContent, realCpyFrameNum);
								pInfo->pCopyright[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->copyrightLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pCopyright, pExtContent, pInfo->copyrightLen);
							}
							else
							{
								pInfo->pCopyright = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->copyrightLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pCopyright returned = (%s), pInfo->copyrightLen(%d)\n", pInfo->pCopyright, pInfo->copyrightLen);
							#endif
							pInfo->tagV2Info.bCopyRightMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TOPE", 4) == 0 && pInfo->tagV2Info.bOriginArtistMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pOriginArtist= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pOriginArtist, pExtContent, realCpyFrameNum);
								pInfo->pOriginArtist[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->originartistLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pOriginArtist, pExtContent, pInfo->originartistLen);
							}
							else
							{
								pInfo->pOriginArtist = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->originartistLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pOriginArtist returned = (%s), pInfo->originartistLen(%d)\n", pInfo->pOriginArtist, pInfo->originartistLen);
							#endif
							pInfo->tagV2Info.bOriginArtistMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TCOM", 4) == 0 && pInfo->tagV2Info.bComposerMarked == false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pComposer= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pComposer, pExtContent, realCpyFrameNum);
								pInfo->pComposer[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->composerLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pComposer, pExtContent, pInfo->composerLen);
							}
							else
							{
								pInfo->pComposer = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->composerLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pComposer returned = (%s), pInfo->originartistLen(%d)\n", pInfo->pComposer, pInfo->composerLen);
							#endif
							pInfo->tagV2Info.bComposerMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TDRC", 4) == 0 && pInfo->tagV2Info.bRecDateMarked== false)	//TYER(year) and TRDA are replaced by the TDRC
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pRecDate= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pRecDate, pExtContent, realCpyFrameNum);
								pInfo->pRecDate[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->recdateLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pRecDate, pExtContent, pInfo->recdateLen);
							}
							else
							{
								pInfo->pRecDate = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->recdateLen);
							}

							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pRecDate returned = (%s), pInfo->recdateLen(%d)\n", pInfo->pRecDate, pInfo->recdateLen);
							#endif
							pInfo->tagV2Info.bRecDateMarked = true;
						}
						else if(strncmp((char *)CompTmp, "TIT1", 4) == 0 && pInfo->tagV2Info.bContentGroupMarked== false)
						{
							if(textEncodingType == AV_ID3V2_UTF8)
							{
								pInfo->pContentGroup= mmfile_malloc (realCpyFrameNum+2);//Ignore NULL char for UTF16
								memcpy(pInfo->pContentGroup, pExtContent, realCpyFrameNum);
								pInfo->pContentGroup[realCpyFrameNum] = '\0';
								/*string copy with '\0'*/
								pInfo->contentGroupLen = realCpyFrameNum;
								_STRNCPY_EX (pInfo->pContentGroup, pExtContent, pInfo->contentGroupLen);
							}
							else
							{
								pInfo->pContentGroup = mmfile_string_convert ((const char*)pExtContent, realCpyFrameNum, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&pInfo->contentGroupLen);
							}
							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "pInfo->pContentGroup returned = (%s), pInfo->contentGroupLen(%d)\n", pInfo->pContentGroup, pInfo->contentGroupLen);
							#endif
							pInfo->tagV2Info.bContentGroupMarked = true;
						}
						else if(strncmp((char *)CompTmp, "APIC", 4) == 0 && pInfo->tagV2Info.bImageMarked == false && realCpyFrameNum <= 2000000)
						{
							if(pExtContent[0] != '\0')
							{
								for(inx = 0; inx < MP3_ID3_IMAGE_MIME_TYPE_MAX_LENGTH-1; inx++)
									pInfo->imageInfo.imageMIMEType[inx] = '\0';//ini mimetype variable

								while((checkImgMimeTypeMax < MP3_ID3_IMAGE_MIME_TYPE_MAX_LENGTH-1) && pExtContent[checkImgMimeTypeMax] != '\0')
								{
									pInfo->imageInfo.imageMIMEType[checkImgMimeTypeMax] = pExtContent[checkImgMimeTypeMax];
									checkImgMimeTypeMax++;
								}
								pInfo->imageInfo.imgMimetypeLen = checkImgMimeTypeMax;
							}
							else
							{
								pInfo->imageInfo.imgMimetypeLen = 0;
							}

							imgstartOffset += checkImgMimeTypeMax;

							if((pExtContent[imgstartOffset] == '\0') && (realCpyFrameNum - imgstartOffset > 0))
							{
								imgstartOffset++;//endofMIME(1byte)

								if(pExtContent[imgstartOffset] < AV_ID3V2_PICTURE_TYPE_MAX)
								{
									pInfo->imageInfo.pictureType = pExtContent[imgstartOffset];
								}
								imgstartOffset++;//PictureType(1byte)

								if(pExtContent[imgstartOffset] != 0x0)
								{
									int cur_pos = 0;
									int dis_len = 0;
									int new_dis_len = 0;
									char jpg_sign[3] = {0xff, 0xd8, 0xff};
									char png_sign[8] = {0x80, 0x50, 0x4e, 0x47,0x0d, 0x0a, 0x1a, 0x0a};
									char *tmp_desc = NULL;

									while (1) {
										if (pExtContent[imgstartOffset + cur_pos] == '\0') {
											if (realCpyFrameNum < imgstartOffset + cur_pos) {
												debug_error("End of APIC Tag %d %d %d\n", realCpyFrameNum, imgstartOffset, cur_pos);
												break;
											}
											/*check end of image description*/
											if ((pExtContent[imgstartOffset + cur_pos + 1] == jpg_sign[0]) ||
												(pExtContent[imgstartOffset + cur_pos + 1] == png_sign[0])) {
												#ifdef __MMFILE_TEST_MODE__
												debug_msg ( "length of description (%d)", cur_pos);
												#endif

												break;
											}
										}
										cur_pos ++;
									}

									dis_len = cur_pos + 1;

									tmp_desc = mmfile_malloc(sizeof(char) * dis_len);
									debug_msg ( "tmp_desc %x\n", tmp_desc);
									memcpy(tmp_desc, pExtContent + imgstartOffset, dis_len);
									debug_msg ( "tmp_desc %s\n", tmp_desc);

									/*convert description*/
									pInfo->imageInfo.imageDescription = mmfile_string_convert (tmp_desc, dis_len, "UTF-8", charset_array[textEncodingType], NULL, (unsigned int*)&new_dis_len);
									debug_msg ( "new_desc %s(%d)\n", pInfo->imageInfo.imageDescription, new_dis_len);
									mmfile_free(tmp_desc);

									pInfo->imageInfo.imgDesLen = new_dis_len; /**/
									imgstartOffset += cur_pos;
								}
								else
								{
									pInfo->imageInfo.imgDesLen= 0;
								}

								if((pExtContent[imgstartOffset] == '\0') && (realCpyFrameNum - imgstartOffset > 0))
								{
									imgstartOffset ++; // endofDesceriptionType(1byte)

									while(pExtContent[imgstartOffset] == '\0')	//some content has useless '\0' in front of picture data
									{
										imgstartOffset ++;
									}

									#ifdef __MMFILE_TEST_MODE__
									debug_msg ( "after scaning imgDescription imgstartOffset(%d) value!\n", imgstartOffset);
									#endif

									if(realCpyFrameNum - imgstartOffset > 0)
									{
										pInfo->imageInfo.imageLen = realCpyFrameNum - imgstartOffset;
										pInfo->imageInfo.pImageBuf= mmfile_malloc (pInfo->imageInfo.imageLen+1);
										memcpy(pInfo->imageInfo.pImageBuf, pExtContent+ imgstartOffset, pInfo->imageInfo.imageLen);
										pInfo->imageInfo.pImageBuf[pInfo->imageInfo.imageLen] = 0;
										if(IS_INCLUDE_URL(pInfo->imageInfo.imageMIMEType))
											pInfo->imageInfo.bURLInfo = true; //if mimetype is "-->", image date has an URL
									}
									else
									{
										#ifdef __MMFILE_TEST_MODE__
										debug_msg ( "No APIC image!! realCpyFrameNum(%d) - imgstartOffset(%d)\n", realCpyFrameNum, imgstartOffset);
										#endif
									}
								}
							}

							checkImgMimeTypeMax = 0;
							inx = 0;
							imgstartOffset = 0;
							pInfo->tagV2Info.bImageMarked= true;
						}
						else
						{
							#ifdef __MMFILE_TEST_MODE__
							debug_msg ( "CompTmp(%s) This Frame ID currently not Supports!!\n", CompTmp);
							#endif
						}
					}

				}
				else
				{
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "mmf_file_id3tag_parse_v224: All of the pExtContent Values are NULL\n");
					#endif
				}

			}
			else
			{
				curPos += purelyFramelen;
				if(purelyFramelen != 0)
					needToloopv2taglen = MP3_TAGv2_23_TXT_HEADER_LEN;
			}

			if(pExtContent)	_FREE_EX(pExtContent);
			memset(CompTmp, 0, 4);
			if(curPos < taglen)
			{
				needToloopv2taglen -= oneFrameLen;
				v2numOfFrames++;
			}
			else
				needToloopv2taglen = MP3_TAGv2_23_TXT_HEADER_LEN;

			oneFrameLen = 0;
			encodingOffSet = 0;
			realCpyFrameNum = 0;
			textEncodingType = 0;
			purelyFramelen = 0;

		}
	}

	release_characterset_array(charset_array);

	if(taglen)
		return true;
	else
		return false;

}

EXPORT_API
void mm_file_id3tag_restore_content_info(AvFileContentInfo* pInfo)
{
	char	*mpegAudioGenre=NULL, *tmpGenreForV1Tag = NULL;
	bool	bAdditionGenre= false /*, bMpegAudioFrame = false*/;
	int 	mpegAudioFileLen=0, idv2IntGenre=148, tmpinx=0/*, tmpinx2=0*/;
	char	*pGenreForUTF16;

	// for Genre Info
	if(pInfo->tagV2Info.bGenreMarked == false)
	{
		if(pInfo->bV1tagFound == true)
		{
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ( "Genre: %d\n", pInfo->genre);
			#endif
			if(pInfo->genre > 147)
			{
				mpegAudioGenre = MpegAudio_Genre[148];
			}
			else
			{
				mpegAudioGenre = MpegAudio_Genre[pInfo->genre];
			}

			if(mpegAudioGenre!=NULL)
			{
				pInfo->genreLen = strlen(mpegAudioGenre);
				if (pInfo->genreLen > 0)
				{
					// Give space for NULL character. Hence added "+1"
					pInfo->pGenre = mmfile_malloc (sizeof(char) * (pInfo->genreLen + 1) );
					if (pInfo->pGenre)
					{
						strncpy(pInfo->pGenre, mpegAudioGenre, pInfo->genreLen);
						pInfo->pGenre[pInfo->genreLen] = '\0';
					}
				}
			}
		}
		else
		{
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ( "Genre was not Found.\n");
			#endif
		}
	}
	else if(pInfo->tagV2Info.bGenreMarked == true)
	{
		if(pInfo->genreLen && pInfo->tagV2Info.bGenreUTF16)
		{
			pInfo->pGenre[pInfo->genreLen+1] = '\0';
			mpegAudioGenre = mmfile_malloc (sizeof(char) * (pInfo->genreLen*AV_WM_LOCALCODE_SIZE_MAX + 1) );
			pGenreForUTF16 = (char *)pInfo->pGenre;
#ifdef _SM_ONLY
			if(WmConvert2LCode(mpegAudioGenre, sizeof ( char ) * AV_WM_LOCALCODE_SIZE_MAX * ( pInfo->genreLen + 1 ), pGenreForUTF16))
			{
				pInfo->genreLen = strlen(mpegAudioGenre);
				mpegAudioGenre[pInfo->genreLen] = '\0';
			}
#endif
		}
		else
		{
			#ifdef __MMFILE_TEST_MODE__
			debug_msg (  "pInfo->genreLen size is Zero Or not UTF16 code! genreLen[%d] genre[%s]\n",pInfo->genreLen,pInfo->pGenre);
			#endif
			if (pInfo->pGenre) {
				pInfo->genreLen = strlen(pInfo->pGenre);
				mpegAudioGenre = mmfile_malloc (sizeof(char) * (pInfo->genreLen + 1) );
				mpegAudioGenre[pInfo->genreLen] = '\0';
				strncpy(mpegAudioGenre, pInfo->pGenre, pInfo->genreLen);
			} else {
				pInfo->genreLen = 0;
			}
		}

		if(pInfo->pGenre) _FREE_EX(pInfo->pGenre);

		tmpinx = 0;
		if(mpegAudioGenre!=NULL)
		{
			/**
			 *Genre number
			 * (XXX)	XXX is 0 - 148
			 */
			pInfo->genreLen = strlen (mpegAudioGenre);
			if (pInfo->genreLen >= 3 &&
				mpegAudioGenre[0] == '(' && mpegAudioGenre[pInfo->genreLen - 1] == ')') {
				bAdditionGenre = true;
				for (mpegAudioFileLen = 1; mpegAudioFileLen <= pInfo->genreLen - 2; mpegAudioFileLen++) {
					if(mpegAudioGenre[mpegAudioFileLen] < '0' || mpegAudioGenre[mpegAudioFileLen] > '9') {
						bAdditionGenre = false;
						break;
					}
				}
			}

			if(bAdditionGenre == true)
			{
				idv2IntGenre = atoi(mpegAudioGenre+1);
				if(idv2IntGenre > 147 || idv2IntGenre < 0)
				{
					tmpGenreForV1Tag = MpegAudio_Genre[148];
				}
				else
				{
					tmpGenreForV1Tag = MpegAudio_Genre[idv2IntGenre];
				}
				if(tmpGenreForV1Tag!=NULL)
				{
					pInfo->genreLen = strlen(tmpGenreForV1Tag);
					if (pInfo->genreLen > 0)
					{
						// Give space for NULL character. Hence added "+1"
						pInfo->pGenre = mmfile_malloc (sizeof(char) * (pInfo->genreLen + 1) );
						if (pInfo->pGenre)
						{
							strncpy(pInfo->pGenre, tmpGenreForV1Tag, pInfo->genreLen);
							pInfo->pGenre[pInfo->genreLen] = 0;
						}
					}
				}
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ( "pInfo->pGenre = %s\n", pInfo->pGenre);
				#endif
			}
			else if (bAdditionGenre == false && pInfo->genreLen > 0)
			{
				/**
				 * Genre string.
				 */
			
				// Give space for NULL character. Hence added "+1"
				pInfo->pGenre = mmfile_malloc (sizeof(char) * (pInfo->genreLen + 1) );
				if (pInfo->pGenre)
				{
					strncpy(pInfo->pGenre, mpegAudioGenre, pInfo->genreLen);
					pInfo->pGenre[pInfo->genreLen] = '\0';
				}
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ( "pInfo->pGenre = %s, pInfo->genreLen = %d\n", pInfo->pGenre, pInfo->genreLen);
				#endif
			}
			else
			{
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ( "Failed to \"(...)\" value to genre = %s\n", pInfo->pGenre);
				#endif
			}
		}
		else
		{
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ( "mpegAudioGenre = %x\n", mpegAudioGenre);
			#endif
		}
		if(mpegAudioGenre)
			_FREE_EX(mpegAudioGenre);

	}
	else
	{
		#ifdef __MMFILE_TEST_MODE__
		debug_msg (  "Neither ID3 v1 nor v2 info doesn't have Genre Info.\n");
		#endif
	}

}

void mm_file_free_synclyrics_list(GList * synclyrics_list)
{
	int list_len = 0;
	int idx = 0;
	AvSynclyricsInfo *synclyrics_info = NULL;

	if (synclyrics_list == NULL) {
		return;
	}

	list_len = g_list_length(synclyrics_list);
	for (idx = 0; idx < list_len; idx++) {
		synclyrics_info = g_list_nth_data(synclyrics_list, idx);

		free(synclyrics_info->lyric_info);
		synclyrics_info->lyric_info = NULL;
		
		free(synclyrics_info);
		synclyrics_info = NULL;
	}

	if (synclyrics_list != NULL) {
		g_list_free(synclyrics_list);
		synclyrics_list = NULL;
	}
	
	return;
}

