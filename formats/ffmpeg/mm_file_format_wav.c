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
#include <stdlib.h>	/*malloc*/

#include <mm_error.h>
#include "mm_debug.h"

#include "mm_file_utils.h"
#include "mm_file_format_private.h"
#include "mm_file_format_wav.h"


/**
 * Wave File Header.
 *
 * Offset  Size  Endian  Description        Value
 * 0       4     big     Chunk ID           "RIFF" (0x52494646)
 * 4       4     little  Chunk Data Size    (file size) - 8
 * 8       4     big     Format             "WAVE" (0x57415645)
 * 12      4     big     Sub Chunk1 ID      "fmt " (0x666d7420)
 * 16      4     little  Sub Chunk1 Size
 * 20      2     little  Audio Format
 * 22      2     little  Channel number
 * 24      4     little  Sampling rate
 * 28      4     little  Byte rate
 * 32      2     little  Block align
 * 34      2     little  Bit per sample
 */
 
 
#define MMF_FILE_WAVE_CHUNK_LEN					12
#define MMF_FILE_WAVE_SUBCHUNK_LEN				24
#define MMF_FILE_WAVE_HEADER_LEN				(MMF_FILE_WAVE_CHUNK_LEN + MMF_FILE_WAVE_SUBCHUNK_LEN)

#ifdef __MMFILE_TEST_MODE__
typedef struct {
	short	codec;		/**< WAVE form Registration Number*/
	char	*name;		/**< WAVE form wFormatTag ID*/
} MMF_FILE_WAVE_CODEC_NAME;

MMF_FILE_WAVE_CODEC_NAME g_audio_cdc_tbl [] = {
	{ 0x0000, "WAVE_FORMAT_UNKNOWN" },
	{ 0x0001, "WAVE_FORMAT_PCM" },
	{ 0x0002, "WAVE_FORMAT_ADPCM" },
	{ 0x0003, "WAVE_FORMAT_IEEE_FLOAT" },
	{ 0x0004, "WAVE_FORMAT_VSELP" },
	{ 0x0005, "WAVE_FORMAT_IBM_CVSD" },
	{ 0x0006, "WAVE_FORMAT_ALAW" },
	{ 0x0007, "WAVE_FORMAT_MULAW" },
	{ 0x0010, "WAVE_FORMAT_OKI_ADPCM" },
	{ 0x0011, "WAVE_FORMAT_DVI_ADPCM" },
	{ 0x0012, "WAVE_FORMAT_MEDIASPACE_ADPCM" },
	{ 0x0013, "WAVE_FORMAT_SIERRA_ADPCM" },
	{ 0x0014, "WAVE_FORMAT_G723_ADPCM" },
	{ 0x0015, "WAVE_FORMAT_DIGISTD" },
	{ 0x0016, "WAVE_FORMAT_DIGIFIX" },
	{ 0x0017, "WAVE_FORMAT_DIALOGIC_OKI_ADPCM" },
	{ 0x0018, "WAVE_FORMAT_MEDIAVISION_ADPCM" },
	{ 0x0019, "WAVE_FORMAT_CU_CODEC" },
	{ 0x0020, "WAVE_FORMAT_YAMAHA_ADPCM" },
	{ 0x0021, "WAVE_FORMAT_SONARC" },
	{ 0x0022, "WAVE_FORMAT_DSPGROUP_TRUESPEECH" },
	{ 0x0023, "WAVE_FORMAT_ECHOSC1" },
	{ 0x0024, "WAVE_FORMAT_AUDIOFILE_AF36" },
	{ 0x0025, "WAVE_FORMAT_APTX" },
	{ 0x0026, "WAVE_FORMAT_AUDIOFILE_AF10" },
	{ 0x0027, "WAVE_FORMAT_PROSODY_1612" },
	{ 0x0028, "WAVE_FORMAT_LRC" },
	{ 0x0030, "WAVE_FORMAT_DOLBY_AC2" },
	{ 0x0031, "WAVE_FORMAT_GSM610" },
	{ 0x0032, "WAVE_FORMAT_MSNAUDIO" },
	{ 0x0033, "WAVE_FORMAT_ANTEX_ADPCME" },
	{ 0x0034, "WAVE_FORMAT_CONTROL_RES_VQLPC" },
	{ 0x0035, "WAVE_FORMAT_DIGIREAL" },
	{ 0x0036, "WAVE_FORMAT_DIGIADPCM" },
	{ 0x0037, "WAVE_FORMAT_CONTROL_RES_CR10" },
	{ 0x0038, "WAVE_FORMAT_NMS_VBXADPCM" },
	{ 0x0039, "WAVE_FORMAT_ROLAND_RDAC" },
	{ 0x003A, "WAVE_FORMAT_ECHOSC3" },
	{ 0x003B, "WAVE_FORMAT_ROCKWELL_ADPCM" },
	{ 0x003C, "WAVE_FORMAT_ROCKWELL_DIGITALK" },
	{ 0x003D, "WAVE_FORMAT_XEBEC" },
	{ 0x0040, "WAVE_FORMAT_G721_ADPCM" },
	{ 0x0041, "WAVE_FORMAT_G728_CELP" },
	{ 0x0042, "WAVE_FORMAT_MSG723" },
	{ 0x0050, "WAVE_FORMAT_MPEG" },
	{ 0x0052, "WAVE_FORMAT_RT24" },
	{ 0x0053, "WAVE_FORMAT_PAC" },
	{ 0x0055, "WAVE_FORMAT_MPEGLAYER3" },
	{ 0x0059, "WAVE_FORMAT_LUCENT_G723" },
	{ 0x0060, "WAVE_FORMAT_CIRRUS" },
	{ 0x0061, "WAVE_FORMAT_ESPCM" },
	{ 0x0062, "WAVE_FORMAT_VOXWARE" },
	{ 0x0063, "WAVE_FORMAT_CANOPUS_ATRAC" },
	{ 0x0064, "WAVE_FORMAT_G726_ADPCM" },
	{ 0x0065, "WAVE_FORMAT_G722_ADPCM" },
	{ 0x0066, "WAVE_FORMAT_DSAT" },
	{ 0x0067, "WAVE_FORMAT_DSAT_DISPLAY" },
	{ 0x0069, "WAVE_FORMAT_VOXWARE_BYTE_ALIGNED" },
	{ 0x0070, "WAVE_FORMAT_VOXWARE_AC8" },
	{ 0x0071, "WAVE_FORMAT_VOXWARE_AC10" },
	{ 0x0072, "WAVE_FORMAT_VOXWARE_AC16" },
	{ 0x0073, "WAVE_FORMAT_VOXWARE_AC20" },
	{ 0x0074, "WAVE_FORMAT_VOXWARE_RT24" },
	{ 0x0075, "WAVE_FORMAT_VOXWARE_RT29" },
	{ 0x0076, "WAVE_FORMAT_VOXWARE_RT29HW" },
	{ 0x0077, "WAVE_FORMAT_VOXWARE_VR12" },
	{ 0x0078, "WAVE_FORMAT_VOXWARE_VR18" },
	{ 0x0079, "WAVE_FORMAT_VOXWARE_TQ40" },
	{ 0x0080, "WAVE_FORMAT_SOFTSOUND" },
	{ 0x0081, "WAVE_FORMAT_VOXWARE_TQ60" },
	{ 0x0082, "WAVE_FORMAT_MSRT24" },
	{ 0x0083, "WAVE_FORMAT_G729A" },
	{ 0x0084, "WAVE_FORMAT_MVI_MV12" },
	{ 0x0085, "WAVE_FORMAT_DF_G726" },
	{ 0x0086, "WAVE_FORMAT_DF_GSM610" },
	{ 0x0088, "WAVE_FORMAT_ISIAUDIO" },
	{ 0x0089, "WAVE_FORMAT_ONLIVE" },
	{ 0x0091, "WAVE_FORMAT_SBC24" },
	{ 0x0092, "WAVE_FORMAT_DOLBY_AC3_SPDIF" },
	{ 0x0097, "WAVE_FORMAT_ZYXEL_ADPCM" },
	{ 0x0098, "WAVE_FORMAT_PHILIPS_LPCBB" },
	{ 0x0099, "WAVE_FORMAT_PACKED" },
	{ 0x0100, "WAVE_FORMAT_RHETOREX_ADPCM" },
	{ 0x0101, "WAVE_FORMAT_IRAT" },
	{ 0x0111, "WAVE_FORMAT_VIVO_G723" },
	{ 0x0112, "WAVE_FORMAT_VIVO_SIREN" },
	{ 0x0123, "WAVE_FORMAT_DIGITAL_G723" },
	{ 0x0200, "WAVE_FORMAT_CREATIVE_ADPCM" },
	{ 0x0202, "WAVE_FORMAT_CREATIVE_FASTSPEECH8" },
	{ 0x0203, "WAVE_FORMAT_CREATIVE_FASTSPEECH10" },
	{ 0x0220, "WAVE_FORMAT_QUARTERDECK" },
	{ 0x0300, "WAVE_FORMAT_FM_TOWNS_SND" },
	{ 0x0400, "WAVE_FORMAT_BTV_DIGITAL" },
	{ 0x0680, "WAVE_FORMAT_VME_VMPCM" },
	{ 0x1000, "WAVE_FORMAT_OLIGSM" },
	{ 0x1001, "WAVE_FORMAT_OLIADPCM" },
	{ 0x1002, "WAVE_FORMAT_OLICELP" },
	{ 0x1003, "WAVE_FORMAT_OLISBC" },
	{ 0x1004, "WAVE_FORMAT_OLIOPR" },
	{ 0x1100, "WAVE_FORMAT_LH_CODEC" },
	{ 0x1400, "WAVE_FORMAT_NORRIS" },
	{ 0x1401, "WAVE_FORMAT_ISIAUDIO" },
	{ 0x1500, "WAVE_FORMAT_SOUNDSPACE_MUSICOMPRESS" },
	{ 0x2000, "WAVE_FORMAT_DVM" },
};
#endif

typedef struct {
	int		size;				/**< Chunk size*/
	short	format;				/**< Wave form Resistration Number*/
	short	channel;			/**< Number of channels*/
	int		sample_rate;		/**< Sampling-rate per second*/
	int		byte_rate;			/**< Byte per second (== Sampling-rate * Channels * Bit per Sample / 8)*/
	short	block_align;		/**< Block align(== Channels * Bit per Sample / 8)*/
	short	bits_per_sample;	/**< Bit per sample*/

} MM_FILE_WAVE_INFO;


/* internal */
static unsigned char *mmf_file_wave_get_header (char *src);
static int mmf_file_wave_get_info (unsigned char *header, MM_FILE_WAVE_INFO *info);


/* mm plugin porting */
int mmfile_format_read_stream_wav (MMFileFormatContext *formatContext);
int mmfile_format_read_frame_wav  (MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int mmfile_format_read_tag_wav    (MMFileFormatContext *formatContext);
int mmfile_format_close_wav       (MMFileFormatContext *formatContext);


EXPORT_API
int mmfile_format_open_wav (MMFileFormatContext *formatContext)
{
	int ret = 0;

	if (NULL == formatContext) {
		debug_error("formatContext is NULL\n");
		return MMFILE_FORMAT_FAIL;
	}

	if (formatContext->pre_checked == 0) {
		ret = MMFileFormatIsValidWAV (formatContext->uriFileName);
		if ( ret == 0 )
		{
			debug_error("It is not wav file\n");
			return MMFILE_FORMAT_FAIL;        
		}
	}

	formatContext->ReadStream   = mmfile_format_read_stream_wav;
	formatContext->ReadFrame    = mmfile_format_read_frame_wav;
	formatContext->ReadTag      = mmfile_format_read_tag_wav;
	formatContext->Close        = mmfile_format_close_wav;

	formatContext->videoTotalTrackNum = 0;
	formatContext->audioTotalTrackNum = 1;

	formatContext->privateFormatData = NULL;

	return MMFILE_FORMAT_SUCCESS;
}

static bool __check_uhqa(int sample_rate,  short bits_per_sample)
{
	bool ret = FALSE;

#ifdef __MMFILE_TEST_MODE__
	debug_error("[sample rate %d, sample format %d]", sample_rate, bits_per_sample);
#endif

	if ((sample_rate >= 44100) && (bits_per_sample >= 24)) {
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
int mmfile_format_read_stream_wav (MMFileFormatContext *formatContext)
{
	unsigned char *header = NULL;
	MM_FILE_WAVE_INFO *waveinfo = NULL;
	long long     filesize = 0;
	MMFileIOHandle *fp = NULL;
	int ret = 0;

	if (formatContext == NULL) {
		debug_error("formatContext is NULL\n");
		return MMFILE_FORMAT_FAIL;    
	}

	header = mmf_file_wave_get_header (formatContext->uriFileName);
	if (header == NULL) {
		debug_error("error: mmf_file_wave_get_header\n");
		goto exception;
	}

	waveinfo = mmfile_malloc (sizeof(MM_FILE_WAVE_INFO));
	if (waveinfo == NULL) {
		debug_error("error: mmfile_malloc\n");
		goto exception;
	}

	ret = mmf_file_wave_get_info (header, waveinfo);
	if (ret == -1) {
		debug_error("error: mmf_file_wave_get_info\n");
		goto exception;
	}

	mmfile_free (header);

	 /* Get file size. because sometimes waveinfo->size is wrong */
	ret = mmfile_open (&fp, formatContext->uriFileName, MMFILE_RDONLY);
	if(fp) {
		mmfile_seek (fp, 0, MMFILE_SEEK_END);
		filesize = mmfile_tell(fp);
		mmfile_seek (fp, 0, MMFILE_SEEK_SET);
		mmfile_close (fp);
	}

	formatContext->privateFormatData = waveinfo;

	if(waveinfo->size > filesize) {
		/*Wrong information*/
		formatContext->duration = (int)((((float)filesize - MMF_FILE_WAVE_HEADER_LEN) / (float)(waveinfo->byte_rate)) * 1000.0F);
	} else {
		formatContext->duration = (int)(((float)(waveinfo->size) / (float)(waveinfo->byte_rate)) * 1000.0F);
	}

	formatContext->audioTotalTrackNum = 1;
	formatContext->nbStreams = 1;
	formatContext->streams[MMFILE_AUDIO_STREAM] = mmfile_malloc (sizeof(MMFileFormatStream));

	if (!formatContext->streams[MMFILE_AUDIO_STREAM]) {
		debug_error("error: mmfile_malloc audio stream for wav\n");
		return MMFILE_FORMAT_FAIL;        
	}

	formatContext->streams[MMFILE_AUDIO_STREAM]->streamType = MMFILE_AUDIO_STREAM;

	switch (waveinfo->format) {
		case 0x0001:
			formatContext->streams[MMFILE_AUDIO_STREAM]->codecId = MM_AUDIO_CODEC_PCM;
			break;
		case 0x0002:
			formatContext->streams[MMFILE_AUDIO_STREAM]->codecId = MM_AUDIO_CODEC_MS_ADPCM;
			break;
		case 0x0006:
			formatContext->streams[MMFILE_AUDIO_STREAM]->codecId = MM_AUDIO_CODEC_ALAW;
			break;
		case 0x0007:
			formatContext->streams[MMFILE_AUDIO_STREAM]->codecId = MM_AUDIO_CODEC_MULAW;
			break;
		case 0x0011:
			formatContext->streams[MMFILE_AUDIO_STREAM]->codecId = MM_AUDIO_CODEC_ADPCM;
			break;
		default:
			formatContext->streams[MMFILE_AUDIO_STREAM]->codecId = MM_AUDIO_CODEC_INVALID;
			break;
	}

	formatContext->streams[MMFILE_AUDIO_STREAM]->bitRate = waveinfo->byte_rate*8;
	formatContext->streams[MMFILE_AUDIO_STREAM]->nbChannel = waveinfo->channel;
	formatContext->streams[MMFILE_AUDIO_STREAM]->framePerSec = 0;
	formatContext->streams[MMFILE_AUDIO_STREAM]->samplePerSec = waveinfo->sample_rate;
	formatContext->streams[MMFILE_AUDIO_STREAM]->bitPerSample = waveinfo->bits_per_sample;
	formatContext->streams[MMFILE_AUDIO_STREAM]->is_uhqa = __check_uhqa(waveinfo->sample_rate, waveinfo->bits_per_sample);

	return MMFILE_FORMAT_SUCCESS;

exception:
	mmfile_free (header);
	mmfile_free (waveinfo);

	return MMFILE_FORMAT_FAIL;
}


EXPORT_API
int mmfile_format_read_frame_wav (MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame)
{
	return MMFILE_FORMAT_SUCCESS;
}


EXPORT_API
int mmfile_format_read_tag_wav (MMFileFormatContext *formatContext)
{
	return MMFILE_FORMAT_SUCCESS;
}


EXPORT_API
int mmfile_format_close_wav (MMFileFormatContext *formatContext)
{
	if (formatContext == NULL) {
		debug_error("formatContext is NULL\n");
		return MMFILE_FORMAT_FAIL;
	}

	if (formatContext->privateFormatData)
		mmfile_free (formatContext->privateFormatData);

	return MMFILE_FORMAT_SUCCESS;
}

static char*
_dump_codec_name (short codec)
{
#ifdef __MMFILE_TEST_MODE__
	int sz = sizeof (g_audio_cdc_tbl) / sizeof (MMF_FILE_WAVE_CODEC_NAME);
	int i;
	
	for (i = 0; i < sz; i++) {
		if (g_audio_cdc_tbl[i].codec == codec) {
			return g_audio_cdc_tbl[i].name;
		}
	}
#endif
	return NULL;
}

static int _get_fmt_subchunk_offset (MMFileIOHandle *fp, long long limit, long long *offset)
{
	long long fmt_offset;
	int readed;
	int i;
	unsigned char buf[4];

	fmt_offset = mmfile_tell (fp);
	if (fmt_offset < 0)
		return 0;

	for (i = 0; i < limit; i++) {
		mmfile_seek (fp, fmt_offset + i, MMFILE_SEEK_SET);
		readed = mmfile_read (fp, buf, 4);
		if (readed != 4) {
			debug_error ( "failed to read. size = %d\n", readed);
			return 0;
		}
		
		if (buf[0] == 'f' && buf[1] == 'm' && buf[2] == 't' && buf[3] == ' ') {
			*offset = fmt_offset + i;
			return 1;
		}
	}
	
	return 0;
}

static unsigned char *
mmf_file_wave_get_header (char *src)
{
	int				readed = 0;
	MMFileIOHandle	*fp = NULL;
	int				ret = 0;
	long long		src_size = 0L;
	unsigned char	*header = NULL;
	long long		offset = 0;
	long long		limit;

	header = mmfile_malloc (MMF_FILE_WAVE_HEADER_LEN);
	if (!header)
		return NULL;

	/*open*/
	ret = mmfile_open (&fp, src, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error ( "open failed.\n");
		goto failed;
	}


	/*get file size*/
	mmfile_seek (fp, 0L, MMFILE_SEEK_END);
	src_size = mmfile_tell (fp);
	mmfile_seek (fp, 0L, MMFILE_SEEK_SET);

	if (src_size < MMF_FILE_WAVE_HEADER_LEN) {
		debug_error ( "header is too small.\n");
		goto failed;
	}

	/*read chunk data*/
	readed = mmfile_read (fp, header, MMF_FILE_WAVE_CHUNK_LEN);
	if (readed != MMF_FILE_WAVE_CHUNK_LEN) {
		debug_error ( "read error. size = %d\n", readed);
		goto failed;
	}

	/*seach 'fmt ' sub chunk*/
	limit = (src_size - MMF_FILE_WAVE_HEADER_LEN > 10240 ? 10240 : src_size - MMF_FILE_WAVE_HEADER_LEN);
	ret = _get_fmt_subchunk_offset (fp, limit, &offset);
	if (ret == 0) {
		debug_error ( "failed to seach 'fmt ' chunk\n");
		goto failed;
	}

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ("fmt offset: %lld\n", offset);
	#endif
	
	mmfile_seek (fp, offset, MMFILE_SEEK_SET);
	
	/*read sub chunk data*/
	readed = mmfile_read (fp, header + MMF_FILE_WAVE_CHUNK_LEN, MMF_FILE_WAVE_SUBCHUNK_LEN);
	if (readed != MMF_FILE_WAVE_SUBCHUNK_LEN) {
		debug_error ( "read error. size = %d\n", readed);
		goto failed;
	}

	mmfile_close (fp);

	return header;

failed:
	if (header) mmfile_free (header);
	if (fp) mmfile_close (fp);

	return NULL;
}

static int
mmf_file_wave_get_info (unsigned char *header, MM_FILE_WAVE_INFO *info)
{
	if (!header || !info) {
		return -1;
	}

	/*get chunk size*/
	info->size = *((int*)(header + 4));
	
	/*get format*/
	info->format = *((short*)(header + 20));

	/*get channel*/
	info->channel = *((short*)(header + 22));

	/*get sampling-rate*/
	info->sample_rate = *((int*)(header + 24));

	/*get byte rate*/
	info->byte_rate = *((int*)(header + 28));

	/*get byte align*/
	info->block_align = *((short*)(header + 32));
	
	/*get bits per sample*/
	info->bits_per_sample = *((short*)(header + 34));

	info->size				= mmfile_io_le_int32 (info->size);
	info->format			= mmfile_io_le_int16 (info->format);
	info->channel			= mmfile_io_le_int16 (info->channel);
	info->sample_rate		= mmfile_io_le_int32 (info->sample_rate);
	info->byte_rate			= mmfile_io_le_int32 (info->byte_rate);
	info->block_align		= mmfile_io_le_int16 (info->block_align);
	info->bits_per_sample	= mmfile_io_le_int16 (info->bits_per_sample);

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "----------------------------------------------\n");
	debug_msg ( "chunk size: %d\n", info->size);
	debug_msg ( "WAVE form Registration Number: 0x%04X\n", info->format);
	debug_msg ( "WAVE form wFormatTag ID: %s\n", _dump_codec_name (info->format));
	debug_msg ( "channel: %d\n", info->channel);
	debug_msg ( "sampling-rate: %d\n", info->sample_rate);
	debug_msg ( "byte-rate: %d\n", info->byte_rate);
	debug_msg ( "byte align: %d\n", info->block_align);
	debug_msg ( "bit per sample: %d\n", info->bits_per_sample);
	debug_msg ( "----------------------------------------------\n");
	#endif

	return 0;

}


