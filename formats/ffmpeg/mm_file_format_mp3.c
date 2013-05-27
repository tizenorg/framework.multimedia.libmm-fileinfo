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

#include <sys/types.h>
#include <sys/stat.h>	/*stat*/
#include <unistd.h>

#include <stdlib.h>	/*malloc*/

#include "mm_debug.h"
#include "mm_file_utils.h"
#include "mm_file_format_private.h"
#include "mm_file_format_audio.h"
#include "mm_file_format_mp3.h"


#define __MMFILE_NEW_FRAME_FUNC

#ifdef __MMFILE_NEW_TAGLIBS__
#include "mm_file_format_tags.h"
#endif


#define AV_MP3_FIND_SYNC_LEN		1024*30
#undef MIN 
#define MIN(a, b) ((a) < (b) ? (a) : (b))


static const unsigned char mp3FrameMasking[4] = {0xFF,0xFE,0x0C,0x00};
static unsigned char mp3FrameDataValid[4];

static const int mp3BitRateTable[2][3][16] = {
	{	{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
		{0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
		{0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,}	},

	{	{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,}	}
};

static const int mp3SamRateTable[3][3] = 
{	{44100, 48000, 32000},
	{22050, 24000, 16000} ,
	{11025, 12000, 8000} 
};

#define IS_VALID_FRAME_MP3(x) \
  ((((x)[0] & mp3FrameMasking[0]) == mp3FrameDataValid[0]) && \
   (((x)[1] & mp3FrameMasking[1]) == mp3FrameDataValid[1]) && \
   (((x)[2] & mp3FrameMasking[2]) == mp3FrameDataValid[2]) && \
   (((x)[3] & mp3FrameMasking[3]) == mp3FrameDataValid[3])) 



/* interface functions */
int mmfile_format_read_stream_mp3 (MMFileFormatContext *formatContext);
int mmfile_format_read_frame_mp3  (MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int mmfile_format_read_tag_mp3    (MMFileFormatContext *formatContext);
int mmfile_format_close_mp3       (MMFileFormatContext *formatContext);

/* internal */
static int mmf_file_mp3_get_infomation (char *src, AvFileContentInfo* pInfo );

EXPORT_API
int mmfile_format_open_mp3 (MMFileFormatContext *formatContext)
{
    AvFileContentInfo *privateData = NULL;;
    int ret = 0;

	debug_fenter();
	
    if (NULL == formatContext)
    {
        debug_error("formatContext is NULL\n");
        return MMFILE_FORMAT_FAIL;
    }

	if (formatContext->pre_checked == 0) {
		ret = MMFileFormatIsValidMP3 (formatContext->uriFileName,5);
		if ( ret == 0 )
		{
			debug_error("It is not mp3 file\n");
			return MMFILE_FORMAT_FAIL;        
		}
	}

    
    formatContext->ReadStream   = mmfile_format_read_stream_mp3;
    formatContext->ReadFrame    = mmfile_format_read_frame_mp3;
    formatContext->ReadTag      = mmfile_format_read_tag_mp3;
    formatContext->Close        = mmfile_format_close_mp3;

    formatContext->videoTotalTrackNum = 0;
    formatContext->audioTotalTrackNum = 1;

    privateData = mmfile_malloc (sizeof (AvFileContentInfo));
    if (!privateData)
    {
        debug_error ("error: mmfile_malloc MP3 privateData\n");
        return MMFILE_FORMAT_FAIL;
    }

    formatContext->privateFormatData = privateData;

    ret = mmf_file_mp3_get_infomation (formatContext->uriFileName, privateData);
    if ( ret == -1 )
    {
        debug_error ("error: mmfile_format_read_stream_mp3\n");
        goto exception;
    }
   
    return MMFILE_FORMAT_SUCCESS;
    
exception:
    mmfile_format_close_mp3 (formatContext);
    return MMFILE_FORMAT_FAIL;    
}


EXPORT_API
int mmfile_format_read_stream_mp3 (MMFileFormatContext *formatContext)
{
    AvFileContentInfo *privateData = NULL;
	debug_fenter();

    if (!formatContext || !formatContext->privateFormatData)
    {
        debug_error("formatContext is NULL\n");
        return MMFILE_FORMAT_FAIL;
    }

    privateData = formatContext->privateFormatData;

    formatContext->duration = privateData->duration;
    formatContext->videoTotalTrackNum = 0;
    formatContext->audioTotalTrackNum = 1;
    formatContext->nbStreams = 1;
    formatContext->streams[MMFILE_AUDIO_STREAM] = mmfile_malloc (sizeof (MMFileFormatStream));
    if (NULL == formatContext->streams[MMFILE_AUDIO_STREAM])
    {
        debug_error ("formatContext->streams[MMFILE_AUDIO_STREAM] is NULL\n");
        return MMFILE_FORMAT_FAIL;
    }
    
    formatContext->streams[MMFILE_AUDIO_STREAM]->streamType = MMFILE_AUDIO_STREAM;
    formatContext->streams[MMFILE_AUDIO_STREAM]->codecId = MM_AUDIO_CODEC_MP3;
    formatContext->streams[MMFILE_AUDIO_STREAM]->bitRate = (privateData->bitRate*1000);    
    formatContext->streams[MMFILE_AUDIO_STREAM]->framePerSec = (privateData->duration == 0 ? 0 : privateData->frameNum/privateData->duration);
    formatContext->streams[MMFILE_AUDIO_STREAM]->width = 0;
    formatContext->streams[MMFILE_AUDIO_STREAM]->height = 0;
    formatContext->streams[MMFILE_AUDIO_STREAM]->nbChannel = privateData->channels;
    formatContext->streams[MMFILE_AUDIO_STREAM]->samplePerSec = privateData->sampleRate;
    
    return MMFILE_FORMAT_SUCCESS;  
}


EXPORT_API
int mmfile_format_read_frame_mp3  (MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame)
{
    return MMFILE_FORMAT_SUCCESS;    
}

EXPORT_API
int mmfile_format_read_tag_mp3    (MMFileFormatContext *formatContext)
{
    AvFileContentInfo *privateData = NULL;
	debug_fenter();

    if (!formatContext || !formatContext->privateFormatData)
    {
        debug_error("formatContext is NULL\n");
        return MMFILE_FORMAT_FAIL;
    }

    privateData = formatContext->privateFormatData;

	if (privateData->pTitle)				formatContext->title = mmfile_strdup(privateData->pTitle);
	if (privateData->pArtist)			formatContext->artist = mmfile_strdup(privateData->pArtist);
	if (privateData->pAuthor)			formatContext->author = mmfile_strdup(privateData->pAuthor);
	if (privateData->pCopyright)		formatContext->copyright = mmfile_strdup(privateData->pCopyright);
	if (privateData->pComment)		formatContext->comment = mmfile_strdup(privateData->pComment);
	if (privateData->pAlbum)			formatContext->album = mmfile_strdup(privateData->pAlbum);
	if (privateData->pYear)				formatContext->year = mmfile_strdup(privateData->pYear);
	if (privateData->pGenre)			formatContext->genre = mmfile_strdup(privateData->pGenre);
	if (privateData->pTrackNum)		formatContext->tagTrackNum = mmfile_strdup(privateData->pTrackNum);
	if (privateData->pComposer)		formatContext->composer = mmfile_strdup(privateData->pComposer);
	if (privateData->pContentGroup)	formatContext->classification = mmfile_strdup(privateData->pContentGroup);
	if (privateData->pConductor)		formatContext->conductor = mmfile_strdup(privateData->pConductor);
	if (privateData->pUnsyncLyrics)		formatContext->unsyncLyrics= mmfile_strdup(privateData->pUnsyncLyrics);
	if (privateData->pSyncLyrics)		formatContext->syncLyrics= privateData->pSyncLyrics;
	if (privateData->syncLyricsNum)		formatContext->syncLyricsNum= privateData->syncLyricsNum;
	if (privateData->pRecDate)			formatContext->recDate= mmfile_strdup(privateData->pRecDate);

	if(privateData->imageInfo.imageLen > 0)
	{
		formatContext->artwork = mmfile_malloc (privateData->imageInfo.imageLen);
		if(formatContext->artwork)
		{
			formatContext->artworkSize = privateData->imageInfo.imageLen;
			memcpy (formatContext->artwork, privateData->imageInfo.pImageBuf, privateData->imageInfo.imageLen);
			if (strlen(privateData->imageInfo.imageMIMEType) > 0)
				formatContext->artworkMime= mmfile_strdup(privateData->imageInfo.imageMIMEType);
			else if(strlen(privateData->imageInfo.imageExt) > 0) {
				debug_msg("ID3 tag V2 File");
				formatContext->artworkMime= mmfile_strdup(privateData->imageInfo.imageExt);
			}
			else {
				debug_error("Album art image exist but there is no type information of album art\n");
			}
		}
	}

	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_close_mp3 (MMFileFormatContext *formatContext)
{
    AvFileContentInfo *privateData = NULL;

    if (formatContext)
    {
        privateData = formatContext->privateFormatData;
        if (privateData)
        {
            mm_file_free_AvFileContentInfo (privateData);
            mmfile_free (privateData);
            formatContext->privateFormatData = NULL;
        }
    }

    return MMFILE_FORMAT_SUCCESS;      
}

static int
__AvExtractI4(unsigned char *buf)
{
	int I4;

	I4 = buf[0];
	I4 <<= 8;
	I4 |= buf[1];
	I4 <<= 8;
	I4 |= buf[2];
	I4 <<= 8;
	I4 |= buf[3];

	return I4;
}

static int
__AvExtractI2(unsigned char *buf)
{
	int I2;

	I2 = buf[0];
	I2 <<= 8;
	I2 |= buf[1];
	I2 <<= 8;

	return I2;
}

static int
__AvGetXingHeader( AvXHeadData* headData,  unsigned char *buf )
{
	int			index, headFlags;
	int			hId, hMode, hSrIndex;
	static int	mp3SamRateTable[4] = { 44100, 48000, 32000, 99999 };

	// get Xing header data
	headData->flags = 0;

	// get selected MP3 header data
	hId			= (buf[1] >> 3) & 1;
	hSrIndex	= (buf[2] >> 2) & 3;
	hMode		= (buf[3] >> 6) & 3;


	// determine offset of header
	if( hId )	// mpeg1
	{
		if( hMode != 3 ) 
			buf += (32+4);
		else
			buf += (17+4);
	}
	else		// mpeg2
	{
		if( hMode != 3 )
			buf += (17+4);
		else
			buf += (9+4);
	}

	/* There could be 2 attrs in this header : Xing or Info */
	if (buf[0] == 'X') {
		if( buf[1] != 'i' ) return 0;
		if( buf[2] != 'n' ) return 0;
		if( buf[3] != 'g' ) return 0;
	} else if (buf[0] == 'I') {
		if( buf[1] != 'n' ) return 0;
		if( buf[2] != 'f' ) return 0;
		if( buf[3] != 'o' ) return 0;
	} else {
		return 0;
	}

	buf += 4;

	headData->hId = hId;
	headData->sampRate = mp3SamRateTable[hSrIndex];
	if( hId == 0 )
		headData->sampRate >>= 1;

	headFlags = headData->flags = __AvExtractI4( buf );		// get flags
	buf+=4;

	if( headFlags & FRAMES_FLAG )
	{
		headData->frames   = __AvExtractI4( buf );
		buf+=4;
	}
	if( headFlags & BYTES_FLAG )
	{
		headData->bytes = __AvExtractI4( buf ); 
		buf+=4;
	}

	if( headFlags & TOC_FLAG ) 
	{
		if( headData->toc != NULL ) 
		{
			for( index = 0; index < 100; index++ )
				headData->toc[index] = buf[index];
		}
		buf+=100;
	}

	headData->vbrScale = -1;
	if( headFlags & VBR_SCALE_FLAG )
	{
		headData->vbrScale = __AvExtractI4( buf );
		buf+=4;
	}

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("Xing header: sampling-rate:%d, stream-size:%d, frame-number:%d\n",
		headData->sampRate, headData->bytes, headData->frames);
	#endif

	return 1;       // success
}

static int
__AvGetVBRIHeader( AvVBRIHeadData* headData,  unsigned char *buf )
{
	int			hId, hSrIndex;
	static int	mp3SamRateTable[4] = { 44100, 48000, 32000, 99999 };


	// get selected MP3 header data
	hId			= (buf[1] >> 3) & 1;
	hSrIndex	= (buf[2] >> 2) & 3;

	buf += (32+4);

	if( buf[0] != 'V' ) return 0;    // fail
	if( buf[1] != 'B' ) return 0;    // header not found
	if( buf[2] != 'R' ) return 0;
	if( buf[3] != 'I' ) return 0;
	buf += 4;

	headData->hId = hId;
	headData->sampRate = mp3SamRateTable[hSrIndex];
	if( hId == 0 )
		headData->sampRate >>= 1;

	headData->vID = __AvExtractI2( buf );		// get ver ID
	buf+=2;
	headData->delay = __AvExtractI2( buf );
	buf+=2;
	headData->qualityIndicator = buf[0];
	buf+=2;
	headData->bytes = __AvExtractI4( buf );
	buf+=4;
	headData->frames= __AvExtractI4( buf );
	buf+=4;
	headData->numOfTOC = __AvExtractI2( buf );
	buf+=2;
	headData->vbriScale = __AvExtractI2( buf );
	buf+=2;
	headData->sizePerTable = __AvExtractI2( buf );
	buf+=2;
	headData->framesPerTable = __AvExtractI2( buf );
	buf+=2;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("Vbri header: sampling-rate:%d, stream-size:%d, frame-number:%d\n",
		headData->sampRate, headData->bytes, headData->frames);
	#endif

	return true;       // success
}
static bool
__AvIsValidHeader(AvFileContentInfo* pInfo, unsigned char *buf)
{
	bool	bSync = false;

	if (VALID_SYNC(buf))
	{
		mp3FrameDataValid[0] = (0xFF) & (mp3FrameMasking[0]);
		mp3FrameDataValid[1] = (0xE0 | (buf[AV_MP3HDR_VERSION_OFS] & AV_MP3HDR_VERSION_M)
						| (buf[AV_MP3HDR_LAYER_OFS] & AV_MP3HDR_LAYER_M)) & (mp3FrameMasking[1]);
		mp3FrameDataValid[2] = (buf[AV_MP3HDR_SAMPLERATE_OFS] & AV_MP3HDR_SAMPLERATE_M) &
						(mp3FrameMasking[2]);
		mp3FrameDataValid[3] = (buf[AV_MP3HDR_CHANNEL_OFS] & AV_MP3HDR_CHANNEL_M) &
						(mp3FrameMasking[3]);

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("*** [%02x][%02x][%02x][%02x] : [%02x][%02x][%02x][%02x]",
				buf[0], buf[1], buf[2],buf[3],
				mp3FrameDataValid[0], mp3FrameDataValid[1], mp3FrameDataValid[2],mp3FrameDataValid[3]);
		#endif

		/*
		 * MPEG Audio Layer I/II/III frame header
		 * from : http://www.mp3-tech.org/programmer/frame_header.html		 *
		 *
		 * AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM
		 *
		 * A 	11 	(31-21) 	Frame sync (all bits must be set)
		 * B 	2 	(20,19) 	MPEG Audio version ID
		 * C 	2 	(18,17) 	Layer description
		 * D 	1 	(16) 	Protection bit
		 * E 	4 	(15,12) 	Bitrate index
		 * F 	2 	(11,10) 	Sampling rate frequency index
		 * G 	1 	(9) 	Padding bit
		 * H 	1 	(8) 	Private bit. This one is only informative.
		 * I 	2 	(7,6) 	Channel Mode
		 * J 	2 	(5,4) 	Mode extension (Only used in Joint stereo)
		 * K 	1 	(3) 	Copyright
		 * L 	1 	(2) 	Original
		 * M 	2 	(1,0) 	Emphasis
		 *
		 */

		/* Simple check for version, layer, bitrate, samplerate */
		if (	(buf[1] & 0x18) != 0x08 && /* 000XX000 : MPEG Audio version ID, XX=01 - reserved => 00001000(0x08) */
			(buf[1] & 0x06) != 0x00 && /* 00000XX0 : Layer description, XX=00 - reserved => 00000000(0x00) */
			(buf[2] & 0xF0) != 0xF0 && /* XXXX0000 : Bitrate index, XX=1111 - bad => 11110000(0xF0) */
			(buf[2] & 0x0C) != 0x0C)   /* 0000XX00 : Sampling rate frequency index , XX=11 -reserved => 00001100(0x0C) */
		{
			bSync = true;
		}
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ("=> %s\n", bSync? "Good!":"Bad...");
		#endif
	}

	if(bSync == true)
		return true;
	else 
		return false;
}

static bool
__AvParseMp3Header( AvFileContentInfo* pInfo,  unsigned char* header )
{
	unsigned char	result;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg  ("### [%02x][%02x][%02x][%02x] ###\n", header[0], header[1], header[2],header[3]);
	#endif

	// 1. Check the version of mp3
	result = header[1] & MASK_MPEG;
	switch (result)
	{
	case MASK_MPEG_1:
		pInfo->mpegVersion = AV_MPEG_VER_1;
		break;
	case MASK_MPEG_2:
		pInfo->mpegVersion = AV_MPEG_VER_2;
		break;
	case MASK_MPEG_25:
		pInfo->mpegVersion = AV_MPEG_VER_25;
		break;
	default:
		return false;
	}

	// 2. Get a layer
	result = header[1] & MASK_LAYER;
	switch (result)
	{
	case MASK_LAYER_1:
		pInfo->layer = AV_MP3_LAYER_1;
		break;
	case MASK_LAYER_2:
		pInfo->layer = AV_MP3_LAYER_2;
		break;
	case MASK_LAYER_3:
		pInfo->layer = AV_MP3_LAYER_3;
		break;
	default:
		return false;
	}
	
	// 3. bitrate
	result = header[2] >> 4;
	if ( pInfo->mpegVersion == AV_MPEG_VER_1 )
		pInfo->bitRate = mp3BitRateTable[0][pInfo->layer-1][(int)result] ;
	else
		pInfo->bitRate = mp3BitRateTable[1][pInfo->layer-1][(int)result] ;

	// 4. samplerate
	result = ( header[2] & MASK_SAMPLERATE ) >> 2;
	if ( result == 0x03 )
		return false;
	else
		pInfo->sampleRate = mp3SamRateTable[pInfo->mpegVersion - 1][(int)result];
	
	// 5. channel
	result = header[3] & MASK_CHANNEL;
	switch (result)
	{
	case MASK_CHANNEL_ST:
		pInfo->channelIndex = 0;
		pInfo->channels = 2;
		break;
	case MASK_CHANNEL_JS:
		pInfo->channelIndex = 1;
		pInfo->channels = 2;
		break;
	case MASK_CHANNEL_DC:
		pInfo->channelIndex = 2;
		pInfo->channels = 2;
		break;
	case MASK_CHANNEL_MN:
		pInfo->channelIndex = 3;;
		pInfo->channels = 1;
		break;
	default:
		return false;
	}

	//	6. padding
	result = header[2] & MASK_PADDING;
	if ( result == MASK_PADDING )
		pInfo->bPadding = true;
	else
		pInfo->bPadding = false;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("=> samplerate=%d, bitrate=%d, layer=%d, version=%d, channel=%d, padding=%d",
			 pInfo->sampleRate, pInfo->bitRate, pInfo->layer, pInfo->mpegVersion, pInfo->channels, pInfo->bPadding	);
	#endif
	
	return true;
}

static bool
__AvParseXingHeader( AvFileContentInfo* pInfo, unsigned char* buf )
{
	AvXHeadData data;
	memset( &data, 0x00, sizeof(AvXHeadData) );

	//	1. Xing Header
	/* There could be 2 attrs in this header : Xing or Info */
	if((pInfo->mpegVersion == AV_MPEG_VER_1) && (pInfo->channels == 2))
	{
		if (buf[36] =='X') {
			if( buf[37] != 'i' ) return false;
			if( buf[38] != 'n' ) return false;
			if( buf[39] != 'g' ) return false;
		} else if (buf[36] == 'I') {
			if( buf[37] != 'n' ) return false;
			if( buf[38] != 'f' ) return false;
			if( buf[39] != 'o' ) return false;
		} else {
			return false;
		}
	}
	else
	if((pInfo->mpegVersion == AV_MPEG_VER_2 || pInfo->mpegVersion == AV_MPEG_VER_25) && (pInfo->channels == 1))
	{
		if (buf[13] =='X') {
			if( buf[14] != 'i' ) return false;
			if( buf[15] != 'n' ) return false;
			if( buf[16] != 'g' ) return false;
		} else if (buf[13] == 'I') {
			if( buf[14] != 'n' ) return false;
			if( buf[15] != 'f' ) return false;
			if( buf[16] != 'o' ) return false;
		} else {
			return false;
		}
	}
	else
	{
		if (buf[21] =='X') {
			if( buf[22] != 'i' ) return false;
			if( buf[23] != 'n' ) return false;
			if( buf[24] != 'g' ) return false;
		} else if (buf[21] == 'I') {
			if( buf[22] != 'n' ) return false;
			if( buf[23] != 'f' ) return false;
			if( buf[24] != 'o' ) return false;
		} else {
			return false;
		}
	}

	//	2. TOC
	if ( pInfo->pToc )
		data.toc = (unsigned char *)(pInfo->pToc);

	if ( __AvGetXingHeader( &data, (unsigned char *)buf ) == 1 ) // VBR.
	{
		if (data.sampRate == 0 || data.bytes == 0 || data.frames == 0) {
			debug_error ("invalid Xing header\n");
			return false;
		}

		pInfo->datafileLen = data.bytes;
		pInfo->frameNum = data.frames;
		pInfo->frameSize = (int) ( (float) data.bytes / (float) data.frames )  ;
		pInfo->bVbr = true;
		return true;
	}
	else
		return false;
}

static bool
__AvParseVBRIHeader( AvFileContentInfo* pInfo, unsigned char* buf )
{
	AvVBRIHeadData data;
	memset( &data, 0x00, sizeof(AvVBRIHeadData) );

	//	1. Xing Header
	if((pInfo->mpegVersion == AV_MPEG_VER_1) && (pInfo->channels == 2))
	{
		if( buf[36] != 'V' ) return false;   
		if( buf[37] != 'B' ) return false;   
		if( buf[38] != 'R' ) return false;
		if( buf[39] != 'I' ) return false;
	}
	else
	if((pInfo->mpegVersion == AV_MPEG_VER_2) && (pInfo->channels == 1))
	{
		if( buf[36] != 'V' ) return false;   
		if( buf[37] != 'B' ) return false;   
		if( buf[38] != 'R' ) return false;
		if( buf[39] != 'I' ) return false;
	}
	else
	{
		if( buf[36] != 'V' ) return false;   
		if( buf[37] != 'B' ) return false;   
		if( buf[38] != 'R' ) return false;
		if( buf[39] != 'I' ) return false;	
	}

	//	2. TOC
	if ( pInfo->pToc )
		data.toc = (unsigned char*)(pInfo->pToc);

	if ( __AvGetVBRIHeader( &data, (unsigned char*)buf ) == 1 ) // VBR.
	{
		if (data.sampRate == 0 || data.bytes == 0 || data.frames == 0) {
			debug_error ("invalid Vbri header\n");
			return false;
		}

		pInfo->sampleRate = data.sampRate;
		pInfo->datafileLen = data.bytes;
		pInfo->frameNum = data.frames;
		pInfo->frameSize = (int) ( (float) data.bytes / (float) data.frames )  ;
		pInfo->bVbr = true;
		return true;
	}
	else
		return false;
}

#ifdef __MMFILE_NEW_FRAME_FUNC // from gst
static bool
__AvGetMp3FrameSize( AvFileContentInfo* pInfo )
{
	unsigned int frameSize = 0;
	if ( pInfo == NULL )
		return false;

	frameSize =  pInfo->bPadding;
	 if (pInfo->bitRate == 0) {
	   if (pInfo->layer == 1) {
		   frameSize *= 4;
		   frameSize += 0/* FIXME: possible_free_framelen*/;
		   pInfo->bitRate = frameSize * pInfo->sampleRate / 48000;
	   } else {
		   frameSize += 0/* FIXME: possible_free_framelen*/;
		   pInfo->bitRate = frameSize * pInfo->sampleRate /
	         ((pInfo->layer == AV_MP3_LAYER_3 && pInfo->mpegVersion != AV_MPEG_VER_1) ? 72000 : 144000);
	   }
	 } else {
	   /* calculating */
	   if (pInfo->layer == 1) {
		   frameSize = ((12000 * pInfo->bitRate / pInfo->sampleRate) + frameSize) * 4;
	   } else {
		   frameSize += ((pInfo->layer == AV_MP3_LAYER_3
	             && pInfo->mpegVersion != AV_MPEG_VER_1) ? 72000 : 144000) * pInfo->bitRate / pInfo->sampleRate;
	   }
	 }

	pInfo->frameSize = (int)frameSize;

	return true;
}
#endif


static bool 
__AvGetXingBitrate( AvFileContentInfo* pInfo )
{
	float	br, factor;
	int		padding;

	if ( pInfo == NULL || pInfo->bVbr == false )
		return false;

	if ( pInfo->bPadding )
		padding = 1;
	else
		padding = 0;

	if (pInfo->mpegVersion == AV_MPEG_VER_1 )	// MPEG version 1
	{
		if (pInfo->layer == AV_MP3_LAYER_1 )	// Layer 1
			factor = 48000.0;
		else						// Layer 2, 3
			factor = 144000.0;
	}
	else							// MPEG version 2
	{
		if (pInfo->layer == AV_MP3_LAYER_1 )	//	Layer 1
			factor = 24000.0;
		else						//	Layer 2, 3
			factor = 72000.0;
	}

	br = ( pInfo->frameSize - padding ) * pInfo->sampleRate / factor;

	pInfo->bitRate = (int) br;

	return true;
}

static bool
__AvGetVBRIBitrate( AvFileContentInfo* pInfo )
{
	float	br, factor;
	int		padding;

	if ( pInfo == NULL || pInfo->bVbr == false )
		return false;

	if ( pInfo->bPadding )
		padding = 1;
	else
		padding = 0;

	if (pInfo->mpegVersion == AV_MPEG_VER_1 )	// MPEG version 1
	{
		if (pInfo->layer == AV_MP3_LAYER_1 )	// Layer 1
			factor = 48000.0;
		else						// Layer 2, 3
			factor = 144000.0;
	}
	else							// MPEG version 2
	{
		if (pInfo->layer == AV_MP3_LAYER_1 )	//	Layer 1
			factor = 24000.0;
		else						//	Layer 2, 3
			factor = 72000.0;
	}

	br = ( pInfo->frameSize - padding ) * pInfo->sampleRate / factor;

	pInfo->bitRate = (int) br;

	return true;
}

static int __AvGetLastID3offset (MMFileIOHandle *fp, unsigned int *offset)
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



/*
 *	This fuction retrieves the start position of header.
 *	Param	_pFile [in]	Specifies the file pointer of mp3 file.
 *	This function returns the start position of header.
 */
static int
__AvFindStartOfMp3Header(MMFileIOHandle *hFile,  unsigned char *buf, AvFileContentInfo* pInfo)
{
	int		index=0;
	long	readLen;
	unsigned long	id3v2TagLen = 0;
	unsigned char	*pHeader = NULL;
	unsigned long  preHeaderGap = 0;
	unsigned long  frameLen = 0;
	unsigned long  nextFrameOff = 0;     /* Offset to the start of the next frame */
	unsigned long  nextFrameOffEnd = 0;
	unsigned long  bufLen = 0;
	bool bFoundSync = false;
	unsigned long  minLen;

	

	if(pInfo->fileLen > (_AV_MP3_HEADER_POSITION_MAX+ pInfo->tagV2Info.tagLen))
		bufLen = _AV_MP3_HEADER_POSITION_MAX;
	else
		bufLen = pInfo ->fileLen - pInfo->tagV2Info.tagLen;

	if(IS_ID3V2_TAG(buf))
	{
		

		if(pInfo->tagV2Info.tagVersion == 0x02)
		{
			
			if(!mm_file_id3tag_parse_v222(pInfo, buf))
				pInfo->tagV2Info.tagLen = 0;
		}
		else if (pInfo->tagV2Info.tagVersion == 0x03)
		{
			
			if(!mm_file_id3tag_parse_v223(pInfo, buf))
				pInfo->tagV2Info.tagLen = 0;
		}
		else if (pInfo->tagV2Info.tagVersion == 0x04)
		{
			
			if(!mm_file_id3tag_parse_v224(pInfo, buf)) // currently 2.4 ver pased by 2.3 routine
				pInfo->tagV2Info.tagLen = 0;
		}
		else
		{
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ( "pInfo->tagV2Info.tagVersion(%d)\n", pInfo->tagV2Info.tagVersion);
			#endif
		}

		id3v2TagLen = pInfo->tagV2Info.tagLen;

		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "id3v2TagLen(%d)\n", id3v2TagLen);
		#endif

		if(id3v2TagLen)
		{
			if (mmfile_seek (hFile, id3v2TagLen, SEEK_SET) < 0) {
				debug_error ( "seek failed.\n");
				return -1;
			}
			if ((readLen = mmfile_read (hFile, buf, bufLen)) <= 0) {
				debug_error ( "seek failed.\n");
				return -1;
			}
		}
		while(1)
		{
			if (preHeaderGap == bufLen -2)
				break;
			if(__AvIsValidHeader(pInfo, buf+preHeaderGap))
				break;
			preHeaderGap++;
		}
		
  	}
	else
	{
		while(1)
		{
			if (preHeaderGap == bufLen -2)
				break;
			if(__AvIsValidHeader(pInfo, buf+preHeaderGap))
				break;			
			preHeaderGap++;
		}

	}
	minLen = 4;

	buf += preHeaderGap;
	index += preHeaderGap;
	while (index <= (bufLen - minLen)) 
	{
		if(buf[0] == 0xff)
		{
			if(VALID_SYNC(buf))
			{
				if(bufLen - index > 256)
				{
					pHeader = mmfile_malloc (256);
					if (pHeader == NULL)
					{
						debug_error ( "malloc failed.\n");
						return -1;
					}
					strncpy((char *)pHeader, (char *)buf, 256);
				}
				else
				{
					debug_error ( "Header field is not exist\n");
					return -1;
				}
				if ( __AvParseMp3Header( pInfo, pHeader ) == false)
				{
					//return -1;
					if(pHeader)
						_FREE_EX(pHeader);
					debug_warning ( "Mp3 parse header failed & index(%d)\n", index);
					buf++;
					index++;
					continue;
				}
				else
				{
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "This header is valid. index(%d)\n", index);
					#endif
				}

				if ( __AvParseXingHeader( pInfo, pHeader ) )
				{
					__AvGetXingBitrate( pInfo );
				}
				else if(__AvParseVBRIHeader( pInfo, pHeader ))
				{
					__AvGetVBRIBitrate( pInfo );
				}
				
				if (pInfo->bVbr)
				{
					if(pHeader)
						_FREE_EX(pHeader);
					bFoundSync = true;
					break;
				}
				else
				{
					if(__AvIsValidHeader(pInfo, pHeader))
					{
						if(pHeader)
							_FREE_EX(pHeader);
						
						__AvGetMp3FrameSize( pInfo );
						pInfo->datafileLen = pInfo->fileLen - pInfo->headerPos;
						frameLen = pInfo->frameSize;
						if (frameLen) 
						{
							#ifdef __MMFILE_TEST_MODE__
							debug_msg ("<<< frameLen=[%d] >>> \n", frameLen);
							#endif

							#ifndef __MMFILE_NEW_FRAME_FUNC // FIXME : what purpose to do this?
							/* Account for loss of precision in the frame length calculation*/
							frameLen--;
							#endif

							/* Check if the remaining buffer size is large enough to
							* look for another sync */
							if ((index + frameLen) < (bufLen - (minLen - 1))) 
							{
								nextFrameOff = frameLen;
								nextFrameOffEnd = nextFrameOff +MIN(6, bufLen - (index+frameLen) - (minLen - 1));

								/* Search the next few bytes for the next sync */
								while (nextFrameOff < nextFrameOffEnd) 
								{
									if (VALID_SYNC(buf+nextFrameOff)) 
									{
										if(IS_VALID_FRAME_MP3(buf+nextFrameOff)) 
										{
											bFoundSync = true;
											break;
										}
									}
									nextFrameOff++;
								}
								if (bFoundSync == true) 
									break;
							}
							else 
							{
								/* Assume that the first sync is valid, since there is not
								* enough data in the buffer to look for the next sync */
								bFoundSync = true;
							break;
							}
						}

					}				
					else
					{
						debug_warning ( "Is not vaild header pHeader\n");
					}
				}
				if(pHeader)
					_FREE_EX(pHeader);
			}
			else
			{
				debug_warning ( "Mp3 file frist byte is 0xff, but not header sync\n");
			}
		}
		buf++;
		index++;
	}

	_FREE_EX (pHeader);

	if (mmfile_seek(hFile, 0, SEEK_SET) < 0) {
		debug_error ( "seek error!\n");
		return -1;
	}
	if(index > (bufLen - minLen))
	{
		 debug_warning ( "Mp3 file sync is not found : index(%d) bufLen(%d), minLen(%d)\n", index, bufLen, minLen); 
		return -1;
	}
	
	if(bFoundSync == true)
	{
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Mp3 file found a sync Success!\n"); 
		#endif
	}
	else
	{
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Mp3 file found a sync Failed!\n");
		#endif
		return -1;
	}

	return index+id3v2TagLen;
}

/*
 *	This function retrieves the mp3 information.
 *	Param	szFileName [in] Specifies a mp3 file path.
 *	Param	_frame [out]	Specifies a struct pointer for mp3 information.
 *	This function returns true on success, or false on failure.
 */
static int mmf_file_mp3_get_infomation (char *filename, AvFileContentInfo* pInfo )
{
	MMFileIOHandle	*hFile;
	unsigned char	header[256];
	unsigned long   frameSamples=0;
	unsigned char	*buf = NULL;	
	unsigned char*	v2TagExistCheck = NULL;
	int 	readAmount = 0, readedDataLen = 0;
	unsigned char	TagBuff[MP3TAGINFO_SIZE + TAGV1_SEEK_GAP];
	unsigned char		TagV1ID[4] = { 0x54, 0x41, 0x47}; //TAG
	int		tagHeaderPos = 0;
	int ret = 0;
	unsigned int head_offset = 0;
	debug_fenter();
	
	if (pInfo == NULL || filename == NULL)
		return -1;

	memset( pInfo, 0x00, sizeof(AvFileContentInfo) );

	pInfo->tagV2Info.tagLen = 0;
	pInfo->headerPos = 0;
	pInfo->genre = 148;

	/*open*/
	ret = mmfile_open (&hFile, filename, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) 
	{
		debug_error ( "open failed.\n");
		return -1;
	}

	mmfile_seek (hFile, 0L, SEEK_END);
	pInfo->fileLen = mmfile_tell (hFile);
	if (pInfo->fileLen <= 0) 
	{
		debug_error ( "file is too small.\n");
		goto EXCEPTION;
	}
	mmfile_seek (hFile, 0L, SEEK_SET);

	v2TagExistCheck = mmfile_malloc (MP3_TAGv2_HEADER_LEN);
	if (v2TagExistCheck == NULL) {
		debug_error ( "malloc failed.\n");
		goto EXCEPTION;
	}

	if (mmfile_read (hFile, v2TagExistCheck, MP3_TAGv2_HEADER_LEN) > 0)
	{
		if(IS_ID3V2_TAG(v2TagExistCheck))
		{
			if(!(v2TagExistCheck[3] == 0xFF || v2TagExistCheck[4] == 0xFF ||v2TagExistCheck[6] >= 0x80 || v2TagExistCheck[7] >= 0x80 || v2TagExistCheck[8] >= 0x80 || v2TagExistCheck[9] >= 0x80)) 
			{
				if(!(v2TagExistCheck[3] > 0x04))
				{
					pInfo->tagV2Info.tagVersion = v2TagExistCheck[3];
					pInfo->tagV2Info.tagLen = MP3_TAGv2_HEADER_LEN;
					pInfo->tagV2Info.tagLen += (unsigned long)v2TagExistCheck[6] << 21 | (unsigned long)v2TagExistCheck[7] << 14 |(unsigned long)v2TagExistCheck[8] << 7  | (unsigned long)v2TagExistCheck[9];
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "pInfo->tagV2Info.tagLen(%d), pInfo->tagV2Info.tagVersion(%d)\n", pInfo->tagV2Info.tagLen, pInfo->tagV2Info.tagVersion);
					#endif
				}
				else
				{
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "tag is a not supported version(%d)\n", v2TagExistCheck[3]);
					#endif
				}
			}
			else
			{
				#ifdef __MMFILE_TEST_MODE__
				debug_msg ( "tag is a tag data is valid.\n");
				#endif
			}
		}
		else
		{
			#ifdef __MMFILE_TEST_MODE__
			debug_msg ( "this mp3 file is not included ID3v2 tag info!\n");
			#endif
		}
	}
	else
	{
		debug_error ( "v2TagExistCheck value read fail!\n");
		if(v2TagExistCheck)
			_FREE_EX(v2TagExistCheck);
		goto EXCEPTION;
	}

	if(v2TagExistCheck)
		_FREE_EX(v2TagExistCheck);

	if(!(pInfo->fileLen > pInfo->tagV2Info.tagLen ))
		pInfo->tagV2Info.tagLen = 0;

	if (mmfile_seek(hFile, 0L, SEEK_SET) < 0)
		goto EXCEPTION;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "pInfo->fileLen(%lld)\n", pInfo->fileLen);
	#endif

	if(pInfo->fileLen > (_AV_MP3_HEADER_POSITION_MAX + pInfo->tagV2Info.tagLen))
	{
		readAmount = _AV_MP3_HEADER_POSITION_MAX + pInfo->tagV2Info.tagLen;
		buf = mmfile_malloc (readAmount);
		if (buf == NULL) {
			debug_error ( "malloc failed.\n");
			goto EXCEPTION;
		}

		while(readAmount > 0)
		{
			if(readAmount >= AV_MP3_HEADER_READ_MAX)
			{
				if ((readedDataLen <= _AV_MP3_HEADER_POSITION_MAX + pInfo->tagV2Info.tagLen)
					&&(mmfile_read(hFile, buf+readedDataLen, AV_MP3_HEADER_READ_MAX) <= 0))
				{
					if(buf)
						_FREE_EX(buf);
					
					goto EXCEPTION;
				}
				else
				{
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "Reading buf readedDataLen(%d) readAmount (%d)\n", readedDataLen,readAmount);
					#endif
				}
			}
			else
			{
				if ((readedDataLen <= _AV_MP3_HEADER_POSITION_MAX + pInfo->tagV2Info.tagLen)
					&&(mmfile_read(hFile, buf+readedDataLen, readAmount) <= 0))
				{
					if(buf)
						_FREE_EX(buf);
					
					goto EXCEPTION;
				}
				else
				{
					#ifdef __MMFILE_TEST_MODE__
					debug_msg ( "The remained buf readed! readedDataLen(%d) readAmount (%d)\n", readedDataLen,readAmount);
					#endif
				}
			}

			readAmount -= AV_MP3_HEADER_READ_MAX;
			readedDataLen += AV_MP3_HEADER_READ_MAX;
			
			if(readAmount <= 0)
				break;
		}
	}
	else
	{
		buf = mmfile_malloc (pInfo->fileLen);
		if (buf == NULL)
		{
			 goto EXCEPTION;
		}

		if (mmfile_read(hFile, buf, pInfo->fileLen) <= 0)
		{
			if(buf)
				_FREE_EX(buf);
			goto EXCEPTION;
		}
	}

	
	if (__AvGetLastID3offset (hFile, &head_offset)) {
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "search start offset: %u\n", head_offset);
		#endif
		pInfo->tagV2Info.tagLen = head_offset;
	}
	
	pInfo->headerPos = (long) __AvFindStartOfMp3Header(hFile, buf, pInfo);

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "Header Pos: %ld\n", pInfo->headerPos);
	#endif

	if(buf)
		_FREE_EX(buf);

	if (pInfo->headerPos == -1)
		goto EXCEPTION;

	if (mmfile_seek(hFile, pInfo->headerPos, SEEK_SET) < 0)
		goto EXCEPTION;

	if (mmfile_read (hFile, header, 256) <= 0)
		goto EXCEPTION;

	if ( __AvParseMp3Header( pInfo, header ) == false)
		goto EXCEPTION;

	if ( __AvParseXingHeader( pInfo, header ) )
	{
		__AvGetXingBitrate( pInfo );
	}
	else if(__AvParseVBRIHeader( pInfo, header ))
	{
		__AvGetVBRIBitrate( pInfo );
	}
	else
	{
		__AvGetMp3FrameSize( pInfo );
		pInfo->datafileLen = pInfo->fileLen - pInfo->headerPos;
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Mp3 File FrameSize (%d) pInfo->headerPos(%d)\n", pInfo->frameSize,pInfo->headerPos);
		#endif
	}

	if (mmfile_seek (hFile, -(MP3TAGINFO_SIZE + TAGV1_SEEK_GAP), SEEK_END) < 0)
		goto EXCEPTION;


	pInfo ->bV1tagFound = false;

	if (mmfile_read (hFile, TagBuff, MP3TAGINFO_SIZE + TAGV1_SEEK_GAP) <= 0)
		goto EXCEPTION;

	if ((tagHeaderPos = __AvMemstr(TagBuff, TagV1ID, 3, TAGV1_SEEK_GAP+5)) >= 0)
	{
		#ifdef __MMFILE_TEST_MODE__
		debug_msg ( "Mp3 File Tag is existing\n");
		#endif

		pInfo ->bV1tagFound = true;
		memcpy(TagBuff, (TagBuff + tagHeaderPos), MP3TAGINFO_SIZE);

		if(!mm_file_id3tag_parse_v110(pInfo, TagBuff))
			goto EXCEPTION;
	}

	mm_file_id3tag_restore_content_info (pInfo);

	if(pInfo->mpegVersion== 1)
	{
		if(pInfo->layer== 1)
			frameSamples = MPEG_1_SIZE_LAYER_1;
		else
			frameSamples = MPEG_1_SIZE_LAYER_2_3;
	}
	else
	{
		if(pInfo->layer == 1)
			frameSamples = MPEG_2_SIZE_LAYER_1;
		else
			frameSamples = MPEG_2_SIZE_LAYER_2_3;
	}

#if 0
	unsigned long   numOfFrames=0;
	unsigned long long 	tempduration = 0;
	unsigned int 		tempNumFrames = 0;

	if(pInfo->bVbr) 
		numOfFrames = pInfo->frameNum*10;
	else
	{
		numOfFrames = ((pInfo->fileLen
		-(pInfo->headerPos + (pInfo ->bV1tagFound ? MP3TAGINFO_SIZE : 0) ) )*10) / pInfo->frameSize;
  	}
	tempNumFrames = (unsigned int)(numOfFrames/10);

	if((numOfFrames - tempNumFrames * 10 ) > 5)
		numOfFrames = (numOfFrames/10) + 1;
	else
		numOfFrames = numOfFrames/10;

	

	tempduration = (unsigned long long)(numOfFrames *1000);
	
	if(pInfo->mpegVersion== 1) 
	{
		if(pInfo->layer== 1) 
			frameSamples = MPEG_1_SIZE_LAYER_1;
		else 
			frameSamples = MPEG_1_SIZE_LAYER_2_3;
	}
	else 
	{
		if(pInfo->layer == 1) 
			frameSamples = MPEG_2_SIZE_LAYER_1;
		else 
			frameSamples = MPEG_2_SIZE_LAYER_2_3;
	}

	debug_msg("frameSamples : %d, tempduration : %ld", frameSamples, tempduration);

	if(tempduration < (unsigned long long)pInfo->sampleRate)
	{
		tempduration = (tempduration*frameSamples*10)/pInfo->sampleRate;
		tempduration = (tempduration/10);
	}
	else
		tempduration = (tempduration*frameSamples)/pInfo->sampleRate;

	pInfo->duration = tempduration;
#else
	int file_size_except_header = pInfo->fileLen - (pInfo->headerPos + (pInfo->bV1tagFound ? MP3TAGINFO_SIZE : 0));
	pInfo->duration = ((double)file_size_except_header / (double)pInfo->frameSize) * (frameSamples * 1000 / pInfo->sampleRate);
	debug_msg("duration from new algorithm : %lld", pInfo->duration);
#endif

	mmfile_close(hFile);
	
	/*debug print*/
	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "Mp3 File pInfo->duration (%lld) \n", pInfo->duration);
	debug_msg ( "** MP3 **\n");
	debug_msg ( "Version    : %u\n", pInfo->mpegVersion);
	debug_msg ( "Layer      : %u\n", pInfo->layer);
	debug_msg ( "Channel idx: %u\n", pInfo->channelIndex);
	debug_msg ( "Is VBR     : %d\n", (pInfo->bVbr == true ? 1 : 0));
	debug_msg ( "Bitrate    : %u\n", pInfo->bitRate);
	debug_msg ( "SampleRate : %u\n", pInfo->sampleRate);
	debug_msg ( "Channels   : %u\n", pInfo->channels);
	debug_msg ( "**** Info #1 ****\n");
	debug_msg ( "Title       : %s\n", pInfo->pTitle);
	debug_msg ( "Artist      : %s\n", pInfo->pArtist);
	debug_msg ( "Album       : %s\n", pInfo->pAlbum);
	debug_msg ( "Year        : %s\n", pInfo->pYear);
	debug_msg ( "Comment     : %s\n", pInfo->pComment);
	debug_msg ( "TrackNum    : %s\n", pInfo->pTrackNum);
	debug_msg ( "Genre       : %s\n", pInfo->pGenre);
	debug_msg ( "**** Info #2 ****\n");
	debug_msg ( "Author      : %s\n", pInfo->pAuthor);
	debug_msg ( "Copyright   : %s\n", pInfo->pCopyright);
	debug_msg ( "Comment : %s\n", pInfo->pComment);
	debug_msg ( "Rating      : %s\n", pInfo->pRating);
	debug_msg ( "RecDate     : %s\n", pInfo->pRecDate);
	debug_msg ( "Encoded by  : %s\n", pInfo->pEncBy);
	debug_msg ( "URL         : %s\n", pInfo->pURL);
	debug_msg ( "Ori. Artist : %s\n", pInfo->pOriginArtist);
	debug_msg ( "Composer    : %s\n", pInfo->pComposer);
	debug_msg ( "Conductor   : %s\n", pInfo->pConductor);
	debug_msg ( "Artwork     : mime(%s) addr(%p) size(%d)\n", pInfo->imageInfo.imageMIMEType, pInfo->imageInfo.pImageBuf, pInfo->imageInfo.imageLen);
	debug_msg ( "UnsyncLyrics   : %s\n", pInfo->pUnsyncLyrics);
	debug_msg ( "SyncLyrics size  : %d\n", pInfo->syncLyricsNum);

	#endif

	return 0;

EXCEPTION:
	debug_error ("Error occured!\n");
	mmfile_close(hFile);
	return -1;
}
