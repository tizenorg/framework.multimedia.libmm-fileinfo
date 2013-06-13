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

#include "mm_debug.h"

#include "mm_file_utils.h"
#include "mm_file_format_mmf.h"
#include "mm_file_format_private.h"


/**
 *MMF
 *
 */

#define	AV_MMF_CONTENTS_CLASS_0			0x00
#define	AV_MMF_CONTENTS_CLASS_1			0x20
#define	AV_MMF_CONTENTS_CLASS_2			0x10

#define	AV_MMF_CONTENTS_TYPE_0				0x00
#define	AV_MMF_CONTENTS_TYPE_1				0x10
#define	AV_MMF_CONTENTS_TYPE_2				0x20
#define	AV_MMF_CONTENTS_TYPE_3				0x30
#define	AV_MMF_CONTENTS_TYPE_4				0x40
#define	AV_MMF_CONTENTS_TYPE_5				0x50


#define AVMASMW_SUCCESS				(0)		/* success 								*/
#define AVMASMW_ERROR				(-1)	/* error								*/
#define AVMASMW_ERROR_ARGUMENT		(-2)	/* error of arguments					*/
#define AVMASMW_ERROR_RESOURCE_OVER	(-3)	/* over specified resources				*/
#define AVMASMW_ERROR_ID			(-4)	/* error id number 						*/
#define AVMASMW_ERROR_TIMEOUT		(-5)	/* timeout		 						*/
#define AVMASMW_ERROR_SOFTRESET		(-6)	/* error of soft reset for MA-5			*/

#define AVMASMW_ERROR_FILE				(-16)	/* file error							*/
#define AVMASMW_ERROR_CONTENTS_CLASS	(-17)	/* SAVMAF Contents Class shows can't play */
#define AVMASMW_ERROR_CONTENTS_TYPE		(-18)	/* SAVMAF Contents Type shows can't play	*/
#define AVMASMW_ERROR_CHUNK_SIZE		(-19)	/* illegal SMAF Chunk Size value		*/
#define AVMASMW_ERROR_CHUNK				(-20)	/* illegal SMAF Track Chunk value		*/
#define AVMASMW_ERROR_UNMATCHED_TAG		(-21)	/* unmathced specified TAG 				*/
#define AVMASMW_ERROR_SHORT_LENGTH		(-22)	/* short sequence 						*/
#define AVMASMW_ERROR_LONG_LENGTH		(-23)	/* long sequence 						*/
#define AVMASMW_ERROR_UNSUPPORTED		(-24)	/* unsupported format					*/
#define AVMASMW_ERROR_NO_INFORMATION	(-25)	/* no specified information				*/
#define AVMASMW_ERROR_HV_CONFLICT		(-26)	/* conflict about HV resource			*/

#define	AVMALIB_MAKEDWORD(a, b, c, d)	(unsigned int)(((unsigned int)(a) << 24) | \
						((unsigned int)(b) << 16) | ((unsigned int)(c) << 8) | (unsigned int)(d))

/****************************************************************************
 *	define
 ****************************************************************************/
#define	AVMALIB_SIZE_OF_CHUNKHEADER		(8)
#define	AVMALIB_SIZE_OF_CRC				(2)
#define	AVMALIB_SIZE_OF_MIN_CNTI		(5)
/* for Phrase */
#define	AVMALIB_CNTI_CLASS_YAMAHA		(0x00)
#define	AVMALIB_CNTI_TYPE_PHRASE		(0xF0)
#define	AVMALIB_PHRASE_TIMEBASE			(20)

/* Chunk ID		*/
#define	AVMALIB_CHUNKID_MMMD			0x4D4D4D44
#define	AVMALIB_CHUNKID_CNTI			0x434E5449
#define	AVMALIB_CHUNKID_OPDA			0x4F504441
#define	AVMALIB_CHUNKID_DCH				0x44636800
#define	AVMALIB_CHUNKID_M5P				0x50726F00
#define	AVMALIB_CHUNKID_MTR				0x4D545200
#define	AVMALIB_CHUNKID_MSPI			0x4D737049
#define	AVMALIB_CHUNKID_MTSU			0x4D747375
#define	AVMALIB_CHUNKID_MTSQ			0x4D747371
#define	AVMALIB_CHUNKID_MTSP			0x4D747370
#define	AVMALIB_CHUNKID_MWA				0x4D776100
#define	AVMALIB_CHUNKID_ATR				0x41545200
#define	AVMALIB_CHUNKID_ASPI			0x41737049
#define	AVMALIB_CHUNKID_ATSU			0x41747375
#define	AVMALIB_CHUNKID_ATSQ			0x41747371
#define	AVMALIB_CHUNKID_AWA				0x41776100
#define	AVMALIB_CHUNKID_MTHV			0x4D746876
#define	AVMALIB_CHUNKID_MHVS			0x4D687673
#define	AVMALIB_CHUNKID_HVP				0x48565000
#define	AVMALIB_CHUNKID_MHSC			0x4D687363

/****************************************************************************
 *	table
 ****************************************************************************/
static const unsigned short g_crc_tbl[256] = {
  0x0000U,0x1021U,0x2042U,0x3063U,0x4084U,0x50A5U,0x60C6U,0x70E7U,
  0x8108U,0x9129U,0xA14AU,0xB16BU,0xC18CU,0xD1ADU,0xE1CEU,0xF1EFU,
  0x1231U,0x0210U,0x3273U,0x2252U,0x52B5U,0x4294U,0x72F7U,0x62D6U,
  0x9339U,0x8318U,0xB37BU,0xA35AU,0xD3BDU,0xC39CU,0xF3FFU,0xE3DEU,
  0x2462U,0x3443U,0x0420U,0x1401U,0x64E6U,0x74C7U,0x44A4U,0x5485U,
  0xA56AU,0xB54BU,0x8528U,0x9509U,0xE5EEU,0xF5CFU,0xC5ACU,0xD58DU,
  0x3653U,0x2672U,0x1611U,0x0630U,0x76D7U,0x66F6U,0x5695U,0x46B4U,
  0xB75BU,0xA77AU,0x9719U,0x8738U,0xF7DFU,0xE7FEU,0xD79DU,0xC7BCU,
  0x48C4U,0x58E5U,0x6886U,0x78A7U,0x0840U,0x1861U,0x2802U,0x3823U,
  0xC9CCU,0xD9EDU,0xE98EU,0xF9AFU,0x8948U,0x9969U,0xA90AU,0xB92BU,
  0x5AF5U,0x4AD4U,0x7AB7U,0x6A96U,0x1A71U,0x0A50U,0x3A33U,0x2A12U,
  0xDBFDU,0xCBDCU,0xFBBFU,0xEB9EU,0x9B79U,0x8B58U,0xBB3BU,0xAB1AU,
  0x6CA6U,0x7C87U,0x4CE4U,0x5CC5U,0x2C22U,0x3C03U,0x0C60U,0x1C41U,
  0xEDAEU,0xFD8FU,0xCDECU,0xDDCDU,0xAD2AU,0xBD0BU,0x8D68U,0x9D49U,
  0x7E97U,0x6EB6U,0x5ED5U,0x4EF4U,0x3E13U,0x2E32U,0x1E51U,0x0E70U,
  0xFF9FU,0xEFBEU,0xDFDDU,0xCFFCU,0xBF1BU,0xAF3AU,0x9F59U,0x8F78U,
  0x9188U,0x81A9U,0xB1CAU,0xA1EBU,0xD10CU,0xC12DU,0xF14EU,0xE16FU,
  0x1080U,0x00A1U,0x30C2U,0x20E3U,0x5004U,0x4025U,0x7046U,0x6067U,
  0x83B9U,0x9398U,0xA3FBU,0xB3DAU,0xC33DU,0xD31CU,0xE37FU,0xF35EU,
  0x02B1U,0x1290U,0x22F3U,0x32D2U,0x4235U,0x5214U,0x6277U,0x7256U,
  0xB5EAU,0xA5CBU,0x95A8U,0x8589U,0xF56EU,0xE54FU,0xD52CU,0xC50DU,
  0x34E2U,0x24C3U,0x14A0U,0x0481U,0x7466U,0x6447U,0x5424U,0x4405U,
  0xA7DBU,0xB7FAU,0x8799U,0x97B8U,0xE75FU,0xF77EU,0xC71DU,0xD73CU,
  0x26D3U,0x36F2U,0x0691U,0x16B0U,0x6657U,0x7676U,0x4615U,0x5634U,
  0xD94CU,0xC96DU,0xF90EU,0xE92FU,0x99C8U,0x89E9U,0xB98AU,0xA9ABU,
  0x5844U,0x4865U,0x7806U,0x6827U,0x18C0U,0x08E1U,0x3882U,0x28A3U,
  0xCB7DU,0xDB5CU,0xEB3FU,0xFB1EU,0x8BF9U,0x9BD8U,0xABBBU,0xBB9AU,
  0x4A75U,0x5A54U,0x6A37U,0x7A16U,0x0AF1U,0x1AD0U,0x2AB3U,0x3A92U,
  0xFD2EU,0xED0FU,0xDD6CU,0xCD4DU,0xBDAAU,0xAD8BU,0x9DE8U,0x8DC9U,
  0x7C26U,0x6C07U,0x5C64U,0x4C45U,0x3CA2U,0x2C83U,0x1CE0U,0x0CC1U,
  0xEF1FU,0xFF3EU,0xCF5DU,0xDF7CU,0xAF9BU,0xBFBAU,0x8FD9U,0x9FF8U,
  0x6E17U,0x7E36U,0x4E55U,0x5E74U,0x2E93U,0x3EB2U,0x0ED1U,0x1EF0U
};

/*	phase	*/
#define	AVMALIB_CHUNK_PHASE_MMMD		0
#define	AVMALIB_CHUNK_PHASE_CNTI		1
#define	AVMALIB_CHUNK_PHASE_MMMDSUB		2
#define	AVMALIB_CHUNK_PHASE_MTRSUB		3
#define	AVMALIB_CHUNK_PHASE_ATRSUB		4
#define	AVMALIB_CHUNK_PHASE_OPDASUB		5
#define	AVMALIB_CHUNK_PHASE_MTSPSUB		6
#define	AVMALIB_CHUNK_PHASE_MTHVSUB		7

/*	return value	*/
#define	AVMALIB_CHUNKCODE_MMMD			0x00
#define	AVMALIB_CHUNKCODE_CNTI			0x01
#define	AVMALIB_CHUNKCODE_OPDA			0x02
#define	AVMALIB_CHUNKCODE_DCH			0x20
#define	AVMALIB_CHUNKCODE_M5P			0x21
#define	AVMALIB_CHUNKCODE_MTR			0x03
#define	AVMALIB_CHUNKCODE_MSPI			0x30
#define	AVMALIB_CHUNKCODE_MTSU			0x31
#define	AVMALIB_CHUNKCODE_MTSQ			0x32
#define	AVMALIB_CHUNKCODE_MTSP			0x33
#define	AVMALIB_CHUNKCODE_MWA			0x3F
#define	AVMALIB_CHUNKCODE_ATR			0x04
#define	AVMALIB_CHUNKCODE_ASPI			0x40
#define	AVMALIB_CHUNKCODE_ATSU			0x41
#define	AVMALIB_CHUNKCODE_ATSQ			0x42
#define	AVMALIB_CHUNKCODE_AWA			0x43

#define	AVMALIB_CHUNKCODE_MTHV			0x35
#define	AVMALIB_CHUNKCODE_MHVS			0x36
#define	AVMALIB_CHUNKCODE_HVP			0x37
#define	AVMALIB_CHUNKCODE_MHSC			0x38

#define	AVMALIB_CHUNKCODE_UNKNOWN		0xFF

#define	AVMALIB_CHUNK_ID_ERROR			-1
#define	AVMALIB_CHUNK_SIZE_ERROR		-2

#define	AV_MMF_MAX_TRACK_NUM			8
#define	AV_MMF_MAX_PHRASE_INFO			7
#define	AV_MMF_MAX_EVENT_NUM			10
#define	AV_MMF_MAX_NOTE_OFF_NUM			256
#define	AV_MMF_MAX_STREAM_DATA_NUM2		62
#define	AV_MMF_MAX_STREAM_DATA_NUM3		32
#define	AV_MMF_MAX_VOICE_DATA_NUM2		16
#define	AV_MMF_MAX_WAVE_DATA_NUM3		127
#define	AV_MMF_CHANNEL_NUM				16

#define	AV_MMF_FUNC_SUCCESS				AVMASMW_SUCCESS
#define	AV_MMF_FUNC_ERROR				AVMASMW_ERROR

#define	AV_MMF_ERR_ARGUMENT				AVMASMW_ERROR_ARGUMENT
#define	AV_MMF_ERR_FILE					AVMASMW_ERROR_FILE
#define	AV_MMF_ERR_CLASS				AVMASMW_ERROR_CONTENTS_CLASS
#define	AV_MMF_ERR_TYPE					AVMASMW_ERROR_CONTENTS_TYPE
#define	AV_MMF_ERR_SIZE					AVMASMW_ERROR_CHUNK_SIZE
#define	AV_MMF_ERR_CHUNK				AVMASMW_ERROR_CHUNK
#define	AV_MMF_ERR_NOTAG				AVMASMW_ERROR_UNMATCHED_TAG
#define	AV_MMF_ERR_SLENGTH				AVMASMW_ERROR_SHORT_LENGTH
#define	AV_MMF_ERR_LLENGTH				AVMASMW_ERROR_LONG_LENGTH
#define	AV_MMF_ERR_NODATA				AVMASMW_ERROR_NO_INFORMATION

#define	AV_MMF_SEQ_ID_NULL				-1

#define	AV_MMF_STATUS_IDLE				0
#define	AV_MMF_STATUS_SAT_PROFILE		1
#define	AV_MMF_STATUS_LOADED			2

#define	AV_MMF_CRC_NULL					0xFFFF0000
#define	AV_MMF_POSITION_OF_CCLASS		16
#define	AV_MMF_POSITION_OF_CTYPE		17
#define	AV_MMF_STSP_OFFSET_NULL			0xFFFFFFFF
#define	AV_MMF_STSP_TIME_NULL			0xFFFFFFFF
#define	AV_MMF_CHUNK_HEADER_SIZE		8
#define	AV_MMF_FILE_CRC_SIZE			2
#define	AV_MMF_ATR_TRACK_NO				AV_MMF_MAX_TRACK_NUM - 1
#define	AV_MMF_MINIMUM_TRACKSIZE2		6
#define	AV_MMF_MINIMUM_TRACKSIZE3		20
#define	AV_MMF_PLAY_TIME_MIN			20
#define	AV_MMF_PLAY_TIME_MAX			0x00200000
#define	AV_MMF_MA2_VOICE_NOTFOUND		0
#define	AV_MMF_MA2_VOICE_FOUND			1
#define	AV_MMF_FM_VOICE_MODE_4OP		0
#define	AV_MMF_FM_VOICE_MODE_2OP		1

#define	AV_MMF_STREAM_ID_INI			0xFF
#define	AV_MMF_STREAM_ID_REGIST			0x00
#define	AV_MMF_MA2_VOICE_NULL			0xFF
#define	AV_MMF_MA3_WAVE_NULL			0xFF



#define	AV_MMF_HUFFMAN_TREE_FAILURE		0
#define	AV_MMF_HUFFMAN_TREE_SUCCESS		1

#define	AV_MMF_PHRAZE_SIZE_A			8
#define	AV_MMF_PHRAZE_SIZE_B			12

#define	AV_MMF_TAG_STARTPOINT			0x7374
#define	AV_MMF_TAG_STOPPOINT			0x7370
#define	AV_MMF_TAG_PHRASE_A				0x5041
#define	AV_MMF_TAG_PHRASE_B				0x5042
#define	AV_MMF_TAG_PHRASE_E				0x5045
#define	AV_MMF_TAG_PHRASE_I				0x5049
#define	AV_MMF_TAG_PHRASE_K				0x504B
#define	AV_MMF_TAG_PHRASE_R				0x5052
#define	AV_MMF_TAG_PHRASE_S				0x5053

#define	AV_MMF_SMAF_TYPE_NULL			0
#define	AV_MMF_SMAF_TYPE_MA1			1
#define	AV_MMF_SMAF_TYPE_MA2			2
#define	AV_MMF_SMAF_TYPE_MA3			3
#define	AV_MMF_SMAF_TYPE_MA5			5

#define	AV_MMF_FM_MODE_2OP32			0
#define	AV_MMF_FM_MODE_4OP16			1
#define	AV_MMF_P_SOUNDSET_GMX			0
#define	AV_MMF_P_SOUNDSET_GML1			2
#define	AV_MMF_WT_VOLUME_MA3			0
#define	AV_MMF_WT_VOLUME_MA5			4

#define	AV_MMF_RESOUCE_MODE_MA3			2
#define	AV_MMF_RESOUCE_MODE_MA12		3
#define	AV_MMF_RESOUCE_MODE_MA5			4
#define	AV_MMF_RESOUCE_MODE_MA5_64		5

#define	AV_MMF_SEQUENCETYPE_DERAYED	0
#define	AV_MMF_AL_CHANNEL_NULL			0xFF
#define	AV_MMF_HV_CHANNEL_NULL			16


#define	AV_MMF_BANK_NUMBER_DEF			0
#define	AV_MMF_PROGRAM_NUMBER_DEF		0
#define	AV_MMF_RESONANCE_DEF			0x40
#define	AV_MMF_BRIGHTNESS_DEF			0x40

#define	AV_MMF_SETVOLUME_3RD_PARAM		0x07

#define	AV_MMF_LED_SEQ_SYNC_OFF			0
#define	AV_MMF_LED_SEQ_SYNC_ON			1
#define	AV_MMF_VIB_SEQ_SYNC_OFF			0
#define	AV_MMF_VIB_SEQ_SYNC_ON			1
#define	AV_MMF_KEYCONTROL_ON			2
#define	AV_MMF_KEYCONTROL_OFF			1
#define	AV_MMF_KEYCONTROL_DEFAULT		0

#define	AV_MMF_EVNET_EOS				0xFF

#define	AV_MMF_CHANNEL_MODE_POLY		0
#define	AV_MMF_CHANNEL_MODE_MONO		1

#define	AV_MMF_BANK_NUMBER_DEF			0
#define	AV_MMF_PROGRAM_NUMBER_DEF		0
#define	AV_MMF_RPN_DEF					0x7F
#define	AV_MMF_MODULATION_DEF			0
#define	AV_MMF_CHANNEL_VOLUME_DEF		100
#define	AV_MMF_CHANNEL_PAN_DEF			0x40
#define	AV_MMF_EXPRESSION_DEF			0x7F
#define	AV_MMF_HOLD_DEF					0
#define	AV_MMF_MONO_POLY_MODE_DEF		AV_MMF_CHANNEL_MODE_POLY
#define	AV_MMF_PITCH_BEND_DEF			0x40
#define	AV_MMF_BEND_RANGE_DEF			2
#define	AV_MMF_OCTAVE_SHIFT_DEF			4
#define	AV_MMF_VELOCITY_DEF_MA3			90
#define	AV_MMF_VELOCITY_DEF_MA5			100
#define	AV_MMF_BLOCK_FNUM_DEF			0
#define	AV_MMF_STREAM_PAIR_DEF			0xFF
#define	AV_MMF_STREAM_PAN_DEF			0xFF
#define	AV_MMF_STREAM_PAN_OFF			0x80

#define	SNDDRV_DEF_STREAM_PAN			255
#define	SNDDRV_DEF_RESONANCE			64
#define	SNDDRV_DEF_BRIGHTNESS			64
#define	SNDDRV_DEF_CHANNEL_VOLUME		100
#define	SNDDRV_DEF_CHANNEL_PANPOT		64
#define	SNDDRV_DEF_EXPRESSION			127
#define	SNDDRV_DEF_BENDRANGE			2
#define	SNDDRV_DEF_PITCHBEND			0x2000

#define	AV_MMF_CONVERT_PHASE_PLAY		0x00000000
#define	AV_MMF_CONVERT_PHASE_SEEK_G		0x00010000
#define AV_MMF_CONVERT_PHASE_SEEK_C		0x00020000
#define AV_MMF_CONVERT_PHASE_SEEK_END	0x00030000
#define AV_MMF_CONVERT_PHASE_PLAY_END	0x00FF0000

#define	AV_MMF_SEEK_EVENT_END			0xFFFF

/****************************************************************************
 *	definition of resources
 ****************************************************************************/
#define	AVMAPLIB_MAX_LV2_VOICES			(16)

/****************************************************************************
 *	typedef
 ****************************************************************************/

/* OPDA infomation structure	*/
typedef struct AvTagOptionInfo
{
	unsigned char	*pbCnti;				/* pointer to CNTI Body				*/
	unsigned int	dCntiSize;				/* size of CNTI Body				*/
	unsigned char	*pbOpda;				/* pointer to OPDA Body				*/
	unsigned int	dOpdaSize;				/* size of OPDA Body				*/
} OPTIONINFO, *POPTIONINFO;

/* Track information structure	*/
typedef struct AvTagTrackInfo
{
	unsigned char	*pbMtr;					/* pointer to MTR(ATR) Body			*/
	unsigned int	dMtrSize;				/* size of MTR(ATR) Body			*/
	unsigned char	*pbMspi;				/* pointer to MspI(AspI) Body		*/
	unsigned int	dMspiSize;				/* size of MspI(AspI) Body			*/
	unsigned char	*pbMtsu;				/* pointer to Mtsu Body				*/
	unsigned int	dMtsuSize;				/* size of Mtsu Body				*/
	unsigned char	*pbMtsq;				/* pointer to Mtsq(Atsq) Body		*/
	unsigned int	dMtsqSize;				/* size of Mtsq(Atsq) Body			*/
	unsigned char	*pbMtsp;				/* pointer to Mtsp Body				*/
	unsigned int	dMtspSize;				/* size of Mtsp Body				*/
	unsigned char	*pbMthv;				/* pointer to Mthv Body				*/
	unsigned int	dMthvSize;				/* size of Mthv Body				*/
	unsigned int	dPlayTime;				/* play time (tick)					*/
	unsigned int	dTimeBase;				/* time base (msec/tick)			*/
	unsigned int	dStartPoint;			/* start point(offset)				*/
	unsigned int	dStopPoint;				/* stop point(offset)				*/
	unsigned int	dStartTick;				/* start point(tick)				*/
	unsigned int	dStopTick;				/* stop point(tick)					*/
} TRACKINFO, *PTRACKINFO;

/* Phrase information structure	*/
typedef struct AvTagPhraseInfo
{
	unsigned int	dStartPoint;			/* start point of phrase(offset)	*/
	unsigned int	dStopPoint;				/* stop point of phrase(offset)		*/
	unsigned int	dStartTick;				/* start point of phrase(tick)		*/
	unsigned int	dStopTick;				/* stop point of phrase(tick)		*/
} PHRASEINFO, *PPHRASEINFO;

/* Huffman information structure	*/
typedef struct AvTagHuffmanInfo
{
	unsigned int	dMtsqSize;				/* size of Mtsq(Atsq) Body			*/
	unsigned int	dSeqSize;				/* size of sequence data			*/
	unsigned int	dReadSize;				/* read data size 					*/
	short			swLeft[512];			/* Huffman Tree (Left)				*/
	short			swRight[512];			/* Huffman Tree (Right)				*/
	unsigned char	*psBuffer;				/* pointer to reference area		*/
	char			sbBitC;					/* counter of reference bit			*/
	unsigned char	bByte;					/* value of reference byte			*/
	unsigned char	*psFBuf;				/* pointer to sequence data top		*/
	char			sbFBit;					/* counter of sequence data top bit	*/
	unsigned char	bFByte	;				/* value of sequence data top byte	*/
} HUFFMANINFO, *PHUFFMANINFO;

/* HV information structure		*/
typedef struct AvTagHvInfo
{
	unsigned char	*pbVoice;				/* pointer to HVP0 chunk header		*/
	unsigned int	dVoiceSize;				/* size of HV voice parameter		*/
	unsigned char	*pbScript;				/* pointer to Mhsc body				*/
	unsigned int	dScriptSize;			/* size of Mhsc body				*/
	unsigned char	bHvChannel;				/* HV channel #						*/
} HVINFO, *PHVINFO;


/* Load information structure	*/
typedef struct AvTagLoadInfo
{
	unsigned char	*pbMmmd;				/* pointer to MMMD top				*/
	unsigned int	dMmmdSize;				/* size of MMMD (whole)				*/
	unsigned int	dCrc;					/* file CRC							*/
	unsigned int	dSmafType;				/* SMAF Type						*/
	unsigned int	dPlayTime;				/* play time (tick)					*/
	unsigned int	dStartTime;				/* start time (start point tick)	*/
	unsigned int	dTimeBase;				/* time base (msec/tick)			*/
	unsigned char	(*pfnGetByte)(PHUFFMANINFO psHuf);
	OPTIONINFO		sOption_Info;
	TRACKINFO		sTrack_Info[AV_MMF_MAX_TRACK_NUM];
	PHRASEINFO		sPhrase_Info[AV_MMF_MAX_PHRASE_INFO];
	HUFFMANINFO		sHuffman_Info;
	HVINFO			sHV_Info;
} LOADINFO, *PLOADINFO;

/* Stream information structure(for MA-2)	*/
typedef struct AvTagStreamInfo2
{
	unsigned char	bStrmID;				/* key number of stream				*/
	unsigned char	*pbWave;				/* pointer to Awa body				*/
	unsigned int	dWaveSize;				/* size of Awa body					*/
	unsigned int	dFs;					/* sampling frequency				*/
} STREAMINFO2, *PSTREAMINFO2;

/* Stream information structure(for MA-3/5)	*/
typedef struct AvTagStreamInfo3
{
	unsigned char	fbNote;					/* stream data flag					*/
	unsigned char	bPairID;				/* stream pair ID					*/
	unsigned char	bPan;					/* stream pan						*/
} STREAMINFO3, *PSTREAMINFO3;

/* Stream information structure	*/
typedef struct AvTagStreamInfo
{
	STREAMINFO2		sStream_Info2[AV_MMF_MAX_STREAM_DATA_NUM2];
	STREAMINFO3		sStream_Info3[AV_MMF_MAX_STREAM_DATA_NUM3];
} STREAMINFO, *PSTREAMINFO;

/* Voice information structure(for MA-1/2)	*/
typedef struct AvTagVoiceInfo2
{
	unsigned char	bBank;					/* bank number						*/
	unsigned char	bProg;					/* program number					*/
} VOICEINFO2, *PVOICEINFO2;

/* Wave information structure(for MA-3/5)	*/
typedef struct AvTagWaveInfo3
{
	unsigned int	dAdrs;					/* wave address						*/
	unsigned int	dSize;					/* wave data size					*/
} WAVEINFO3, *PWAVEINFO3;

/* Voice information structure	*/
typedef struct AvTagVoiceInfo
{
	VOICEINFO2		sVoice_Info2[AV_MMF_MAX_VOICE_DATA_NUM2];
	WAVEINFO3		sWave_Info3[AV_MMF_MAX_WAVE_DATA_NUM3];
} VOICEINFO, *PVOICEINFO;

/* Channel information structure	*/
typedef struct AvTagChannelInfo
{
	unsigned char	bBankM;					/* bank select MSB					*/
	unsigned char	bBankL;					/* bank select LSB					*/
	unsigned char	bBank;					/* bank number (sound driver)		*/
	unsigned char	bProg;					/* program change					*/
	unsigned char	bRpnM;					/* RPN MSB							*/
	unsigned char	bRpnL;					/* RPN LSB							*/
	unsigned char	bMod;					/* modulation						*/
	unsigned char	bChVol;					/* channel volume					*/
	unsigned char	bPan;					/* channel pan						*/
	unsigned char	bExp;					/* expression						*/
	unsigned char	bHold;					/* hold								*/
	unsigned char	bMono;					/* channel mode mono/poly			*/
	unsigned char	bPitch;					/* pitch bend (MSB)					*/
	unsigned char	bSens1;					/* pitch bend lenge 1				*/
	unsigned char	bSens2;					/* pitch bend lenge 2				*/
	unsigned char	bOct;					/* octerve shift					*/
	unsigned char	bVel;					/* note on velocity					*/
	unsigned char	bBlockFnum1;			/* ma-2 pitch bend (0xB0)			*/
	unsigned char	bBlockFnum2;			/* ma-2 pitch bend (0xC0)			*/
	unsigned char	fbLed;					/* LED synchronous flag				*/
	unsigned char	fbVib;					/* Motor synchronous flag			*/
} CHANNELINFO, *PCHANNELINFO;

/* Event information structure	*/
typedef struct AvTagEventBlock
{
	unsigned int	dEventTime;				/* event activation time			*/
	unsigned int	dSeqID;					/* sequencer ID (sound driver)		*/
	unsigned int	dCmd;					/* command ID (sound driver)		*/
	unsigned int	dParam1;				/* parameter 1						*/
	unsigned int	dParam2;				/* parameter 2						*/
	unsigned int	dParam3;				/* parameter 3						*/
	unsigned char	*pbSeq;					/* pointer to next event data		*/
	unsigned int	dIndex;					/* index of next event				*/
	unsigned int	dTrackNo;				/* track no.						*/
	void  *			pvNext;					/* pointer to next event block		*/
} EVENTBLOCK, *PEVENTBLOCK;

/* Note OFF information structure	*/
typedef struct AvTagNoteOffBlock
{
	unsigned int	dOffTime;				/* note off activation time			*/
	unsigned int	dSeqID;					/* sequencer ID (sound driver)		*/
	unsigned int	dCmd;					/* command ID (sound driver)		*/
	unsigned int	dCh;					/* channel no.						*/
	unsigned int	dKey;					/* key no.							*/
	void  *			pvNext;					/* pointer to next note off block	*/
} OFFBLOCK, *POFFBLOCK;

/* Playback information structure	*/
typedef struct AvTagPlayInfo
{
	PEVENTBLOCK		psNextEvent;			/* pointer to next event block		*/
	PEVENTBLOCK		psEmptyEvent;			/* pointer to empty event block		*/
	POFFBLOCK		psNextOff;				/* pointer to next note off block	*/
	POFFBLOCK		psEmptyOff;				/* pointer to empty note off block	*/
	unsigned int	dSmafType;				/* SMAF Type						*/
	unsigned int	dHWTimeBase;			/* tick to H/W time base			*/
	unsigned int	dPlayTime;				/* play time (tick)					*/
	unsigned int	dStartTime;				/* start time (start point tick)	*/
	unsigned int	dSeekTime;				/* seek time (msec)					*/
	unsigned int	dSeekError;				/* seek time error (msec)			*/
	unsigned int	dRamAdrs;				/* ram address						*/
	unsigned int	dRamSize;				/* ram size							*/
	unsigned int	dSeekParam;				/* seek event flag					*/
	unsigned int	dPastTime;				/* past time (tick)					*/
	unsigned char	bMasterVol;				/* master volume (sequence)			*/
	unsigned char	bEos;					/* EOS flag							*/
	unsigned short	fwMono;					/* channel mode change flag			*/
	unsigned char	bResonance;				/* resonance						*/
	unsigned char	bBrightness;			/* brightness						*/
	unsigned char	bStreamReserve;			/* stream reserve					*/
	unsigned char	bAlChReserve;			/* AL channel number				*/
	unsigned char	bHVChannel;				/* HV channel number				*/
	unsigned char	bFmVoice;				/* FM Voice Mode 0:4op, 1:2op		*/
	unsigned char	(*pfnGetByte)(void);	/* pointer to byte get function		*/
	int				(*pfnGetEvent)(void);	/* pointer to event get function	*/
	STREAMINFO		sStream_Info;
	VOICEINFO		sVoice_Info;
	CHANNELINFO		sChannel_Info[AV_MMF_CHANNEL_NUM + 1];
	EVENTBLOCK		sEvent_Block[AV_MMF_MAX_EVENT_NUM];
	OFFBLOCK		sNoteOff_Block[AV_MMF_MAX_NOTE_OFF_NUM];
} PLAYINFO, *PPLAYINFO;

/* SMAF information structure	*/
typedef struct AvTagSmafInfo
{
	int				sdMmfSeqID;				/* sequence id (sound driver)		*/
	unsigned int	dStatus;				/* converter status					*/
	LOADINFO		sLoad_Info[2];
	PLAYINFO		sPlay_Info;
} SMAFINFO, *PSMAFINFO;

static SMAFINFO g_sSmaf_Info;
static const unsigned char g_abBitMaskTable1[8] = {0x01,	0x02,	0x04,	0x08,	0x10,	0x20,	0x40,	0x80};
static const unsigned short g_awBitMaskTable2[8] = {0x00FF,	0x01FE, 0x03FC,	0x07F8,	0x0FF0,	0x1FE0,	0x3FC0,	0x7F80};
static const unsigned char g_abTableA[16] = {0, 1, 1, 1, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15};
static const unsigned char g_abTableD[16] = {0, 1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
static const unsigned char g_abExpression2[16] = {0x80,0x00,0x1f,0x27,0x2f,0x37,0x3f,0x47,0x4f,0x57,0x5f,0x67,0x6f,0x77,0x7f,0x80};
static const unsigned char g_abModulation2[16] = {0x80,0x00,0x08,0x10,0x18,0x20,0x28,0x30,0x38,0x40,0x48,0x50,0x60,0x70,0x7f,0x80};

/**
 * Define.
 */
#define MMF_FILE_MMF_HEADER_LEN			18

#define MMF_FILE_MMF_MAGIC_STR_LEN		4
#define MMF_FILE_MMF_MAGIC_STR			"MMMD"

#define MMF_FILE_MMF_TYPE_POSITION		((char)0x11)



/* internals */
static int mmf_file_mmf_get_duration (char *src, int is_xmf);



/* mm plugin porting */
/* plugin manadatory API */
int mmfile_format_read_stream_mmf (MMFileFormatContext *formatContext);
int mmfile_format_read_frame_mmf  (MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int mmfile_format_read_tag_mmf    (MMFileFormatContext *formatContext);
int mmfile_format_close_mmf       (MMFileFormatContext *formatContext);


EXPORT_API
int mmfile_format_open_mmf (MMFileFormatContext *formatContext)
{   
	int ret = 0;

	if (NULL == formatContext) {
		debug_error ("error: formatContext is NULL\n");
		return MMFILE_FORMAT_FAIL;        
	}

	if (formatContext->pre_checked == 0) {
		ret = MMFileFormatIsValidMMF (formatContext->uriFileName);
		if (ret == 0) {
			debug_error ("error: it is not MMF file\n");
			return MMFILE_FORMAT_FAIL;         
		}
	}

	formatContext->ReadStream   = mmfile_format_read_stream_mmf;
	formatContext->ReadFrame    = mmfile_format_read_frame_mmf;
	formatContext->ReadTag      = mmfile_format_read_tag_mmf;
	formatContext->Close        = mmfile_format_close_mmf;

	formatContext->videoTotalTrackNum = 0;
	formatContext->audioTotalTrackNum = 1;

	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_read_stream_mmf (MMFileFormatContext *formatContext)
{
    int total = 0;

    total = mmf_file_mmf_get_duration (formatContext->uriFileName, 0 /*not XMF*/);
    if ( total < 0 )
    {
        debug_error ("error: get duration\n");
        return MMFILE_FORMAT_FAIL;
    }


    formatContext->duration = total;
    formatContext->audioTotalTrackNum = 1;
    formatContext->nbStreams = 1;
    formatContext->streams[MMFILE_AUDIO_STREAM] = mmfile_malloc (sizeof(MMFileFormatStream));
    if (NULL == formatContext->streams[MMFILE_AUDIO_STREAM])
    {
        debug_error ("error: mmfile_malloc, audido stream\n");
        return MMFILE_FORMAT_FAIL;
    }

    formatContext->streams[MMFILE_AUDIO_STREAM]->codecId =MM_AUDIO_CODEC_MMF;
    formatContext->streams[MMFILE_AUDIO_STREAM]->nbChannel = 1;

    return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_read_frame_mmf  (MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame)
{
    return MMFILE_FORMAT_SUCCESS;
}


EXPORT_API
int mmfile_format_read_tag_mmf    (MMFileFormatContext *formatContext)
{
    return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_close_mmf (MMFileFormatContext *formatContext)
{
    return MMFILE_FORMAT_SUCCESS;
}


/*********************************************************************************
 *	_mmf_Get4Byte
 *
 *	Description:
 *			SMAF data load (error check and regist)
 *	Argument:
 *			pbBuf			pointer to top data
 *	Return:
 *			nothing
 ********************************************************************************/
static unsigned int
_mmf_Get4Byte(unsigned char*	pbBuf)
{
	return (unsigned int)(	(((unsigned int)pbBuf[0]) << 24) +
						(((unsigned int)pbBuf[1]) << 16) +
						(((unsigned int)pbBuf[2]) <<  8) +
						 ((unsigned int)pbBuf[3]));
}


/*********************************************************************************
 *	_mmf_CheckInitial
 *
 *	Description:
 *			SMAF data load (error check and regist)
 *	Argument:
 *			psLoad			pointer to load information structure
 *	Return:
 *			nothing
 ********************************************************************************/
static void
_mmf_CheckInitial(PLOADINFO psLoad)
{
	unsigned char				i;

/* Initialize Load information structure	*/
	psLoad->pbMmmd		= NULL;
	psLoad->dMmmdSize	= 0;
	psLoad->dCrc		= AV_MMF_CRC_NULL;
	psLoad->dSmafType	= AV_MMF_SMAF_TYPE_NULL;
	psLoad->dPlayTime	= 0;
	psLoad->dStartTime	= 0;
	psLoad->dTimeBase	= 0;
	psLoad->pfnGetByte	= NULL;

	psLoad->sOption_Info.pbCnti		= NULL;
	psLoad->sOption_Info.dCntiSize	= 0;
	psLoad->sOption_Info.pbOpda		= NULL;
	psLoad->sOption_Info.dOpdaSize	= 0;

	for ( i = 0; i < AV_MMF_MAX_TRACK_NUM; i++)
	{
		psLoad->sTrack_Info[i].pbMtr		= NULL;
		psLoad->sTrack_Info[i].dMtrSize		= 0;
		psLoad->sTrack_Info[i].pbMspi		= NULL;
		psLoad->sTrack_Info[i].dMspiSize	= 0;
		psLoad->sTrack_Info[i].pbMtsu		= NULL;
		psLoad->sTrack_Info[i].dMtsuSize	= 0;
		psLoad->sTrack_Info[i].pbMtsq		= NULL;
		psLoad->sTrack_Info[i].dMtsqSize	= 0;
		psLoad->sTrack_Info[i].pbMtsp		= NULL;
		psLoad->sTrack_Info[i].dMtspSize	= 0;
		psLoad->sTrack_Info[i].pbMthv		= NULL;
		psLoad->sTrack_Info[i].dMthvSize	= 0;
		psLoad->sTrack_Info[i].dPlayTime	= 0;
		psLoad->sTrack_Info[i].dTimeBase	= 0;
		psLoad->sTrack_Info[i].dStartPoint	= AV_MMF_STSP_OFFSET_NULL;
		psLoad->sTrack_Info[i].dStopPoint	= AV_MMF_STSP_OFFSET_NULL;
		psLoad->sTrack_Info[i].dStartTick	= AV_MMF_STSP_TIME_NULL;
		psLoad->sTrack_Info[i].dStopTick	= AV_MMF_STSP_TIME_NULL;
	}

	for ( i = 0; i < AV_MMF_MAX_PHRASE_INFO; i++)
	{
		psLoad->sPhrase_Info[i].dStartPoint	= AV_MMF_STSP_OFFSET_NULL;
		psLoad->sPhrase_Info[i].dStopPoint	= AV_MMF_STSP_OFFSET_NULL;
		psLoad->sPhrase_Info[i].dStartTick	= AV_MMF_STSP_TIME_NULL;
		psLoad->sPhrase_Info[i].dStopTick	= AV_MMF_STSP_TIME_NULL;
	}

	psLoad->sHV_Info.pbVoice		= NULL;
	psLoad->sHV_Info.dVoiceSize		= 0;
	psLoad->sHV_Info.pbScript		= NULL;
	psLoad->sHV_Info.dScriptSize	= 0;
	psLoad->sHV_Info.bHvChannel		= AV_MMF_HV_CHANNEL_NULL;
}


/*********************************************************************************
 *	_mmf_CheckInitial
 *
 *	Description:
 *			get time base (data value -> msec/tick)
 *	Argument:
 *			bData			time base (data value)
 *	Return:
 *			>=0				success(time base (msec/tick))
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_GetTimebase(unsigned char	bData)
{
	switch ( bData )
	{
		case 0x02:	return  4;						/*  4[msec/tick]			*/
		case 0x03:	return  5;						/*  5[msec/tick]			*/
		case 0x10:	return 10;						/* 10[msec/tick]			*/
		case 0x11:	return 20;						/* 20[msec/tick]			*/
		case 0x12:	return 40;						/* 40[msec/tick]			*/
		case 0x13:	return 50;						/* 50[msec/tick]			*/
		default:	return AV_MMF_FUNC_ERROR;			/* Time Base Error			*/
	}
}

static int
_mmf_MalibNextChunk(unsigned char* pbFile, unsigned int dSize, unsigned int dState, unsigned int*	pdChunkID, unsigned int* pdChunkNo)
{
	unsigned int				dChunkID, dChunkSize;

	if (dSize < AVMALIB_SIZE_OF_CHUNKHEADER)
		return AVMALIB_CHUNK_SIZE_ERROR;

	dChunkID	= AVMALIB_MAKEDWORD(pbFile[0], pbFile[1], pbFile[2], pbFile[3]);
	dChunkSize	= AVMALIB_MAKEDWORD(pbFile[4], pbFile[5], pbFile[6], pbFile[7]);

	switch (dChunkID)
	{
	case AVMALIB_CHUNKID_MMMD :
		*pdChunkID = AVMALIB_CHUNKCODE_MMMD;
		if (dState != AVMALIB_CHUNK_PHASE_MMMD)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_CNTI :
		*pdChunkID = AVMALIB_CHUNKCODE_CNTI;
		if (dState != AVMALIB_CHUNK_PHASE_CNTI)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_OPDA :
		*pdChunkID = AVMALIB_CHUNKCODE_OPDA;
		if (dState != AVMALIB_CHUNK_PHASE_MMMDSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_MSPI :
		*pdChunkID = AVMALIB_CHUNKCODE_MSPI;
		if (dState != AVMALIB_CHUNK_PHASE_MTRSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_MTSU :
		*pdChunkID = AVMALIB_CHUNKCODE_MTSU;
		if (dState != AVMALIB_CHUNK_PHASE_MTRSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_MTSQ :
		*pdChunkID = AVMALIB_CHUNKCODE_MTSQ;
		if (dState != AVMALIB_CHUNK_PHASE_MTRSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_MTSP :
		*pdChunkID = AVMALIB_CHUNKCODE_MTSP;
		if (dState != AVMALIB_CHUNK_PHASE_MTRSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_ASPI :
		*pdChunkID = AVMALIB_CHUNKCODE_ASPI;
		if (dState != AVMALIB_CHUNK_PHASE_ATRSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_ATSU :
		*pdChunkID = AVMALIB_CHUNKCODE_ATSU;
		if (dState != AVMALIB_CHUNK_PHASE_ATRSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_ATSQ :
		*pdChunkID = AVMALIB_CHUNKCODE_ATSQ;
		if (dState != AVMALIB_CHUNK_PHASE_ATRSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;

	case AVMALIB_CHUNKID_MTHV :
		*pdChunkID = AVMALIB_CHUNKCODE_MTHV;
		if (dState != AVMALIB_CHUNK_PHASE_MTRSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_MHVS :
		*pdChunkID = AVMALIB_CHUNKCODE_MHVS;
		if (dState != AVMALIB_CHUNK_PHASE_MTHVSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;
	case AVMALIB_CHUNKID_MHSC :
		*pdChunkID = AVMALIB_CHUNKCODE_MHSC;
		if (dState != AVMALIB_CHUNK_PHASE_MTHVSUB)
			return AVMALIB_CHUNK_ID_ERROR;
		break;

	default :
		*pdChunkNo = (unsigned char)(dChunkID & 0x000000FF);
		switch (dChunkID & 0xFFFFFF00)
		{
		case AVMALIB_CHUNKID_MTR :
			*pdChunkID = AVMALIB_CHUNKCODE_MTR;
			if (dState != AVMALIB_CHUNK_PHASE_MMMDSUB)
				return AVMALIB_CHUNK_ID_ERROR;
			break;
		case AVMALIB_CHUNKID_ATR :
			*pdChunkID = AVMALIB_CHUNKCODE_ATR;
			if (dState != AVMALIB_CHUNK_PHASE_MMMDSUB)
				return AVMALIB_CHUNK_ID_ERROR;
			break;

		case AVMALIB_CHUNKID_DCH :
			*pdChunkID = AVMALIB_CHUNKCODE_DCH;
			if (dState != AVMALIB_CHUNK_PHASE_OPDASUB)
				return AVMALIB_CHUNK_ID_ERROR;
			break;
		case AVMALIB_CHUNKID_M5P :
			*pdChunkID = AVMALIB_CHUNKCODE_M5P;
			if (dState != AVMALIB_CHUNK_PHASE_OPDASUB)
				return AVMALIB_CHUNK_ID_ERROR;
			break;

		case AVMALIB_CHUNKID_MWA :
			*pdChunkID = AVMALIB_CHUNKCODE_MWA;
			if (dState != AVMALIB_CHUNK_PHASE_MTSPSUB)
				return AVMALIB_CHUNK_ID_ERROR;
			break;

		case AVMALIB_CHUNKID_AWA :
			*pdChunkID = AVMALIB_CHUNKCODE_AWA;
			if (dState != AVMALIB_CHUNK_PHASE_ATRSUB)
				return AVMALIB_CHUNK_ID_ERROR;
			break;

		case AVMALIB_CHUNKID_HVP :
			*pdChunkID = AVMALIB_CHUNKCODE_HVP;
			if (dState != AVMALIB_CHUNK_PHASE_MTHVSUB)
				return AVMALIB_CHUNK_ID_ERROR;
			break;

		default :
			*pdChunkID = AVMALIB_CHUNKCODE_UNKNOWN;
			break;
		}
		break;
	}

	if (dChunkSize > (dSize - AVMALIB_SIZE_OF_CHUNKHEADER))
		return AVMALIB_CHUNK_SIZE_ERROR;
	else
		return (int)dChunkSize;
}



/*********************************************************************************
 *	_mmf_MTRCheck
 *
 *	Description:
 *			score track chunk check (header information and chunk construction)
 *	Argument:
 *			psTrack			pointer to track information structure
 *			bSmafType		SMAF type
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_MTRCheck(PTRACKINFO psTrack, unsigned char	bSmafType)
{
	int				sdResult, sdChunkSize;
	unsigned int				dSize, dIndex;
	unsigned char*				pbBuf;
	unsigned int				dChunkID, dChunkNo;

/* Check Format Type				*/
	switch (bSmafType)
	{
	case AV_MMF_SMAF_TYPE_MA1 :
	case AV_MMF_SMAF_TYPE_MA2 :
		if (psTrack->pbMtr[0] != 0x00)
		{
			return AV_MMF_ERR_CHUNK;
		}
		break;
	case AV_MMF_SMAF_TYPE_MA3 :
		if ((psTrack->pbMtr[0] != 0x01) && (psTrack->pbMtr[0] != 0x02))
		{
			return AV_MMF_ERR_CHUNK;
		}
		break;
	case AV_MMF_SMAF_TYPE_MA5 :
		if (psTrack->pbMtr[0] != 0x02)
		{
			return AV_MMF_ERR_CHUNK;
		}
		break;
	}

/* Check Sequence Type		*/
	if (psTrack->pbMtr[1] != 0x00)
	{
		return AV_MMF_ERR_CHUNK;
	}

/* Check Time Base		*/
	if (psTrack->pbMtr[2] != psTrack->pbMtr[3])
	{
		return AV_MMF_ERR_CHUNK;
	}
	sdResult = _mmf_GetTimebase(psTrack->pbMtr[2]);
	if (sdResult == AV_MMF_FUNC_ERROR)
	{
		return AV_MMF_ERR_CHUNK;
	}
	psTrack->dTimeBase = (unsigned int)sdResult;

/* Check sub chunk disposition	*/
	if ((bSmafType == AV_MMF_SMAF_TYPE_MA1) || (bSmafType == AV_MMF_SMAF_TYPE_MA2))
		dIndex = AV_MMF_MINIMUM_TRACKSIZE2;
	else
		dIndex = AV_MMF_MINIMUM_TRACKSIZE3;
	pbBuf = psTrack->pbMtr;
	dSize = psTrack->dMtrSize;
	while (dSize > (dIndex + AV_MMF_CHUNK_HEADER_SIZE))
	{
		sdChunkSize = _mmf_MalibNextChunk(&pbBuf[dIndex], (dSize-dIndex),
			AVMALIB_CHUNK_PHASE_MTRSUB, &dChunkID, &dChunkNo);
		if (sdChunkSize < 0)
		{
			return AV_MMF_ERR_CHUNK;
		}
		dIndex += AV_MMF_CHUNK_HEADER_SIZE;
		switch (dChunkID) {
		case AVMALIB_CHUNKCODE_MSPI :
			psTrack->pbMspi		= &(pbBuf[dIndex]);
			psTrack->dMspiSize	= sdChunkSize;
			break;
		case AVMALIB_CHUNKCODE_MTSU :
			psTrack->pbMtsu		= &(pbBuf[dIndex]);
			psTrack->dMtsuSize	= sdChunkSize;
			break;
		case AVMALIB_CHUNKCODE_MTSQ :
			psTrack->pbMtsq		= &(pbBuf[dIndex]);
			psTrack->dMtsqSize	= sdChunkSize;
			break;
		case AVMALIB_CHUNKCODE_MTSP :
			psTrack->pbMtsp		= &(pbBuf[dIndex]);
			psTrack->dMtspSize	= sdChunkSize;
			break;
		case AVMALIB_CHUNKCODE_MTHV :
			psTrack->pbMthv		= &(pbBuf[dIndex]);
			psTrack->dMthvSize	= sdChunkSize;
			break;
		default :
			break;
		}
		dIndex += sdChunkSize;
	}

	if ((psTrack->pbMtsq == NULL) ||(psTrack->dMtsqSize == 0))
	{
		return AV_MMF_ERR_SLENGTH;
	}
	return AV_MMF_FUNC_SUCCESS;
}


/*********************************************************************************
 *	_mmf_ATRCheck
 *
 *	Description:
 *			audio track chunk check (header information and chunk construction)
 *	Argument:
 *			psTrack			pointer to track information structure
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_ATRCheck(PTRACKINFO psTrack)
{
	int				sdResult, sdChunkSize;
	unsigned int				dSize, dIndex;
	unsigned char*				pbBuf;
	unsigned int				dChunkID, dChunkNo;
	unsigned char				fbWave;

/* Check Format Type				*/
	if (psTrack->pbMtr[0] != 0x00)
	{
		return AV_MMF_ERR_CHUNK;
	}

/* Check Sequence Type		*/
	if (psTrack->pbMtr[1] != 0x00)
	{
		return AV_MMF_ERR_CHUNK;
	}

/* Check Wave Type			*/
	if (((psTrack->pbMtr[2] != 0x10) && (psTrack->pbMtr[2] != 0x11)) ||
		((psTrack->pbMtr[3] & 0xF0) != 0x00))
	{
		return AV_MMF_ERR_CHUNK;
	}

/* Check Time Base		*/
	if (psTrack->pbMtr[4] != psTrack->pbMtr[5])
	{
		return AV_MMF_ERR_CHUNK;
	}
	sdResult = _mmf_GetTimebase(psTrack->pbMtr[4]);
	if (sdResult == AV_MMF_FUNC_ERROR)
	{
		return AV_MMF_ERR_CHUNK;
	}
	psTrack->dTimeBase = (unsigned int)sdResult;

	pbBuf	= psTrack->pbMtr;
	dSize	= psTrack->dMtrSize;
	dIndex	= 6;
	fbWave	= AV_MMF_MA2_VOICE_NULL;

/* Check sub chunk disposition	*/
	while (dSize > (dIndex + AV_MMF_CHUNK_HEADER_SIZE))
	{
		sdChunkSize = _mmf_MalibNextChunk(&pbBuf[dIndex], (dSize-dIndex),
			AVMALIB_CHUNK_PHASE_ATRSUB, &dChunkID, &dChunkNo);
		if (sdChunkSize < 0)
		{
			return AV_MMF_ERR_CHUNK;
		}
		dIndex += AV_MMF_CHUNK_HEADER_SIZE;
		switch (dChunkID)
		{
		case AVMALIB_CHUNKCODE_ASPI :
			psTrack->pbMspi		= &(pbBuf[dIndex]);
			psTrack->dMspiSize	= sdChunkSize;
			break;
		case AVMALIB_CHUNKCODE_ATSQ :
			psTrack->pbMtsq		= &(pbBuf[dIndex]);
			psTrack->dMtsqSize	= sdChunkSize;
			break;
		case AVMALIB_CHUNKCODE_AWA :
			if ((0x01 <= dChunkNo) && (dChunkNo <= 0x3E))
				fbWave = AV_MMF_MA2_VOICE_FOUND;
			break;
		default :
			break;
		}
		dIndex += sdChunkSize;
	}

	if ((psTrack->pbMtsq == NULL) ||(psTrack->dMtsqSize == 0) ||(fbWave == AV_MMF_MA2_VOICE_NULL))
	{
		return AV_MMF_ERR_SLENGTH;
	}
	return AV_MMF_FUNC_SUCCESS;
}


/*********************************************************************************
 *	_mmf_MspICheck
 *
 *	Description:
 *			seek & phrase info chunk check (get phrase information)
 *	Argument:
 *			psTrack			pointer to track information structure
 *			psPhrase		pointer to phrase information structure
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static void
_mmf_MspICheck(PTRACKINFO psTrack, PPHRASEINFO psPhrase)
{
	unsigned char*				pbBuf;
	unsigned int				dSize, dIndex;
	unsigned short				wTag;

	if (psTrack->pbMspi == NULL)
		return;

	pbBuf	= psTrack->pbMspi;
	dSize	= psTrack->dMspiSize;
	dIndex	= 0;

	while (dSize >= dIndex + AV_MMF_PHRAZE_SIZE_A)
	{
		wTag = (unsigned short)((((unsigned short)pbBuf[dIndex]) << 8) + (unsigned short)pbBuf[dIndex+1]);
		switch (wTag) {
		case AV_MMF_TAG_STARTPOINT :				/* start point					*/
			psTrack->dStartPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 3]));
			dIndex += AV_MMF_PHRAZE_SIZE_A;
			break;
		case AV_MMF_TAG_STOPPOINT :				/* stop point					*/
			psTrack->dStopPoint		= _mmf_Get4Byte(&(pbBuf[dIndex + 3]));
			dIndex += AV_MMF_PHRAZE_SIZE_A;
			break;
		case AV_MMF_TAG_PHRASE_A :					/* A melody						*/
			if (dSize < dIndex + AV_MMF_PHRAZE_SIZE_B)
				return ;
			psPhrase[0].dStartPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 3]));
			psPhrase[0].dStopPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 7]));
			dIndex += AV_MMF_PHRAZE_SIZE_B;
			break;
		case AV_MMF_TAG_PHRASE_B :					/* B melody						*/
			if (dSize < dIndex + AV_MMF_PHRAZE_SIZE_B)
				return ;
			psPhrase[1].dStartPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 3]));
			psPhrase[1].dStopPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 7]));
			dIndex += AV_MMF_PHRAZE_SIZE_B;
			break;
		case AV_MMF_TAG_PHRASE_E :					/* Ending						*/
			if (dSize < dIndex + AV_MMF_PHRAZE_SIZE_B)
				return ;
			psPhrase[2].dStartPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 3]));
			psPhrase[2].dStopPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 7]));
			dIndex += AV_MMF_PHRAZE_SIZE_B;
			break;
		case AV_MMF_TAG_PHRASE_I :					/* Intro						*/
			if (dSize < dIndex + AV_MMF_PHRAZE_SIZE_B)
				return ;
			psPhrase[3].dStartPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 3]));
			psPhrase[3].dStopPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 7]));
			dIndex += AV_MMF_PHRAZE_SIZE_B;
			break;
		case AV_MMF_TAG_PHRASE_K :					/* Interlude					*/
			if (dSize < dIndex + AV_MMF_PHRAZE_SIZE_B)
				return ;
			psPhrase[4].dStartPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 3]));
			psPhrase[4].dStopPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 7]));
			dIndex += AV_MMF_PHRAZE_SIZE_B;
			break;
		case AV_MMF_TAG_PHRASE_R :					/* Refrain						*/
			if (dSize < dIndex + AV_MMF_PHRAZE_SIZE_B)
				return ;
			psPhrase[5].dStartPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 3]));
			psPhrase[5].dStopPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 7]));
			dIndex += AV_MMF_PHRAZE_SIZE_B;
			break;
		case AV_MMF_TAG_PHRASE_S :					/* Bridge						*/
			if (dSize < dIndex + AV_MMF_PHRAZE_SIZE_B)
				return ;
			psPhrase[6].dStartPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 3]));
			psPhrase[6].dStopPoint	= _mmf_Get4Byte(&(pbBuf[dIndex + 7]));
			dIndex += AV_MMF_PHRAZE_SIZE_B;
			break;
		default :
			return;
		}
	}
	return;
}


/*********************************************************************************
 *	_mmf_STSPCheck
 *
 *	Description:
 *			track chunk check (offset of start point and stop point)
 *	Argument:
 *			psTrack			pointer to track information structure
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_STSPCheck(	PTRACKINFO psTrack)
{
	unsigned int				dStart, dStop, dSize;

	dSize	= psTrack->dMtsqSize;

	if (psTrack->dStartPoint == AV_MMF_STSP_OFFSET_NULL)
		dStart	= 0;
	else
		dStart	= psTrack->dStartPoint;

	if (psTrack->dStopPoint == AV_MMF_STSP_OFFSET_NULL)
		dStop	= dSize;
	else
		dStop	= psTrack->dStopPoint;

	if ((dStart >= dStop) || (dStart > dSize) || (dStop > dSize))
		return AV_MMF_ERR_CHUNK;

	return AV_MMF_FUNC_SUCCESS;
}


/*********************************************************************************
 *	_mmf_MtsuCheck2
 *
 *	Description:
 *			track chunk check (existence voice parameter)
 *	Argument:
 *			psTrack			pointer to track information structure
 *			bSmafType		SMAF type
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_MtsuCheck2(PTRACKINFO	psTrack, unsigned char bSmafType)
{
	unsigned char*				pbBuf;
	unsigned int				dSize, dIndex;

	if (psTrack->pbMtsu == NULL)
	{
		return AV_MMF_MA2_VOICE_NOTFOUND;
	}

	pbBuf	= psTrack->pbMtsu;
	dSize	= psTrack->dMtsuSize;
	dIndex	= 0;

	while (dSize > dIndex + 20)
	{
		if ((pbBuf[dIndex] != 0xFF) || (pbBuf[dIndex + 1] != 0xF0))
		{
			return AV_MMF_MA2_VOICE_NOTFOUND;
		}
		if (pbBuf[dIndex + 3] == 0x43)
		{
			if ((bSmafType == AV_MMF_SMAF_TYPE_MA1) && (pbBuf[dIndex + 4] == 0x02))
				return AV_MMF_MA2_VOICE_FOUND;
			if ((bSmafType == AV_MMF_SMAF_TYPE_MA2) && (pbBuf[dIndex + 4] == 0x03))
				return AV_MMF_MA2_VOICE_FOUND;
		}
		dIndex += (pbBuf[dIndex + 2] + 3);
	}

	return AV_MMF_MA2_VOICE_NOTFOUND;
}


/*********************************************************************************
 *	_mmf_GetFlex2L
 *
 *	Description:
 *			get flex data (duration, gate time)
 *	Argument:
 *			pbBuf			pointer to data top
 *			dSize			size of data (remain)
 *			pdRead			pointer to size of flex data
 *	Return:
 *			>=0				success(flex data value)
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_GetFlex2L(unsigned char* pbBuf, unsigned int dSize, unsigned int* pbRead)
{
	int				sdTemp;

	if ((dSize < 1) || ((dSize < 2) && (pbBuf[0] >= 0x80)))
		return AV_MMF_FUNC_ERROR;
	if (dSize >= 4)
	{
		sdTemp = pbBuf[0] + pbBuf[1] + pbBuf[2] + pbBuf[3];
		if (sdTemp == 0)
			return AV_MMF_FUNC_ERROR;
	}
	if (pbBuf[0] >= 0x80)
	{
		sdTemp = (int)( (((int)(pbBuf[0] & 0x7F)) << 7) +
							((int)(pbBuf[1] & 0x7F)) + 128 );
		*pbRead = 2;
	}
	else
	{
		sdTemp = (int)(pbBuf[0] & 0x7F);
		*pbRead = 1;
	}
	return sdTemp;
}


/*********************************************************************************
 *	_mmf_SeqDataCheck2
 *
 *	Description:
 *			track chunk check (sequence massage)
 *	Argument:
 *			psTrack			pointer to track information structure
 *			bSmafType		SMAF type
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_SeqDataCheck2(PTRACKINFO psTrack, unsigned char bSmafType)
{
	unsigned char*				pbBuf;
	unsigned int				dSize, dIndex;
	int				sdTemp;
	unsigned int				dPast, dGate, dFlexSize;

	if (psTrack->pbMtsq == NULL)
	{
		return AV_MMF_ERR_SLENGTH;
	}

	dPast	= 0;
	dGate	= 0;
	pbBuf	= psTrack->pbMtsq;
	dSize	= psTrack->dMtsqSize;
	dIndex	= 0;

/* scanning to EOS or stop point					*/
	while ( dSize > dIndex )
	{
		if (psTrack->dStartPoint== dIndex)		/* start point	*/
			psTrack->dStartTick	= dPast;
		if (psTrack->dStopPoint == dIndex)
		{	/* stop point	*/
			psTrack->dStopTick	= dPast;
			break;
		}

		if (dSize >= dIndex + 4)
		{
			sdTemp=pbBuf[dIndex]+pbBuf[dIndex+1]+pbBuf[dIndex+2]+pbBuf[dIndex+3];
			if (sdTemp == 0)
			{					/* EOS			*/
				if (bSmafType == AV_MMF_SMAF_TYPE_MA1)
					psTrack->dStopTick	= dPast + dGate;
				else
					psTrack->dStopTick	= dPast;
				break;
			}
		}

		sdTemp = _mmf_GetFlex2L(&pbBuf[dIndex], (dSize - dIndex), &dFlexSize);
		if (sdTemp < 0)
		{
			return AV_MMF_ERR_CHUNK;
		}
		dPast += sdTemp;
		if (dPast >= AV_MMF_PLAY_TIME_MAX)
		{
			return AV_MMF_ERR_LLENGTH;
		}

		if ((unsigned int)sdTemp >= dGate)			/* calculate remain of GT	*/
			dGate = 0;
		else
			dGate -= sdTemp;
		dIndex += dFlexSize;

		if (dSize < dIndex + 2)
		{
			return AV_MMF_ERR_CHUNK;
		}

		switch (pbBuf[dIndex])
		{
		case 0x00 :
			if ((pbBuf[dIndex + 1] & 0x30) != 0x30)
				dIndex += 2;
			else
				dIndex += 3;
			break;
		case 0xFF :
			switch (pbBuf[dIndex + 1])
			{
			case 0x00 :
				dIndex += 2;
				break;
			case 0xF0 :
				if (dSize < dIndex + 3)
				{
					return AV_MMF_ERR_CHUNK;
				}
				dIndex += (pbBuf[dIndex + 2] + 3);
				if (dSize < dIndex) {
					return AV_MMF_ERR_CHUNK;
				}
				if (pbBuf[dIndex - 1] != 0xF7)
				{
					return AV_MMF_ERR_CHUNK;
				}
				break;
			default :
				return AV_MMF_ERR_CHUNK;
			}
			break;
		default :
			sdTemp = _mmf_GetFlex2L(&pbBuf[dIndex+1], (dSize - dIndex- 1), &dFlexSize);
			if (sdTemp < 0)
			{
				return AV_MMF_ERR_CHUNK;
			}
			if (dGate < (unsigned int)sdTemp)
				dGate = (unsigned int)sdTemp;
			dIndex += (1 + dFlexSize);
			break;
		}
		if (dSize < dIndex) {
			return AV_MMF_ERR_CHUNK;
		}
	}


	if (psTrack->dStartTick  == AV_MMF_STSP_TIME_NULL)
	{
		if (psTrack->dStartPoint != AV_MMF_STSP_OFFSET_NULL)
		{
			return AV_MMF_ERR_CHUNK;
		}
		psTrack->dStartTick = 0;
	}

/* check start/stop point potision	*/
	if (psTrack->dStopTick  == AV_MMF_STSP_TIME_NULL)
	{
		if ((psTrack->dStopPoint != AV_MMF_STSP_OFFSET_NULL) && (psTrack->dStopPoint != dIndex))
		{
			return AV_MMF_ERR_CHUNK;
		}
		if (bSmafType == AV_MMF_SMAF_TYPE_MA1)
			psTrack->dStopTick	= dPast + dGate;
		else
			psTrack->dStopTick	= dPast;
	}

/* calculate playback time of this track	*/
	psTrack->dPlayTime = psTrack->dStopTick - psTrack->dStartTick;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "Play time: %ld\n", psTrack->dPlayTime);
	#endif

	return AV_MMF_FUNC_SUCCESS;
}


/*********************************************************************************
 *	TrackChunk_Check2
 *
 *	Description:
 *			MA-1/2 track chunk check
 *	Argument:
 *			psLoad			pointer to load information structure
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_TrackChunkCheck2(PLOADINFO psLoad)
{
	PTRACKINFO			psTrack;
	int				sdResult;
	unsigned char				i, fbVoice;

/* delete track information of MA-3/5	*/
		psLoad->sTrack_Info[5].pbMtr	= NULL;
		psLoad->sTrack_Info[5].dMtrSize	= 0;
		psLoad->sTrack_Info[6].pbMtr	= NULL;
		psLoad->sTrack_Info[6].dMtrSize	= 0;

/* fix SMAF Type												*/
	psLoad->dSmafType = AV_MMF_SMAF_TYPE_MA1;
	for (i = 1; i < 5; i++)
	{
		if (psLoad->sTrack_Info[i].pbMtr != NULL)
		{
			psLoad->dSmafType = AV_MMF_SMAF_TYPE_MA2;
			break;
		}
	}
	if (psLoad->sTrack_Info[AV_MMF_ATR_TRACK_NO].pbMtr != NULL)
		psLoad->dSmafType = AV_MMF_SMAF_TYPE_MA2;

	if (psLoad->dSmafType == AV_MMF_SMAF_TYPE_MA1)
	{	/* MA-1			*/
		if (psLoad->sTrack_Info[0].pbMtr == NULL)
		{
			return AV_MMF_ERR_SLENGTH;
		}
		psTrack = &(psLoad->sTrack_Info[0]);
		if (psTrack->dMtrSize <= AV_MMF_MINIMUM_TRACKSIZE2)
		{
			return AV_MMF_ERR_CHUNK;
		}
		sdResult = _mmf_MTRCheck(psTrack, AV_MMF_SMAF_TYPE_MA1);
		if (sdResult != AV_MMF_FUNC_SUCCESS)
		{
			return sdResult;
		}
		_mmf_MspICheck(psTrack, &(psLoad->sPhrase_Info[0]));
		sdResult = _mmf_STSPCheck(psTrack);
		if (sdResult != AV_MMF_FUNC_SUCCESS)
		{
			return sdResult;
		}
		sdResult = _mmf_MtsuCheck2(psTrack, AV_MMF_SMAF_TYPE_MA1);
		if (sdResult != AV_MMF_MA2_VOICE_FOUND)
		{
			return AV_MMF_ERR_CHUNK;
		}
		sdResult = _mmf_SeqDataCheck2(psTrack, AV_MMF_SMAF_TYPE_MA1);
		if (sdResult != AV_MMF_FUNC_SUCCESS)
		{
			return sdResult;
		}
		psLoad->dPlayTime	= psTrack->dPlayTime;
		psLoad->dStartTime	= psTrack->dStartTick;
		psLoad->dTimeBase	= psTrack->dTimeBase;
		return AV_MMF_FUNC_SUCCESS;
	}
	else
	{											/* MA-2			*/
		psLoad->sTrack_Info[0].pbMtr	= NULL;
		psLoad->sTrack_Info[0].dMtrSize	= 0;

		for (i = 1; i < 5; i++)
		{
			psTrack = &(psLoad->sTrack_Info[i]);
			if (psTrack->pbMtr == NULL)
				continue;
			if (psTrack->dMtrSize <= AV_MMF_MINIMUM_TRACKSIZE2)
			{
				return AV_MMF_ERR_CHUNK;
			}
			sdResult = _mmf_MTRCheck(psTrack, AV_MMF_SMAF_TYPE_MA2);
			if (sdResult != AV_MMF_FUNC_SUCCESS)
			{
				return sdResult;
			}
			_mmf_MspICheck(psTrack, &(psLoad->sPhrase_Info[0]));
			sdResult = _mmf_STSPCheck(psTrack);
			if (sdResult != AV_MMF_FUNC_SUCCESS)
			{
				return sdResult;
			}
			sdResult = _mmf_SeqDataCheck2(psTrack, AV_MMF_SMAF_TYPE_MA2);
			if (sdResult != AV_MMF_FUNC_SUCCESS)
			{
				return sdResult;
			}
			psLoad->dPlayTime	= psTrack->dPlayTime;
			psLoad->dStartTime	= psTrack->dStartTick;
			psLoad->dTimeBase	= psTrack->dTimeBase;
		}
		if (psLoad->sTrack_Info[AV_MMF_ATR_TRACK_NO].pbMtr != NULL)
		{
			psTrack = &(psLoad->sTrack_Info[AV_MMF_ATR_TRACK_NO]);

			if (psTrack->dMtrSize <= AV_MMF_MINIMUM_TRACKSIZE2)
			{
				return AV_MMF_ERR_CHUNK;
			}
			sdResult = _mmf_ATRCheck(psTrack);
			if (sdResult != AV_MMF_FUNC_SUCCESS)
			{
				return sdResult;
			}
			_mmf_MspICheck(psTrack, &(psLoad->sPhrase_Info[0]));
			sdResult = _mmf_STSPCheck(psTrack);
			if (sdResult != AV_MMF_FUNC_SUCCESS)
			{
				return sdResult;
			}
			sdResult = _mmf_SeqDataCheck2(psTrack, AV_MMF_SMAF_TYPE_MA2);
			if (sdResult != AV_MMF_FUNC_SUCCESS)
			{
				return sdResult;
			}
			psLoad->dPlayTime	= psTrack->dPlayTime;
			psLoad->dStartTime	= psTrack->dStartTick;
			psLoad->dTimeBase	= psTrack->dTimeBase;
		}

/* totaling of track information	*/
		for (i = 1; i < AV_MMF_MAX_TRACK_NUM; i++)
		{
			psTrack = &(psLoad->sTrack_Info[i]);
			if (psTrack->pbMtr == NULL)
				continue;

			if (psLoad->dPlayTime   < psTrack->dPlayTime)
				psLoad->dPlayTime   = psTrack->dPlayTime;
			if (psLoad->dTimeBase  != psTrack->dTimeBase)
			{
				return AV_MMF_ERR_CHUNK;
			}
			if (psLoad->dStartTime != psTrack->dStartTick)
			{
				return AV_MMF_ERR_CHUNK;
			}
			if (_mmf_MtsuCheck2(psTrack, AV_MMF_SMAF_TYPE_MA2) == AV_MMF_FUNC_SUCCESS)
				fbVoice = AV_MMF_MA2_VOICE_FOUND;
		}

		fbVoice = AV_MMF_MA2_VOICE_FOUND;
		for (i = 1; i < 5; i++)
		{
			psTrack = &(psLoad->sTrack_Info[i]);
			if (psTrack->pbMtr == NULL)
				continue;
			if (_mmf_MtsuCheck2(psTrack, AV_MMF_SMAF_TYPE_MA2) == AV_MMF_MA2_VOICE_FOUND)
			{
				fbVoice = AV_MMF_MA2_VOICE_FOUND;
				break;
			}
			else
				fbVoice = AV_MMF_MA2_VOICE_NOTFOUND;
		}

		if (fbVoice == AV_MMF_MA2_VOICE_NOTFOUND)
		{
			return AV_MMF_ERR_CHUNK;
		}
	}

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "Play time: %ld\n", psLoad->dPlayTime);
	#endif

	return AV_MMF_FUNC_SUCCESS;
}

static unsigned char
_mmf_GetByte3L(PHUFFMANINFO	psHuf)
{
	psHuf->dReadSize ++;
	if (psHuf->dReadSize > psHuf->dMtsqSize)
	{
		return 0;
	}
	return *(psHuf->psBuffer++);
}


/*********************************************************************************
 *	_mmf_GetFlex3L
 *
 *	Description:
 *			MA-3 sequence data check
 *	Argument:
 *			psLoad			pointer to load information structure
 *			pdRead			pointer to size of flex data
 *	Return:
 *			>=0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_GetFlex3L(PLOADINFO	psLoad, unsigned int* pdRead)
{
	unsigned int				dTemp, dRead;
	unsigned char				bTemp;

	dRead = 1;
	bTemp = psLoad->pfnGetByte( &(psLoad->sHuffman_Info) );
	dTemp = (unsigned int)(bTemp & 0x7F);
	while (bTemp & 0x80)
	{
		if (dRead >= 4)
			return AV_MMF_FUNC_ERROR;
		dRead ++;
		bTemp = psLoad->pfnGetByte( &(psLoad->sHuffman_Info) );
		dTemp = (dTemp << 7) + (unsigned int)(bTemp & 0x7F);
	}
	if (dTemp >= AV_MMF_PLAY_TIME_MAX)
		return AV_MMF_FUNC_ERROR;
	*pdRead = dRead;
	return (int)dTemp;
}


/*********************************************************************************
 *	_mmf_SeqDataCheck3
 *
 *	Description:
 *			MA-3 sequence data check
 *	Argument:
 *			psLoad			pointer to load information structure
 *			bSmafType		SMAF type
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_SeqDataCheck3(PLOADINFO psLoad, unsigned char bSmafType)
{
	PTRACKINFO			psTrk;
	PPHRASEINFO			psPhr;
	PHUFFMANINFO		psHuf;
	unsigned int				dIndex, fdPhrase, dSize, dPast, dGate, dReadSize, i;
	unsigned int				dStartTick, dStopTick;
	int				sdTemp;
	unsigned char				bTemp;

	if (bSmafType == AV_MMF_SMAF_TYPE_MA3)
	{			/* MA-3		*/
		psTrk		= &(psLoad->sTrack_Info[5]);
		dStartTick	= AV_MMF_STSP_TIME_NULL;
		dStopTick	= AV_MMF_STSP_TIME_NULL;
	}
	else
	{											/* MA-5		*/
		psTrk		= &(psLoad->sTrack_Info[6]);
		dStartTick	= psTrk->dStartTick;
		dStopTick	= psTrk->dStopTick;
		psTrk->dStartTick	= AV_MMF_STSP_TIME_NULL;
		psTrk->dStopTick	= AV_MMF_STSP_TIME_NULL;
	}

	psPhr	= &(psLoad->sPhrase_Info[0]);
	psHuf	= &(psLoad->sHuffman_Info);
	fdPhrase= 0;
	dIndex	= 0;
	dPast	= 0;
	dGate	= 0;
	dSize	= psHuf->dSeqSize;

	if (psHuf->dSeqSize == 0)
	{
		return AV_MMF_ERR_SLENGTH;
	}

	for (i = 0; i < AV_MMF_MAX_PHRASE_INFO; i++)
	{
		if (psPhr[i].dStartPoint != AV_MMF_STSP_OFFSET_NULL)
			fdPhrase = 1;
	}

/* scanning sequence data to EOS or stop point	*/
	while (dSize >= dIndex)
	{

		if (psTrk->dStartPoint	== dIndex)
			psTrk->dStartTick	=  dPast;
		if (psTrk->dStopPoint	== dIndex)			/* stop point		*/
			psTrk->dStopTick	=  dPast;
		if (fdPhrase)
		{
			for (i = 0; i < AV_MMF_MAX_PHRASE_INFO; i++)
			{
				if (psPhr[i].dStartPoint== dIndex)
					psPhr[i].dStartTick	=  dPast;
				if (psPhr[i].dStopPoint	== dIndex)
					psPhr[i].dStopTick	=  dPast;
			}
		}

		if ((psTrk->dStopTick != AV_MMF_STSP_TIME_NULL) || (dSize == dIndex))
			break;

		sdTemp = _mmf_GetFlex3L(psLoad, &dReadSize);	/* Duration			*/
		if (sdTemp < 0) {
			return AV_MMF_ERR_CHUNK;
		}
		dPast	+= (unsigned int)sdTemp;
		if (dPast >= AV_MMF_PLAY_TIME_MAX)
		{
			return AV_MMF_ERR_LLENGTH;
		}

		if ((unsigned int)sdTemp >= dGate)
			dGate = 0;
		else
			dGate -= (unsigned int)sdTemp;
		dIndex += dReadSize;

		bTemp = psLoad->pfnGetByte(psHuf);
		dIndex ++;

		switch (bTemp & 0xF0)
		{
		case 0x90 :
			psLoad->pfnGetByte(psHuf);	/*Note number*/
			dIndex ++;
			psLoad->pfnGetByte(psHuf);	/*Key Velocity*/
			dIndex ++;
			sdTemp = _mmf_GetFlex3L(psLoad, &dReadSize);
			if (sdTemp < 0) {
				return AV_MMF_ERR_CHUNK;
			}
			dIndex += dReadSize;
			if ((unsigned int)sdTemp > dGate)
				dGate = sdTemp;
			break;
		case 0x80 :
			psLoad->pfnGetByte(psHuf);	/*Note number*/
			dIndex ++;
			sdTemp = _mmf_GetFlex3L(psLoad, &dReadSize);
			if (sdTemp < 0) {
				return AV_MMF_ERR_CHUNK;
			}
			dIndex += dReadSize;
			if ((unsigned int)sdTemp > dGate)
				dGate = sdTemp;
			break;
		case 0xA0 :
		case 0xB0 :
		case 0xE0 :
			bTemp = psLoad->pfnGetByte(psHuf);	/*B0: Conrol number, E0:Pitch Bend Change LSB*/
			dIndex ++;
			bTemp = psLoad->pfnGetByte(psHuf);	/*B0: Conrol value, E0:Pitch Bend Change MSB*/
			dIndex ++;
			break;
		case 0xC0 :
		case 0xD0 :
			bTemp = psLoad->pfnGetByte(psHuf);
			dIndex ++;
			break;
		default :
			switch (bTemp)
			{
			case 0xF0 :
				sdTemp = _mmf_GetFlex3L(psLoad, &dReadSize);
				if (sdTemp < 0) {
					return AV_MMF_ERR_CHUNK;
				}
				for (i = 0; i < (unsigned int)sdTemp; i++)
					bTemp = psLoad->pfnGetByte(psHuf);
				if (bTemp != 0xF7) {
					return AV_MMF_ERR_CHUNK;
				}
				dIndex += (unsigned int)sdTemp + dReadSize;
				break;
			case 0xFF :
				bTemp = psLoad->pfnGetByte(psHuf);
				dIndex ++;
				switch (bTemp)
				{
				case 0x00 :
					break;
				case 0x2F :
					bTemp = psLoad->pfnGetByte(psHuf);
					dIndex ++;
					if (bTemp != 0x00)
					{
						return AV_MMF_ERR_CHUNK;
					}
					dGate = 0;
					psTrk->dStopTick = dPast;
					dIndex = dSize;
					break;
				default :
					return AV_MMF_ERR_CHUNK;
				}
				break;
			default :
				return AV_MMF_ERR_CHUNK;
			}
			break;
		}
		if ((dSize < dIndex) || (psHuf->dReadSize > psHuf->dMtsqSize))
		{
			return AV_MMF_ERR_CHUNK;
		}
	}

	if (bSmafType == AV_MMF_SMAF_TYPE_MA3)
	{			/* MA-3		*/
/* check start point				*/
		if (psTrk->dStartTick == AV_MMF_STSP_TIME_NULL)
		{
			if (psTrk->dStartPoint != AV_MMF_STSP_OFFSET_NULL)
			{
				return AV_MMF_ERR_CHUNK;
			}
			psTrk->dStartPoint	= 0;
			psTrk->dStartTick	= 0;
		}
/* check stop point				*/
		if (psTrk->dStopTick == AV_MMF_STSP_TIME_NULL)
		{
			if (psTrk->dStopPoint != AV_MMF_STSP_OFFSET_NULL)
			{
				return AV_MMF_ERR_CHUNK;
			}
			psTrk->dStopPoint	= dSize;
			psTrk->dStopTick	= dPast + dGate;
		}
/* adjust phrase information	*/
		for (i = 0; i < AV_MMF_MAX_PHRASE_INFO; i++)
		{
			if (psPhr[i].dStartPoint <= psTrk->dStartPoint)
				psPhr[i].dStartTick   = psTrk->dStartTick;
			if (psPhr[i].dStopPoint  >= psTrk->dStopPoint)
				psPhr[i].dStopTick    = psTrk->dStopTick;
			if (psPhr[i].dStopPoint  <= psTrk->dStartPoint)
				psPhr[i].dStopTick    = AV_MMF_STSP_TIME_NULL;
		}
	}
	else
	{											/* MA-5		*/
/* check stop point				*/
		if (dStopTick > dPast)
		{
			return AV_MMF_ERR_CHUNK;
		}
		psTrk->dStartTick	= dStartTick;
		psTrk->dStopTick	= dStopTick;
	}

/* calculate playback time of this track	*/
	psTrk->dPlayTime = psTrk->dStopTick - psTrk->dStartTick;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "Play time: %ld\n", psTrk->dPlayTime);
	#endif

	return AV_MMF_FUNC_SUCCESS;
}




/*********************************************************************************
 *	_mmf_DecodeGetbitL
 *
 *	Description:
 *			get 1 bit with error check
 *	Argument:
 *			psHuf			pointer to huffman information structure
 *	Return:
 *			read data
 ********************************************************************************/


static unsigned char
_mmf_DecodeGetbitL(PHUFFMANINFO psHuf)
{
	char czero = 0;

	if (--psHuf->sbBitC < czero)
	{
		if (psHuf->dReadSize >= psHuf->dMtsqSize)
			return 0;
		psHuf->sbBitC	= 7;
		psHuf->bByte	= *(psHuf->psBuffer++);
		psHuf->dReadSize ++;
	}
	return (unsigned char)(psHuf->bByte & g_abBitMaskTable1[(int)(psHuf->sbBitC)]);
}


/*********************************************************************************
 *	_mmf_DecodeGetbits
 *
 *	Description:
 *			get 8 bit
 *	Argument:
 *			psHuf			pointer to huffman information structure
 *	Return:
 *			read data
 ********************************************************************************/
static unsigned char
_mmf_DecodeGetbits(PHUFFMANINFO psHuf)
{
	unsigned short				wTemp;
	unsigned char				bData1, bData2;

	if (psHuf->dReadSize >= psHuf->dMtsqSize)
		return 0;
	bData1	= psHuf->bByte;
	bData2	= *(psHuf->psBuffer++);
	psHuf->bByte = bData2;
	psHuf->dReadSize ++;
	wTemp = (unsigned short)((((unsigned short)bData1) << 8) + ((unsigned short)bData2));
	return (unsigned char)((wTemp & g_awBitMaskTable2[(int)(psHuf->sbBitC)]) >> psHuf->sbBitC);
}


/*********************************************************************************
 *	_mmf_DecodeTree
 *
 *	Description:
 *			make MA-3 huffman decode tree
 *	Argument:
 *			psHuf			pointer to huffman information structure
 *	Return:
 *			!0				success(sequence data size)
 *			0				error
 ********************************************************************************/
static int
_mmf_DecodeTree(	PHUFFMANINFO psHuf)
{
	unsigned int				dNode, dEmpty, dIndex, i;
	short				*pswLeft, *pswRight, *pswPNode;
	unsigned char				bTemp;

    	if (_mmf_DecodeGetbitL(psHuf))
	{
		if (psHuf->dReadSize >= psHuf->dMtsqSize)
			return AV_MMF_HUFFMAN_TREE_FAILURE;

		pswLeft	= &(psHuf->swLeft[256]);
		pswRight= &(psHuf->swRight[256]);
		pswPNode= &(psHuf->swRight[0]);
		for (i = 0; i < 256; i++)
		{
			pswLeft[i]	= -1;
			pswRight[i]	= -1;
			pswPNode[i]	= 0;
		}
		dNode	= 2;
		dEmpty	= 1;
		dIndex	= 0;
	}
	else
		return AV_MMF_HUFFMAN_TREE_FAILURE;

	while (dNode != 0)
	{
		if ((dEmpty >= 256) || (dNode >= 257))
			return AV_MMF_HUFFMAN_TREE_FAILURE;

		bTemp = _mmf_DecodeGetbitL(psHuf);
		if (psHuf->dReadSize >= psHuf->dMtsqSize)
			return AV_MMF_HUFFMAN_TREE_FAILURE;

		if (bTemp)
		{
			dNode ++;
			if (pswLeft[dIndex] == -1)
				pswLeft[dIndex]	= (short)(dEmpty + 256);
			else
				pswRight[dIndex]= (short)(dEmpty + 256);
			pswPNode[dEmpty] = (short)dIndex;
			dIndex = dEmpty;
			dEmpty ++;
		}
		else
		{
			dNode --;
			bTemp = _mmf_DecodeGetbits(psHuf);
			if (psHuf->dReadSize >= psHuf->dMtsqSize)
				return AV_MMF_HUFFMAN_TREE_FAILURE;

			if (pswLeft[dIndex] == -1)
				pswLeft[dIndex]	= (short)bTemp;
			else
			{
				pswRight[dIndex]= (short)bTemp;
				while ((pswRight[dIndex] != -1) && (dIndex != 0))
					dIndex = (unsigned int)pswPNode[dIndex];
			}
		}
	}

	for (i = 0; i < 256; i++)
	{
		if (pswLeft[i]	== -1)
			pswLeft[i]	= 0;
		if (pswRight[i]	== -1)
			pswRight[i]	= 0;
	}
	return AV_MMF_HUFFMAN_TREE_SUCCESS;
}


/*********************************************************************************
 *	_mmf_DecodeInit
 *
 *	Description:
 *			make MA-3 huffman decode tree
 *	Argument:
 *			psHuf			pointer to huffman information structure
 *	Return:
 *			!0				success(sequence data size)
 *			0				error
 ********************************************************************************/
static unsigned int
_mmf_DecodeInit(PHUFFMANINFO	psHuf)
{
	unsigned int				dSeqSize;

	if (psHuf->dMtsqSize <= 5)
	{
		return AV_MMF_HUFFMAN_TREE_FAILURE;
	}

	dSeqSize = _mmf_Get4Byte(psHuf->psBuffer);
	psHuf->psBuffer += 4;
	psHuf->dReadSize = 4;
	psHuf->sbBitC = 0;
	psHuf->bByte  = 0;
	if (_mmf_DecodeTree(psHuf) == AV_MMF_HUFFMAN_TREE_FAILURE)
		return AV_MMF_HUFFMAN_TREE_FAILURE;
	else
		return dSeqSize;
}


/*********************************************************************************
 *	_mmf_DecodeByte3L
 *
 *	Description:
 *			get 1 byte (from compressed data)
 *	Argument:
 *			psHuf			pointer to huffman information structure
 *	Return:
 *			unsigned char			success(read data)
 *			0				error code
 ********************************************************************************/
static unsigned char
_mmf_DecodeByte3L (PHUFFMANINFO		psHuf)
{
	unsigned int				bData, bIndex;
	char czero = 0;
	bIndex = 256;
	while (bIndex >= 256)
	{
		if (--psHuf->sbBitC < czero)
		{
			psHuf->dReadSize ++;
			if (psHuf->dReadSize > psHuf->dMtsqSize)
				return 0;
			psHuf->sbBitC	= 7;
			psHuf->bByte	= *(psHuf->psBuffer++);
		}
		bData = (unsigned char)(psHuf->bByte & g_abBitMaskTable1[(int)(psHuf->sbBitC)]);
		if (bData)
			bIndex = psHuf->swRight[bIndex];
		else
			bIndex = psHuf->swLeft[bIndex];
	}
	return (unsigned char)bIndex;
}


/*********************************************************************************
 *	_mmf_TrackChunkCheck3
 *
 *	Description:
 *			MA-3 track chunk check
 *	Argument:
 *			psLoad			pointer to load information structure
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_TrackChunkCheck3(PLOADINFO psLoad)
{
	PTRACKINFO			psTrack;
	int				sdResult;

	if (psLoad->sTrack_Info[5].pbMtr == NULL)
	{
		return AV_MMF_ERR_SLENGTH;
	}
	psTrack = &(psLoad->sTrack_Info[5]);
	if (psTrack->dMtrSize <= AV_MMF_MINIMUM_TRACKSIZE3)
	{
		return AV_MMF_ERR_CHUNK;
	}
	sdResult = _mmf_MTRCheck(psTrack, AV_MMF_SMAF_TYPE_MA3);
	if (sdResult != AV_MMF_FUNC_SUCCESS)
	{
		return sdResult;
	}
	_mmf_MspICheck(psTrack, &(psLoad->sPhrase_Info[0]));

	psLoad->sHuffman_Info.psBuffer	= psTrack->pbMtsq;
	psLoad->sHuffman_Info.dMtsqSize	= psTrack->dMtsqSize;

/* Initialize Huffman information structure	*/
	if (psTrack->pbMtr[0] == 0x01)
	{			/* Compressed Foramt	*/
		psLoad->sHuffman_Info.dSeqSize = _mmf_DecodeInit( &(psLoad->sHuffman_Info) );
		if (psLoad->sHuffman_Info.dSeqSize == AV_MMF_HUFFMAN_TREE_FAILURE)
		{
			return AV_MMF_ERR_CHUNK;
		}
		psLoad->pfnGetByte = _mmf_DecodeByte3L;
		psLoad->sHuffman_Info.psFBuf	= psLoad->sHuffman_Info.psBuffer;
		psLoad->sHuffman_Info.sbFBit	= psLoad->sHuffman_Info.sbBitC;
		psLoad->sHuffman_Info.bFByte	= psLoad->sHuffman_Info.bByte;
	}
	else
	{										/* No Compressed Foramt	*/
		psLoad->pfnGetByte = _mmf_GetByte3L;
		psLoad->sHuffman_Info.dSeqSize	= psTrack->dMtsqSize;
		psLoad->sHuffman_Info.dReadSize	= 0;
		psLoad->sHuffman_Info.psFBuf	= psTrack->pbMtsq;
		psLoad->sHuffman_Info.sbFBit	= 0;
		psLoad->sHuffman_Info.bFByte	= 0;
	}

	psTrack->dMtsqSize = psLoad->sHuffman_Info.dSeqSize;
	sdResult = _mmf_STSPCheck(psTrack);
	psTrack->dMtsqSize = psLoad->sHuffman_Info.dMtsqSize;

	if (sdResult != AV_MMF_FUNC_SUCCESS)
	{
		return sdResult;
	}
	sdResult = _mmf_SeqDataCheck3(psLoad, AV_MMF_SMAF_TYPE_MA3);
	if (sdResult != AV_MMF_FUNC_SUCCESS)
	{
		return sdResult;
	}
	psLoad->dPlayTime	= psTrack->dPlayTime;
	psLoad->dStartTime	= psTrack->dStartTick;
	psLoad->dTimeBase	= psTrack->dTimeBase;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "Play time: %ld\n", psLoad->dPlayTime);
	#endif

	return AV_MMF_FUNC_SUCCESS;
}


/*********************************************************************************
 *	_mmf_CheckM5P
 *
 *	Description:
 *			MA-5 profile data chunk check
 *	Argument:
 *			psLoad			pointer to load information structure
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_CheckM5P(PLOADINFO psLoad)
{
	PTRACKINFO			psTrk;
	PPHRASEINFO			psPhr;
	POPTIONINFO			psOptn;
	unsigned char				*pbOpda, *pbM5p;
	unsigned int				dOSize,  dMSize, dIndex, dPhraseFlag, i;
	unsigned int				dChunkID, dChunkNo;
	int				sdChunkSize;

	psOptn	= &(psLoad->sOption_Info);
	psTrk	= &(psLoad->sTrack_Info[6]);
	psPhr	= &(psLoad->sPhrase_Info[0]);
	pbOpda	= psOptn->pbOpda;
	dOSize	= psOptn->dOpdaSize;
	dIndex	= 0;
	pbM5p	= NULL;
	dMSize	= 0;

/* search Pro5 Chunk	*/
	while (dOSize > (dIndex + AV_MMF_CHUNK_HEADER_SIZE))
	{
		sdChunkSize = _mmf_MalibNextChunk(&pbOpda[dIndex], (dOSize - dIndex),
			AVMALIB_CHUNK_PHASE_OPDASUB, &dChunkID, &dChunkNo);
		if (sdChunkSize < AVMASMW_SUCCESS)		return AVMASMW_ERROR;
		dIndex += AV_MMF_CHUNK_HEADER_SIZE;
		if ((dChunkID == AVMALIB_CHUNKCODE_M5P) && (dChunkNo == 0x05))
		{
			pbM5p	= &pbOpda[dIndex];
			dMSize	= (unsigned int)sdChunkSize;
			break;
		}
		dIndex += sdChunkSize;
	}

	if ((pbM5p == NULL) || (dMSize < 12))		return AVMASMW_ERROR;

	dPhraseFlag			= _mmf_Get4Byte(&pbM5p[0]);
	psTrk->dStartTick	= _mmf_Get4Byte(&pbM5p[4]);		/* start point		*/
	psTrk->dStopTick	= _mmf_Get4Byte(&pbM5p[8]);		/* stop point		*/
	dIndex				= 12;

	if (psTrk->dStartTick >= psTrk->dStopTick)	return AVMASMW_ERROR;

	for (i = 0; i < AV_MMF_MAX_PHRASE_INFO; i ++)
	{
		if (dMSize < (dIndex + 8))		break;
		if (dPhraseFlag & (0x80000000 >> i))
		{
			psPhr[i].dStartTick	= _mmf_Get4Byte(&pbM5p[dIndex]);
			psPhr[i].dStopTick	= _mmf_Get4Byte(&pbM5p[dIndex + 4]);
			if (psPhr[i].dStartTick >= psPhr[i].dStopTick)
			{
				psPhr[i].dStartTick	= AV_MMF_STSP_TIME_NULL;
				psPhr[i].dStopTick	= AV_MMF_STSP_TIME_NULL;
			}
			if (psPhr[i].dStartTick < psTrk->dStartTick)
				psPhr[i].dStartTick = psTrk->dStartTick;
			if (psPhr[i].dStopTick  > psTrk->dStopTick)
				psPhr[i].dStopTick  = psTrk->dStopTick;
			dIndex += 8;
		}
	}
	return AVMASMW_SUCCESS;
}


/*********************************************************************************
 *	_mmf_TrackChunkCheck5
 *
 *	Description:
 *			MA-5 track chunk check
 *	Argument:
 *			psLoad			pointer to load information structure
 *			dMode			load mode
 *	Return:
 *			0				success
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_TrackChunkCheck5(PLOADINFO psLoad)
{
	PTRACKINFO			psTrack;
	int				sdResult;

	if (psLoad->sTrack_Info[6].pbMtr == NULL)
	{
		return AV_MMF_ERR_SLENGTH;
	}
	psTrack = &(psLoad->sTrack_Info[6]);
	if (psTrack->dMtrSize <= AV_MMF_MINIMUM_TRACKSIZE3)
	{
		return AV_MMF_ERR_CHUNK;
	}
	sdResult = _mmf_MTRCheck(psTrack, AV_MMF_SMAF_TYPE_MA5);
	if (sdResult != AV_MMF_FUNC_SUCCESS)			return sdResult;

	psLoad->sHuffman_Info.psBuffer	= psTrack->pbMtsq;
	psLoad->sHuffman_Info.dMtsqSize	= psTrack->dMtsqSize;
	psLoad->sHuffman_Info.dSeqSize	= psTrack->dMtsqSize;
	psLoad->sHuffman_Info.dReadSize	= 0;
	psLoad->pfnGetByte				= _mmf_GetByte3L;


	sdResult = _mmf_SeqDataCheck3(psLoad, AV_MMF_SMAF_TYPE_MA5);
	if (sdResult != AV_MMF_FUNC_SUCCESS)		return sdResult;

	psLoad->dPlayTime	= psTrack->dPlayTime;
	psLoad->dStartTime	= psTrack->dStartTick;
	psLoad->dTimeBase	= psTrack->dTimeBase;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "Play time: %ld\n", psLoad->dPlayTime);
	#endif

	return AV_MMF_FUNC_SUCCESS;
}
/*********************************************************************************
 *	_mmf_GetHvData
 *
 *	Description:
 *			HV Data Chunk Check
 *	Argument:
 *			psLoad			pointer to load information structure
 *			bCType			Contents Type
 *	Return:
 *			nothing
 ********************************************************************************/
static void
_mmf_GetHvData(PLOADINFO psLoad, unsigned char bCType)
{
	unsigned char*				pbHvData;
	unsigned int				dHvDataSize;

	unsigned char*				pbVoice;
	unsigned char*				pbScript;
	unsigned char*				pbSetup;
	unsigned int				dVoiceSize, dScriptSize, dSetupSize, dIndex;
	unsigned char				bHvCh;
	unsigned short				wTag, wSize;

	unsigned int				dChunkID, dChunkNo;
	int				sdChunkSize;

	if ((psLoad->dSmafType == AV_MMF_SMAF_TYPE_MA1) ||(psLoad->dSmafType == AV_MMF_SMAF_TYPE_MA2))
		return ;

	if ((bCType & 0x0F) == 0x08)
		return ;

	if (psLoad->dSmafType == AV_MMF_SMAF_TYPE_MA3)
	{
		pbHvData	= psLoad->sTrack_Info[5].pbMthv;
		dHvDataSize	= psLoad->sTrack_Info[5].dMthvSize;
	}
	else
	{
		pbHvData	= psLoad->sTrack_Info[6].pbMthv;
		dHvDataSize	= psLoad->sTrack_Info[6].dMthvSize;
	}

	if ((pbHvData == NULL) || (dHvDataSize < AV_MMF_CHUNK_HEADER_SIZE))
		return ;

	pbVoice		= NULL;
	pbScript	= NULL;
	pbSetup		= NULL;
	dVoiceSize	= 0;
	dScriptSize	= 0;
	dSetupSize	= 0;
	bHvCh		= AV_MMF_HV_CHANNEL_NULL;
	dIndex		= 0;

	while (dHvDataSize > (dIndex + AV_MMF_CHUNK_HEADER_SIZE))
	{
		sdChunkSize	= _mmf_MalibNextChunk(&(pbHvData[dIndex]), (dHvDataSize - dIndex),
			AVMALIB_CHUNK_PHASE_MTHVSUB, &dChunkID, &dChunkNo);
		if (sdChunkSize < 0)		return ;
		dIndex	+= AV_MMF_CHUNK_HEADER_SIZE;
		switch (dChunkID)
		{
		case AVMALIB_CHUNKCODE_MHVS :
			pbSetup		= &(pbHvData[dIndex]);
			dSetupSize	= (unsigned int)sdChunkSize;
			break;
		case AVMALIB_CHUNKCODE_HVP :
			if (dChunkNo != 0)		break;
			pbVoice		= &(pbHvData[dIndex - AV_MMF_CHUNK_HEADER_SIZE]);
			dVoiceSize	= (unsigned int)(sdChunkSize + AV_MMF_CHUNK_HEADER_SIZE);
			break;
		case AVMALIB_CHUNKCODE_MHSC :
			pbScript	= &(pbHvData[dIndex]);
			dScriptSize	= (unsigned int)sdChunkSize;
			break;
		}
		dIndex	+= sdChunkSize;
	}

	dIndex	= 0;
	while (dSetupSize >= dIndex + 4 )
	{
		wTag	= (unsigned short)(((unsigned short)(pbSetup[dIndex    ]) << 8) + pbSetup[dIndex + 1]);
		wSize	= (unsigned short)(((unsigned short)(pbSetup[dIndex + 2]) << 8) + pbSetup[dIndex + 3]);
		dIndex += 4;
		if (dSetupSize < (dIndex + wSize))			return ;
		if ((wTag == 0x4348) && (wSize == 1))		bHvCh = pbSetup[dIndex];
		dIndex += wSize;
	}

	if ((pbScript == NULL) || (bHvCh >= AV_MMF_HV_CHANNEL_NULL))
		return ;

	psLoad->sHV_Info.pbVoice		= pbVoice;
	psLoad->sHV_Info.dVoiceSize		= dVoiceSize;
	psLoad->sHV_Info.pbScript		= pbScript;
	psLoad->sHV_Info.dScriptSize	= dScriptSize;
	psLoad->sHV_Info.bHvChannel		= bHvCh;

	return ;
}

/*********************************************************************************
 *	Avdecode_byte3
 *
 *	Description:
 *			get 1 byte (from compressed data)
 *	Argument:
 *			nothing
 *	Return:
 *			unsigned char			success(read data)
 ********************************************************************************/
static unsigned short
_mmf_MalibMakeCRC(unsigned int dSize, unsigned char* pbData)
{
	unsigned short			wRes;
	unsigned char			bData;

	wRes  = 0xFFFFU;
	while ( --dSize >= 2 )
	{
		bData = *pbData++;
		wRes = (unsigned short)((wRes << 8) ^ g_crc_tbl[(unsigned char)(wRes >> 8) ^ bData]);
	}
	return (unsigned short)(~wRes & 0xFFFFU);
}


/*********************************************************************************
 *	_mmf_MALoad
 *
 *	Description:
 *			SMAF data load (error check and regist)
 *	Argument:
 *			pbFile			pointer to SMAF data
 *			dFsize			size of SMAF data
 *			dMode			load mode
 *	Return:
 *			0 or 1			success(file id)
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_MALoad(	unsigned char* pbFile, unsigned int dFSize)
{
	PLOADINFO		psLoad_Info;
	unsigned int		bNo = 0;
	unsigned int		dChunkID = 0, dChunkNo = 0;
	unsigned char*	pbBuf = NULL;
	unsigned int		dSize = 0, dIndex = 0;
	int				sdChunkSize = 0, sdResult = 0;
	unsigned int		dCalcCrc = 0, dFileCrc = 0;
	int				rVal = 0;

	pbBuf = pbFile;
	dSize = dFSize;
	psLoad_Info = &(g_sSmaf_Info.sLoad_Info[bNo]);
	_mmf_CheckInitial( psLoad_Info );

/* check File Chunk(ID/Size)	*/
	sdChunkSize = _mmf_MalibNextChunk(pbBuf, dSize, AVMALIB_CHUNK_PHASE_MMMD,
		&dChunkID, &dChunkNo);
	if ((sdChunkSize < 0) || (dChunkID != AVMALIB_CHUNKCODE_MMMD))
	{
		return AV_MMF_ERR_FILE;
	}
	dSize		= (unsigned int)(sdChunkSize + AV_MMF_CHUNK_HEADER_SIZE);
	dCalcCrc	= AV_MMF_CRC_NULL;

	dCalcCrc = _mmf_MalibMakeCRC(dSize, pbBuf);
	dFileCrc = (unsigned int)((((unsigned int)pbBuf[dSize - 2]) << 8) + pbBuf[dSize - 1]);
	if (dCalcCrc != dFileCrc) {
		return AV_MMF_ERR_FILE;
	}


/* check Contents Info Chunk				*/
	dIndex = AV_MMF_CHUNK_HEADER_SIZE;
    sdChunkSize = _mmf_MalibNextChunk(&pbBuf[dIndex], (dSize-dIndex),
		AVMALIB_CHUNK_PHASE_CNTI, &dChunkID, &dChunkNo);
	if ((sdChunkSize < 5) || (dChunkID != AVMALIB_CHUNKCODE_CNTI))
	{
		return AV_MMF_ERR_FILE;
	}

/* check Contents Class		*/
	if ((pbBuf[AV_MMF_POSITION_OF_CCLASS] != AV_MMF_CONTENTS_CLASS_0) &&
		(pbBuf[AV_MMF_POSITION_OF_CCLASS] != AV_MMF_CONTENTS_CLASS_1) &&
		(pbBuf[AV_MMF_POSITION_OF_CCLASS] != AV_MMF_CONTENTS_CLASS_2) )
	{
		return AV_MMF_ERR_CLASS;
	}

/* check Contents Type		*/
	dIndex += AV_MMF_CHUNK_HEADER_SIZE;
	if (((pbBuf[AV_MMF_POSITION_OF_CTYPE] & 0xF0) == AV_MMF_CONTENTS_TYPE_0) ||
	 ((pbBuf[AV_MMF_POSITION_OF_CTYPE] & 0xF0) == AV_MMF_CONTENTS_TYPE_1) ||
	 ((pbBuf[AV_MMF_POSITION_OF_CTYPE] & 0xF0) == AV_MMF_CONTENTS_TYPE_2) )
	{
		psLoad_Info->dSmafType = AV_MMF_SMAF_TYPE_MA2;
	}
	else if (((pbBuf[AV_MMF_POSITION_OF_CTYPE] & 0xF0) == AV_MMF_CONTENTS_TYPE_3) ||
		 ((pbBuf[AV_MMF_POSITION_OF_CTYPE] & 0xF0) == AV_MMF_CONTENTS_TYPE_4) ||
		 ((pbBuf[AV_MMF_POSITION_OF_CTYPE] & 0xF0) == AV_MMF_CONTENTS_TYPE_5) )
	{
		switch (pbBuf[AV_MMF_POSITION_OF_CTYPE] & 0x0F)
		{
		case 0x00 :
		case 0x01 :
			psLoad_Info->dSmafType = AV_MMF_SMAF_TYPE_MA2;
			break;
		case 0x02 :
		case 0x03 :
			psLoad_Info->dSmafType = AV_MMF_SMAF_TYPE_MA3;
			break;
		case 0x04 :
		case 0x05 :
		case 0x06 :
		case 0x07 :
		case 0x08 :
			psLoad_Info->dSmafType = AV_MMF_SMAF_TYPE_MA5;
			break;
		default :
			return AV_MMF_ERR_TYPE;
		}
	}
	else
	{
		return AV_MMF_ERR_TYPE;
	}

/* get pointer & size of option information	*/
	psLoad_Info->sOption_Info.pbCnti	= &pbBuf[dIndex];
	psLoad_Info->sOption_Info.dCntiSize	= (unsigned int)(sdChunkSize);
	dIndex += sdChunkSize;

	if (pbBuf[AV_MMF_POSITION_OF_CTYPE] >= 0x30)
	{
		sdChunkSize = _mmf_MalibNextChunk(&pbBuf[dIndex], (dSize-dIndex),
			AVMALIB_CHUNK_PHASE_MMMDSUB, &dChunkID, &dChunkNo);
		if ((sdChunkSize >= 12) && (dChunkID == AVMALIB_CHUNKCODE_OPDA))
		{
			dIndex += AV_MMF_CHUNK_HEADER_SIZE;
			psLoad_Info->sOption_Info.pbOpda		= &pbBuf[dIndex];
			psLoad_Info->sOption_Info.dOpdaSize	= (unsigned int)sdChunkSize;
			dIndex += sdChunkSize;
		}
	}

/* get Track Chunk information	*/
	while (dSize > (dIndex + AV_MMF_CHUNK_HEADER_SIZE + AV_MMF_FILE_CRC_SIZE))
	{
		sdChunkSize = _mmf_MalibNextChunk(&pbBuf[dIndex], (dSize-dIndex),
			AVMALIB_CHUNK_PHASE_MMMDSUB, &dChunkID, &dChunkNo);
		if (sdChunkSize < 0)
		{
			if (sdChunkSize == AVMALIB_CHUNK_ID_ERROR)
			{
				return AV_MMF_ERR_FILE;
			}
			else {
				return AV_MMF_ERR_SIZE;
			}
		}
		dIndex += AV_MMF_CHUNK_HEADER_SIZE;
		switch (dChunkID)
		{
		case AVMALIB_CHUNKCODE_MTR :
			if (dChunkNo > 6)
				break;
			psLoad_Info->sTrack_Info[dChunkNo].pbMtr	= &(pbBuf[dIndex]);
			psLoad_Info->sTrack_Info[dChunkNo].dMtrSize	= (unsigned int)sdChunkSize;
			break;
		case AVMALIB_CHUNKCODE_ATR :
			if (dChunkNo != 0)
				break;
			psLoad_Info->sTrack_Info[AV_MMF_ATR_TRACK_NO].pbMtr	= &(pbBuf[dIndex]);
			psLoad_Info->sTrack_Info[AV_MMF_ATR_TRACK_NO].dMtrSize	= (unsigned int)sdChunkSize;
			break;
		default :
			break;
		}
		dIndex += sdChunkSize;
	}



/* Error Check of Track Chunk	*/
	switch (psLoad_Info->dSmafType)
	{
	case AV_MMF_SMAF_TYPE_MA2 :
		sdResult = _mmf_TrackChunkCheck2(psLoad_Info);
		break;
	case AV_MMF_SMAF_TYPE_MA3 :
		sdResult = _mmf_TrackChunkCheck3(psLoad_Info);
		break;
	default :
		if (_mmf_CheckM5P(psLoad_Info)!= AV_MMF_FUNC_SUCCESS)	return AV_MMF_ERR_CHUNK;
		sdResult = _mmf_TrackChunkCheck5(psLoad_Info);
		break;
	}

/* check playback time			*/
	if (sdResult != AV_MMF_FUNC_SUCCESS)		return sdResult;

	#ifdef __MMFILE_TEST_MODE__
	debug_msg ( "SUM %ld\n", psLoad_Info->dPlayTime * psLoad_Info->dTimeBase);
	#endif

	if ((psLoad_Info->dPlayTime * psLoad_Info->dTimeBase) <= AV_MMF_PLAY_TIME_MIN)
	{
		return AV_MMF_ERR_SLENGTH;
	}
	if ((psLoad_Info->dPlayTime * psLoad_Info->dTimeBase) >= AV_MMF_PLAY_TIME_MAX)
	{
		return AV_MMF_ERR_LLENGTH;
	}
	rVal = psLoad_Info->dPlayTime*psLoad_Info->dTimeBase;
	_mmf_GetHvData(psLoad_Info, pbBuf[AV_MMF_POSITION_OF_CTYPE]);

	psLoad_Info->pbMmmd		= pbBuf;
	psLoad_Info->dMmmdSize	= dSize;
	psLoad_Info->dCrc		= (unsigned int)dCalcCrc;
	return rVal;
}


/*********************************************************************************
 *	_mmf_RenewalProfile
 *
 *	Description:
 *			renew profile data
 *	Argument:
 *			pbFile			pointer to SMAF data
 *	Return:
 *			0 or 1			success(file id)
 *			< 0				error code
 ********************************************************************************/
static int
_mmf_RenewalProfile(unsigned char* pbFile)
{
	PLOADINFO			psLoad;
	POPTIONINFO			psOptn;
	PTRACKINFO			psTrk;
	PHUFFMANINFO		psHuf;

	psLoad	= &(g_sSmaf_Info.sLoad_Info[1]);
	psOptn	= &(psLoad->sOption_Info);
	psTrk	= &(psLoad->sTrack_Info[5]);
	psHuf	= &(psLoad->sHuffman_Info);

/* renew pointer  offset to pointer			*/
	psLoad->pbMmmd		= pbFile;
	psLoad->dCrc		= AV_MMF_CRC_NULL;

	psOptn->pbCnti	= &(pbFile[(unsigned int)psOptn->pbCnti]);
	psOptn->pbOpda	= &(pbFile[(unsigned int)psOptn->pbOpda]);

	psTrk->pbMtr	= &(pbFile[(unsigned int)psTrk->pbMtr]);
	psTrk->pbMspi	= &(pbFile[(unsigned int)psTrk->pbMspi]);
	psTrk->pbMtsu	= &(pbFile[(unsigned int)psTrk->pbMtsu]);
	psTrk->pbMtsq	= &(pbFile[(unsigned int)psTrk->pbMtsq]);
	psTrk->pbMtsp	= &(pbFile[(unsigned int)psTrk->pbMtsp]);

/* Initialize Huffman information structure	*/
	psHuf->psBuffer		= psTrk->pbMtsq;
	psHuf->dMtsqSize	= psTrk->dMtsqSize;

	if (psTrk->pbMtr[0] == 0x01)
	{
		psHuf->dSeqSize = _mmf_DecodeInit(psHuf);
		if (psHuf->dSeqSize == AV_MMF_HUFFMAN_TREE_FAILURE)
		{
			return AV_MMF_FUNC_ERROR;
		}
	}
	psHuf->psFBuf	= psHuf->psBuffer;
	psHuf->sbFBit	= psHuf->sbBitC;
	psHuf->bFByte	= psHuf->bByte;

	return AV_MMF_FUNC_SUCCESS;
}






static int
_mmf_ParseSkipXmf2Mmf(unsigned char* pbFile, unsigned int dFSize)
{
	unsigned int skipVal = 0, sizeOfpbFile= dFSize;
	char cmpXmfCMMD[5];
	if (pbFile)
		memcpy(cmpXmfCMMD, pbFile, 4);
	else
	{
		debug_msg ( "NULL pointer!\n");
		return -1;
	}

	cmpXmfCMMD[4] = 0;

	if (strncmp(cmpXmfCMMD, "CMMD", 4) == 0)
	{
		while (1)
		{
			if (pbFile[skipVal] == 'M' && pbFile[skipVal+1] == 'M' && pbFile[skipVal+2] == 'M' && pbFile[skipVal+3] == 'D')
			{
				debug_msg ( "MMMD Header found!\n");
				break;
			}
			else
			{
				skipVal++;
				if (skipVal >= sizeOfpbFile)
				{
					debug_msg ( "MMMD Header is not found!\n");
					return -1;
				}
			}

		}
	}
	else
		debug_msg ( "File header is not started CMMD\n");

	debug_msg ( "skip value: %d\n", skipVal);

	return skipVal;
}

static int
mmf_file_mmf_get_duration (char *src, int is_xmf)
{
	int				readed = 0;
	int				xmf_skip_offset = 0;
	MMFileIOHandle	*fp = NULL;
	unsigned char	*buf = 0;
	long long		src_size = 0L;

	PLOADINFO		load_info;
	unsigned char	*p_crc = NULL;
	unsigned int		dCrc = 0;
	int				ret = 0;

	/*total time (millisecond)*/
	int	ret_msec = 0;

	debug_msg ( "\n");

	/*open*/
	ret = mmfile_open (&fp, src, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) 
     {
		debug_error ( "open failed.\n");
		return -1;
	}

	/*get file size*/
	mmfile_seek (fp, 0L, MMFILE_SEEK_END);
	src_size = mmfile_tell (fp);
	mmfile_seek (fp, 0L, MMFILE_SEEK_SET);

	if (src_size <= 0) {
		debug_msg ( "failed to get file size.\n");
		ret_msec = -1;
		goto _RELEASE_RESOURCE;
	}

	/*alloc work buffer*/
	buf = mmfile_malloc (src_size + 1);

	/*read data*/
	if ((readed = mmfile_read (fp, buf, src_size) ) <= 0 ) {
		debug_error ( "read error. size = %d\n", readed);

		ret_msec = -1;
		goto _RELEASE_RESOURCE;
	}

	/*if XMF, get skip offset.*/
	if (is_xmf) {
		xmf_skip_offset = _mmf_ParseSkipXmf2Mmf (buf, src_size);
		if (xmf_skip_offset == -1) {
			ret_msec = -1;
			goto _RELEASE_RESOURCE;
		}
	}

	if (g_sSmaf_Info.dStatus == AV_MMF_STATUS_SAT_PROFILE) {
		load_info = &(g_sSmaf_Info.sLoad_Info[1]);
		if (load_info->dMmmdSize <= src_size) {
			p_crc	= &(buf[load_info->dMmmdSize -2+xmf_skip_offset]);
			dCrc	= (unsigned int)((((unsigned int)p_crc[0]) << 8) + (unsigned int)p_crc[1]);
		} else {
			dCrc	= AV_MMF_CRC_NULL;
		}

		if (dCrc == load_info->dCrc) {
			if (_mmf_RenewalProfile (buf + xmf_skip_offset) == AV_MMF_FUNC_SUCCESS) {
				g_sSmaf_Info.dStatus = AV_MMF_STATUS_LOADED;
				ret_msec = -1;
				goto _RELEASE_RESOURCE;
			}
		}
	}

	ret_msec = _mmf_MALoad (buf + xmf_skip_offset, src_size);

_RELEASE_RESOURCE:

	mmfile_close (fp);

	if (buf) mmfile_free (buf);

	return ret_msec;
}

