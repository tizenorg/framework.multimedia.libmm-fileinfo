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
#include <stdio.h>
#include <stdlib.h>	/*malloc*/
#include <mm_error.h>

#include "mm_file_debug.h"
#include "mm_file_utils.h"
#include "mm_file_format_private.h"
#include "mm_file_format_midi.h"

/**
 * internal defines
 */
enum {
	AV_DEC_AUDIO_MIDI,
	AV_DEC_AUDIO_XMF,
	AV_DEC_AUDIO_RMF,
};

#define MMFILE_XMF_100		"XMF_1.00"
#define MMFILE_XMF_101		"XMF_1.01"
#define MMFILE_MXMF_200		"XMF_2.00"
#define MMFILE_RMF			"IREZ"

#define AvSMW_CNVID_MMF				(1)		/* SMAF/MA-1/MA-2/MA-3/MA-5 */
#define AvSMW_CNVID_PHR				(2)		/* SMAF/Phrase L1/L2 */
#define	AvSMW_CNVID_RMD				(3)		/* Realtime MIDI */
#define	AvSMW_CNVID_AUD				(4)		/* SMAF/Audio */
#define	AvSMW_CNVID_MID				(5)		/* SMF */
#define	AvSMW_CNVID_HVS				(9)		/* HV Script */
#define	AvSMW_CNVID_WAV				(11)	/* WAVE */

#define AvSMW_SUCCESS				(0)		/* success 								*/
#define AvSMW_ERROR					(-1)	/* error								*/
#define AvSMW_ERROR_ARGUMENT		(-2)	/* error of arguments					*/
#define AvSMW_ERROR_RESOURCE_OVER	(-3)	/* over specified resources				*/
#define AvSMW_ERROR_ID				(-4)	/* error id number 						*/
#define AvSMW_ERROR_TIMEOUT			(-5)	/* timeout		 						*/
#define AvSMW_ERROR_SOFTRESET		(-6)	/* error of soft reset for MA-5			*/

#define AvSMW_ERROR_FILE			(-16)	/* file error							*/
#define AvSMW_ERROR_CONTENTS_CLASS	(-17)	/* SMAF Contents Class shows can't play */
#define AvSMW_ERROR_CONTENTS_TYPE	(-18)	/* SMAF Contents Type shows can't play	*/
#define AvSMW_ERROR_CHUNK_SIZE		(-19)	/* illegal SAvF Chunk Size value		*/
#define AvSMW_ERROR_CHUNK			(-20)	/* illegal SAvF Track Chunk value		*/
#define AvSMW_ERROR_UNMATCHED_TAG	(-21)	/* unmathced specified TAG 				*/
#define AvSMW_ERROR_SHORT_LENGTH	(-22)	/* short sequence 						*/
#define AvSMW_ERROR_LONG_LENGTH		(-23)	/* long sequence 						*/
#define	AvSMW_ERROR_UNSUPPORTED		(-24)	/* unsupported format					*/
#define AvSMW_ERROR_NO_INFORMATION	(-25)	/* no specified information				*/
#define AvSMW_ERROR_HV_CONFLICT		(-26)	/* conflict about HV resource			*/

#define AvSMW_ERROR_SMF_FORMAT		(-50)	/* invalid format type != 0/1			*/
#define AvSMW_ERROR_SMF_TRACKNUM	(-51)	/* invalid number of tracks				*/
#define AvSMW_ERROR_SMF_TIMEUNIT	(-52)	/* invalid time unit					*/
#define AvSMW_ERROR_SMF_CMD			(-53)	/* invalid command byte					*/


#define SINT	signed int
#define SINT8	signed char
#define SINT16 	signed short
#define SINT32 	signed long
#define UINT	unsigned int
#define UINT8 	unsigned char
#define UINT16	unsigned short
#define UINT32 	unsigned long
#define ULONG 	unsigned long

/*--------------------------------------------------------------------------*/
/*   Defines                                                                */
/*--------------------------------------------------------------------------*/
#define	SMF_TIMEBASE_SHIFT			2							/*                         */
#define	SMF_TIMEBASE				(1L<<SMF_TIMEBASE_SHIFT)	/* [ms]                    */

#define	MAX_SMF_MESSAGES			256							/*                          */
#define	MAX_SMF_TRACKS				32							/* Should be <= 32          */
#define SMF_MAX_GAIN				76							/* - 6[dB] : 90             */
/* -18[dB] : 45             */
#define	MINIMUM_LENGTH				(20)

#define MELODY_MAP					(0)
#define DRUM_MAP					(1)
#define NUM_OF_MAPS					(2)


/*--------------------------------------------------------------------------*/
/*   Types                                                                  */
/*--------------------------------------------------------------------------*/
typedef struct _tagMidChInfo {
	UINT32					dBank;						/* BankH&L (0x00:00..0x7F7F)       */
	UINT32					dCurrBank;					/* BankH&L (0x00:00..0x7F7F)       */
	UINT32					dProg;						/* ProgramChange (0..127)          */
	UINT32					dVolume;					/* ChannelVolume (0..127)          */
	UINT32					dExpression;				/* Expression (0..127)             */
	UINT32					dModulation;				/* Modulation (0..127)             */
	UINT32					dPitchBend;					/* PitchBendH (0..127)             */
	UINT32					dBendRange;					/* CurrentBendRange (0..24)        */
	UINT32					dPreBendRange;				/* LatestBendRange (0..24)         */
	UINT32					dPanpot;					/* Panpot (0..127)                 */
	UINT32					dHold1;						/* Hold1 (0..127)                  */
	UINT32					dMode;						/* 0:MONO, 1:POLY                  */
	UINT32					dRPN;						/* RPN (0x00:00..0xFF7F)           */
	UINT32					dMipMute;					/* Mute switch (1:mute)            */
	UINT32					dKeyCon;					/* 0:Melady, 1:OFF, 2:ON           */
	UINT32					dLedSync;					/* 0:OFF, 1:ON                     */
	UINT32					dVibSync;					/* 0:OFF, 1:ON                     */
	UINT32					dFineTune;					/* 0..0x3FFF                       */
	UINT32					dCoaseTune;					/* 0..0x7F                         */
} MIDCHINFO, *PMIDCHINFO;

typedef struct _tagMIDPACKET {
	SINT32					sdDeltaTime;
	UINT32					dMsgID;
	UINT32					dP1;
	UINT32					dP2;
	UINT32					dP3;
} MIDPACKET, *PMIDPACKET;

typedef struct _tagTrack {
	UINT32					dSmfCmd;					/* CMD @ now                      */
	UINT32					dSize;						/* [byte] 0 measns nothing in it. */
	UINT8					*pbBase;						/* NULL measns nothing in it.     */
	UINT32					dOffset;					/* offset byte                    */
	SINT32					sdTicks;					/*                                */
} TRACKINFO, *PTRACKINFO;

typedef struct _tagOrderList {
	struct _tagOrderList	*pPrev;
	struct _tagOrderList	*pNext;
	UINT32					dTrack;
	UINT32					dTicks;
} ORDERLIST, *PORDERLIST;

typedef struct _tagMidInfo {
	UINT32					dTimeResolution;			/* 0..0x7fff                       */
	UINT8					*pbText;						/*                                 */
	UINT32					dSizeText;					/*                                 */
	UINT8					*pbTitle;					/*                                 */
	UINT32					dSizeTitle;					/*                                 */
	UINT8					*pbCopyright;				/*                                 */
	UINT32					dSizeCopyright;				/*                                 */
	UINT32					dNumOfTracks;				/* 1..32                           */
	UINT32					dSmfFormat;					/* 0..1                            */
	UINT32					dSetupBar;					/* 0:No, 1:Yes                     */
	UINT32					dStart;						/* Index after SetupBar            */
	UINT32					dVibNoteVoice;				/* 0:No, 1:Yes                     */

	SINT32					sdTotalTicks;				/* Total ticks                     */
	SINT32					sdDataEndTime;				/* (22.10)[ms]                     */
	SINT32					sdDelta;					/* (22.10)[ms]                     */

	UINT32					dEndFlag;					/*                                 */
	TRACKINFO				TrackInfo[MAX_SMF_TRACKS];

	struct _tagOrderList	*pTopOrderList;
	struct _tagOrderList	*pDoneOrderList;
	struct _tagOrderList	*pBottomOrderList;
	ORDERLIST				OrderList[MAX_SMF_TRACKS + 3];

	MIDCHINFO				ChInfo[16];					/*                                 */
	UINT32					dValid;						/* 0:none, 1:Valid                 */

	UINT8					bVoiceMap[NUM_OF_MAPS][128];/* 0:Empty, 1:Valid                */
} MIDINFO, *PMIDINFO;

typedef	struct _tagMidGlobals {
	SINT32		sdSeqID;						/* Sequence ID             */
	SINT32		sdFileID;						/* File ID                 */
	UINT32		dEnable;						/* 0:disable               */
	UINT32		dSetup;							/* 1: Just after seek      */

	UINT32		dRamBase;						/*                         */
	UINT32		dRamOffset;						/*                         */
	UINT32		dRamSize;						/*                         */

	MIDINFO		DataInfo[2];					/*                         */

	SINT32		sdSeekTime;						/* [ms]                    */
	SINT32		sdLastMsgTime;					/* (22.10)[ms]             */
	SINT32		sdSmfCurrentTicks;				/* Ticks @ now             */
	SINT32		sdSmfCurrentTime;				/* (22.10)[ms]             */
	SINT32		sdSmfDataEndTime;				/* (22.10)[ms]             */
	SINT32		sdSmfEndTime;					/* (22.10)[ms]             */
	SINT32		sdSmfDelta;						/* (22.10)[ms]             */

	UINT32		dMaxGain;						/* MaxGain (0..127)        */
	UINT32		dMasterVolume;					/* MsaterVolume (0..127)   */

	UINT32		dHoldMsgs;						/* Number of messages in Q */
	UINT32		dHoldPtrR;						/* Pointer for Read        */
	MIDPACKET	MsgBuffer[MAX_SMF_MESSAGES];	/* Message Q               */

	UINT32		dMuteFlag;						/* 0:Normal, 1:MUTE        */

	UINT32		dSyncNoteCh;					/* 0..15                   */
	UINT32		dSyncNoteKey;					/* 0..127, 255:OFF         */
	UINT32					dVibNote;			/* 0:No VibiNote, 1:Yes,VibNote    */

} MIDGLOBAL, *PMIDGLOBAL;


/*---------------------------------------------------------------------------*/
/*   Globals                                                                 */
/*---------------------------------------------------------------------------*/
static PMIDGLOBAL			gpMidInfo;
static PMIDINFO				gPi;


static SINT32	__AvMidFile_Initialize(void);
static void		__AvMidFile_Deinitialize(void);
static void		__AvMidInitializeOrderList(PMIDINFO pI);
static void		__AvMidSortOrderList(PMIDINFO pI);
static void		__AvMidInsertOrderList(PMIDINFO pI, UINT32 dTrack, SINT32 sdTicks);
static void		__AvMidRemoveFromOrderList(PMIDINFO pI);
static SINT32	__AvMidGetTrackTime(PMIDINFO pI, UINT32 dTrack);
static SINT32	__AvMidUpdateTrackTime(PMIDINFO pI, UINT32 dTrack);
static void		__AvMidResetTimeInfo(PMIDINFO pI);
static SINT32	__AvMidGetLeastTimeTrack(PMIDINFO pI);
static SINT32	__AvGetSizeOfFileInfo(PMIDINFO pI);
static SINT32	__AvCheckSizeOfMidFile(UINT8 *fp, UINT32 dFsize);
static int		__AvParseSkipXmf2Mid(UINT8 *pbFile, UINT32 dFSize);
static int		__AvGetMidiDuration(char *szFileName, MIDI_INFO_SIMPLE *info);


/* mm plugin interface */
int mmfile_format_read_stream_mid(MMFileFormatContext *formatContext);
int mmfile_format_read_frame_mid(MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int mmfile_format_read_tag_mid(MMFileFormatContext *formatContext);
int mmfile_format_close_mid(MMFileFormatContext *formatContext);

EXPORT_API
int mmfile_format_open_mid(MMFileFormatContext *formatContext)
{
	int res = MMFILE_FORMAT_FAIL;

	if (NULL == formatContext || NULL == formatContext->uriFileName) {
		debug_error("error: mmfile_format_open_mid\n");
		return MMFILE_FORMAT_FAIL;
	}

	if (formatContext->pre_checked == 0) {
		res = MMFileFormatIsValidMID(NULL, formatContext->uriFileName);
		if (res == 0) {
			debug_error("It is not MIDI file\n");
			return MMFILE_FORMAT_FAIL;
		}
	}

	formatContext->ReadStream   = mmfile_format_read_stream_mid;
	formatContext->ReadFrame    = mmfile_format_read_frame_mid;
	formatContext->ReadTag      = mmfile_format_read_tag_mid;
	formatContext->Close        = mmfile_format_close_mid;

	formatContext->videoTotalTrackNum = 0;
	formatContext->audioTotalTrackNum = 1;

	formatContext->privateFormatData = NULL;

	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_read_stream_mid(MMFileFormatContext *formatContext)
{
	MMFileFormatStream  *audioStream = NULL;
	int ret = MMFILE_FORMAT_FAIL;
	MIDI_INFO_SIMPLE *info = NULL;

	if (NULL == formatContext) {
		debug_error("error: invalid params\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	/*get infomation*/
	info = mmfile_format_get_midi_infomation(formatContext->uriFileName);
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
	audioStream->codecId = (info->is_xmf == 1) ? MM_AUDIO_CODEC_MXMF : MM_AUDIO_CODEC_MIDI;
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

	mmfile_format_free_midi_infomation(info);
	return MMFILE_FORMAT_SUCCESS;

exception:
	mmfile_format_free_midi_infomation(info);
	mmfile_free(audioStream);

	return ret;
}

EXPORT_API
int mmfile_format_read_tag_mid(MMFileFormatContext *formatContext)
{
	int ret = MMFILE_FORMAT_FAIL;
	MIDI_INFO_SIMPLE *info = NULL;
	const char *locale = MMFileUtilGetLocale(NULL);
	unsigned int tag_len;
	unsigned int cnv_len;

	if (NULL == formatContext) {
		debug_error("error: invalid params\n");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	/*get infomation*/
	info = mmfile_format_get_midi_infomation(formatContext->uriFileName);
	if (!info) {
		debug_error("failed to get infomation");
		ret = MMFILE_FORMAT_FAIL;
		goto exception;
	}

	/**
	 * UTF8 converting.
	 */
	if (info->title) {
		tag_len = strlen(info->title);
		cnv_len = 0;
		formatContext->title = mmfile_string_convert((const char *)info->title,
		                                             tag_len,
		                                             "UTF-8",
		                                             locale,
		                                             NULL,
		                                             (unsigned int *)&cnv_len);
		if (formatContext->title == NULL) {
			debug_warning("failed to UTF8 convert.\n");
			formatContext->title = mmfile_strdup(info->title);
		}
	}

	if (info->copyright) {
		tag_len = strlen(info->copyright);
		cnv_len = 0;
		formatContext->copyright = mmfile_string_convert((const char *)info->copyright,
		                                                 tag_len,
		                                                 "UTF-8",
		                                                 locale,
		                                                 NULL,
		                                                 (unsigned int *)&cnv_len);
		if (formatContext->copyright == NULL) {
			debug_warning("failed to UTF8 convert.\n");
			formatContext->copyright = mmfile_strdup(info->copyright);
		}
	}

	if (info->comment) {
		tag_len = strlen(info->comment);
		cnv_len = 0;
		formatContext->comment = mmfile_string_convert((const char *)info->comment,
		                                               tag_len,
		                                               "UTF-8",
		                                               locale,
		                                               NULL,
		                                               (unsigned int *)&cnv_len);
		if (formatContext->comment == NULL) {
			debug_warning("failed to UTF8 convert.\n");
			formatContext->comment = mmfile_strdup(info->comment);
		}
	}

#ifdef  __MMFILE_TEST_MODE__
	mmfile_format_print_contents(formatContext);
#endif

	mmfile_format_free_midi_infomation(info);
	return MMFILE_FORMAT_SUCCESS;

exception:
	mmfile_format_free_midi_infomation(info);
	return ret;
}

EXPORT_API
int mmfile_format_read_frame_mid(MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame)
{
	debug_error("error: mmfile_format_read_frame_midi, no handling\n");

	return MMFILE_FORMAT_FAIL;
}

EXPORT_API
int mmfile_format_close_mid(MMFileFormatContext *formatContext)
{
	if (NULL == formatContext) {
		debug_error("error: invalid params\n");
		return MMFILE_FORMAT_FAIL;
	}

	if (formatContext->streams[MMFILE_AUDIO_STREAM]) {
		mmfile_free(formatContext->streams[MMFILE_AUDIO_STREAM]);
		formatContext->streams[MMFILE_AUDIO_STREAM] = NULL;
	}

	formatContext->ReadStream   = NULL;
	formatContext->ReadFrame    = NULL;
	formatContext->ReadTag      = NULL;
	formatContext->Close        = NULL;

	return MMFILE_FORMAT_SUCCESS;
}

static char *_lc_strdup(const char *str, unsigned int size)
{
	char *t = NULL;
	t = mmfile_malloc(size + 1);
	if (t) {
		memset(t, 0x00, size + 1);
		memcpy(t, str, size);
	}
	return t;
}

MIDI_INFO_SIMPLE *
mmfile_format_get_midi_infomation(char *szFileName)
{
	/*int duration = -1;*/
	MIDI_INFO_SIMPLE *info = NULL;

	info = mmfile_malloc(sizeof(MIDI_INFO_SIMPLE));
	if (!info)
		return NULL;

	/*get infomation*/
	/*duration = */__AvGetMidiDuration(szFileName, info);

	return info;
}

void
mmfile_format_free_midi_infomation(MIDI_INFO_SIMPLE *info)
{
	if (info) {
		if (info->title) mmfile_free(info->title);
		if (info->copyright) mmfile_free(info->copyright);
		if (info->comment) mmfile_free(info->comment);
		mmfile_free(info);
	}
}



/**
 * internal midi parsing functions
 */

/****************************************************************************
 *	__AvGetMidiDuration(char* szFileName)
 *
 *	Desc.
 *		Load SMF data
 *	Param
 *		pbFile			pointer to the data
 *		dFileSize		size fo the data
 *		dMode			error check (0:No, 1:Yes, 2:Check, 3:OnlyInfo)
 *		pfnFunc			pointer of rhe callback function
 *		pvExtArgs		Reserved
 *	Return
 *		>= 0 : FileID, < 0 : Error code
 ****************************************************************************/
static int
__AvGetMidiDuration(char *szFileName, MIDI_INFO_SIMPLE *info)
{

	int xmfheaderSkip = 0;
	MMFileIOHandle *hFile = NULL;
	UINT8 *pbFile = NULL;
	UINT8 *pIMYbuf = NULL;
	SINT32 dFileSize;
	int	sdCurrentTime = 0;
	/* void* pvExtArgs = "mid"; */
	int readed = 0;
	int ret;
	int codecType = AV_DEC_AUDIO_MIDI;
	int is_xmf = 0;

	if (szFileName == NULL ||  info == NULL)
		return -1;

	/* debug_msg("URI: %s\n", szFileName); */
	/*open*/
	ret = mmfile_open(&hFile, szFileName, MMFILE_RDONLY);
	if (ret == MMFILE_UTIL_FAIL) {
		debug_error("open failed.\n");
		return -1;
	}

	/*get file size*/
	mmfile_seek(hFile, 0L, MMFILE_SEEK_END);
	dFileSize = mmfile_tell(hFile);
	mmfile_seek(hFile, 0L, MMFILE_SEEK_SET);

	if (dFileSize <= 0) {
		debug_error("failed to get file size.\n");
		goto _RELEASE_RESOURCE;
	}

	/*alloc read buffer*/
	pbFile = (UINT8 *) mmfile_malloc(sizeof(UINT8) * (dFileSize + 1));
	if (!pbFile) {
		debug_error("memory allocation failed.\n");
		goto _RELEASE_RESOURCE;
	}

	/*read data*/
	if ((readed = mmfile_read(hFile, pbFile, dFileSize)) != dFileSize) {
		debug_error("read error. size = %d\n", readed);
		goto _RELEASE_RESOURCE;
	}

	/*init global workspace*/
	if (__AvMidFile_Initialize())
		goto _RELEASE_RESOURCE;

	/*check format*/
	if (!(memcmp(pbFile, MMFILE_XMF_100, 8)) ||
	    !(memcmp(pbFile, MMFILE_XMF_101, 8)) ||
	    !(memcmp(pbFile, MMFILE_MXMF_200, 8))) {

		is_xmf = 1;
		codecType = AV_DEC_AUDIO_XMF;
	} else if (!(memcmp(pbFile, MMFILE_RMF, 4))) {
		is_xmf = 0;
		codecType = AV_DEC_AUDIO_RMF;
	} else {
		is_xmf = 0;
		codecType = AV_DEC_AUDIO_MIDI;
	}

	/*set output param*/
	if (codecType == AV_DEC_AUDIO_RMF) {

		info->duration = sdCurrentTime = 0;		/*not yet implemented.*/
		info->track_num = 1;					/*not yet implemented.*/
		info->is_xmf = is_xmf;

	} else {

		/*get duration. XMF/MIDI*/
		if (codecType ==  AV_DEC_AUDIO_XMF) {
			xmfheaderSkip = __AvParseSkipXmf2Mid(pbFile, dFileSize);
			if (xmfheaderSkip == -1)
				goto _RELEASE_RESOURCE;

			sdCurrentTime = __AvCheckSizeOfMidFile(pbFile + xmfheaderSkip, dFileSize);
		} else {
			sdCurrentTime = __AvCheckSizeOfMidFile(pbFile, dFileSize);
		}

		if (sdCurrentTime < 0) {
			debug_error("__AvGetMidiDuration: sdResult's error Code!(%d)\n", sdCurrentTime);
			goto _RELEASE_RESOURCE;
		}

		if (sdCurrentTime > 0)
			sdCurrentTime /= 1000;

		info->duration = sdCurrentTime;
		info->track_num = gPi->dNumOfTracks;
		info->is_xmf = is_xmf;

		info->title = _lc_strdup((const char *)gPi->pbTitle, gPi->dSizeTitle);
		info->copyright = _lc_strdup((const char *)gPi->pbCopyright, gPi->dSizeCopyright);
		info->comment =  _lc_strdup((const char *)gPi->pbText, gPi->dSizeText);
	}

_RELEASE_RESOURCE:

	/*resource release*/
	__AvMidFile_Deinitialize();
	mmfile_close(hFile);
	mmfile_free(pbFile);
	mmfile_free(pIMYbuf);

	return sdCurrentTime;
}

static SINT32
__AvMidFile_Initialize(void)
{
	gpMidInfo = mmfile_malloc(sizeof(MIDGLOBAL));

	if (!gpMidInfo)
		return (AvSMW_ERROR);

	memset(gpMidInfo, 0x00, sizeof(MIDGLOBAL));

	gpMidInfo->sdSeqID = -1;					/* Sequence ID      */
	gpMidInfo->sdFileID = -1;					/* File ID          */
	gpMidInfo->dEnable = 0;						/* 0:disabel        */
	gpMidInfo->DataInfo[0].dValid = 0;
	gpMidInfo->DataInfo[1].dValid = 0;

	return (AvSMW_SUCCESS);
}

static void
__AvMidFile_Deinitialize(void)
{
	mmfile_free(gpMidInfo);
}


/*---------------------------------------------------------------------------*/
/*   Functions (internal use only)                                           */
/*---------------------------------------------------------------------------*/

/****************************************************************************
 *	__AvMidInitializeOrderList(PMIDINFO pI)
 *
 *	Description:
 *			Initialize OrderList.
 *	Param:
 *		pI			... pointer to the data info
 *	Return:
 *			none
 ****************************************************************************/
static void
__AvMidInitializeOrderList(PMIDINFO pI)
{
	int ix2;

	for (ix2 = 1; ix2 <= MAX_SMF_TRACKS + 1; ix2++) {
		pI->OrderList[ix2].pPrev = &pI->OrderList[ix2 - 1];
		pI->OrderList[ix2].pNext = &pI->OrderList[ix2 + 1];
		pI->OrderList[ix2].dTrack = 0xFF;
		pI->OrderList[ix2].dTicks = 0xFFFFFFFFL;
	}
	pI->OrderList[0].pPrev = NULL;
	pI->OrderList[0].pNext = &pI->OrderList[1];
	pI->OrderList[MAX_SMF_TRACKS + 2].pPrev = &pI->OrderList[MAX_SMF_TRACKS + 1];
	pI->OrderList[MAX_SMF_TRACKS + 2].pNext = NULL;
	pI->pTopOrderList = &pI->OrderList[0];
	pI->pDoneOrderList = &pI->OrderList[1];
	pI->pBottomOrderList = &pI->OrderList[MAX_SMF_TRACKS + 2];
}


/****************************************************************************
 *	__AvMidSortOrderList(PMIDINFO pI)
 *
 *	Description:
 *			Sort OrderList. (Ascending order)
 *	Param:
 *		pI			... pointer to the data info
 *	Return:
 *			none
 ****************************************************************************/
static void
__AvMidSortOrderList(PMIDINFO pI)
{
	PORDERLIST pSlot;
	PORDERLIST pTerget;

	pSlot = (pI->pTopOrderList)->pNext;
	(pSlot->pPrev)->pNext = pSlot->pNext;
	(pSlot->pNext)->pPrev = pSlot->pPrev;
	pSlot->dTicks = ((UINT32)pI->TrackInfo[pSlot->dTrack].sdTicks << 5) + pSlot->dTrack;

	pTerget = pSlot->pNext;
	while (pTerget != pI->pDoneOrderList) {
		if (pSlot->dTicks <= pTerget->dTicks) break;
		pTerget = pTerget->pNext;
	}

	(pTerget->pPrev)->pNext = pSlot;
	pSlot->pPrev = pTerget->pPrev;
	pTerget->pPrev = pSlot;
	pSlot->pNext = pTerget;
}


/****************************************************************************
 *	__AvMidInsertOrderList(PMIDINFO pI, UINT32 dTrack, SINT32 sdTicks)
 *
 *	Description:
 *			Add item to the top of the list.
 *	Param:
 *		pI			... pointer to the data info
 *	Return:
 *			none
 ****************************************************************************/
static void
__AvMidInsertOrderList(PMIDINFO pI, UINT32 dTrack, SINT32 sdTicks)
{
	PORDERLIST pTerget;

	if (pI->dNumOfTracks == 1) return;
	if (((pI->dEndFlag >> dTrack) & 0x01) == 0) return;

	pTerget = pI->pDoneOrderList->pNext;
	if (pTerget == pI->pBottomOrderList) return;

	pI->pDoneOrderList->pNext = pTerget->pNext;
	(pTerget->pNext)->pPrev = pI->pDoneOrderList;

	pTerget->dTrack = dTrack;
	pTerget->dTicks = ((UINT32)sdTicks << 5) + dTrack;
	pTerget->pPrev = pI->pTopOrderList;
	pTerget->pNext = (pI->pTopOrderList)->pNext;
	((pI->pTopOrderList)->pNext)->pPrev = pTerget;
	(pI->pTopOrderList)->pNext = pTerget;

	__AvMidSortOrderList(pI);
}


/****************************************************************************
 *	__AvMidRemoveFromOrderList(PMIDINFO pI)
 *
 *	Description:
 *			delete Item from the top of the list.
 *	Param:
 *		pI			... pointer to the data info
 *	Return:
 *			none
 ****************************************************************************/
static void
__AvMidRemoveFromOrderList(PMIDINFO pI)
{
	PORDERLIST pSlot;
	PORDERLIST pTerget;

	pSlot = (pI->pTopOrderList)->pNext;
	(pSlot->pPrev)->pNext = pSlot->pNext;
	(pSlot->pNext)->pPrev = pSlot->pPrev;

	pTerget = pI->pBottomOrderList;
	(pTerget->pPrev)->pNext = pSlot;
	pSlot->pPrev = pTerget->pPrev;
	pTerget->pPrev = pSlot;
	pSlot->pNext = pTerget;
}


/****************************************************************************
 *	__AvMidGetTrackTime(PMIDINFO pI, UINT32 dTrack)
 *
 *	Description:
 *		Get the 1st DT from the list.
 *	Param:
 *		pI			... pointer to the data info
 *		bTrack		... #Track
 *	Return:
 *		0 : NoError, < 0 : Error code
 ****************************************************************************/
static SINT32
__AvMidGetTrackTime(PMIDINFO pI, UINT32 dTrack)
{
	UINT32		dTemp;
	SINT32		dTime;
	PTRACKINFO	pMt;

	if (((pI->dEndFlag >> dTrack) & 0x01) == 0) return (-1);

	pMt = &(pI->TrackInfo[dTrack]);

	dTime = 0;
	do {
		if (pMt->dOffset >= pMt->dSize) {
			pI->dEndFlag &= ~(1L << dTrack);
			return (-1);
		}
		dTemp = (UINT32)pMt->pbBase[pMt->dOffset++];
		dTime = (dTime << 7) + (dTemp & 0x7f);
	} while (dTemp >= 0x80);
	/*debug_msg("dTime is %d\n", dTime); */
	pMt->sdTicks += dTime;

	return (0);
}


/****************************************************************************
 *	__AvMidUpdateTrackTime(PMIDINFO pI, UINT32 dTrack)
 *
 *	Description:
 *		Update the 1st DT on the Track and OrderList
 *	Param:
 *		pI			... pointer to the data info
 *		bTrack		... #Track
 *	Return:
 *		0 : NoError, < 0 : Error code
 ****************************************************************************/
static SINT32
__AvMidUpdateTrackTime(PMIDINFO pI, UINT32 dTrack)
{
	UINT32		dTemp;
	SINT32		dTime;
	PTRACKINFO	pMt;

	if (pI->dNumOfTracks == 1) {
		/* Single track */
		if (((pI->dEndFlag >> dTrack) & 0x01) == 0) {
			return (-1);
		}

		pMt = &(pI->TrackInfo[dTrack]);

		dTime = 0;
		do {
			if (pMt->dOffset >= pMt->dSize) {
				pI->dEndFlag &= ~(1L << dTrack);
				return (-1);
			}
			dTemp = (UINT32)pMt->pbBase[pMt->dOffset++];
			dTime = (dTime << 7) + (dTemp & 0x7f);
		} while (dTemp >= 0x80);

		pMt->sdTicks += dTime;
	} else {
		/* Multi track */
		if (((pI->dEndFlag >> dTrack) & 0x01) == 0) {
			__AvMidRemoveFromOrderList(pI);
			return (-1);
		}

		pMt = &(pI->TrackInfo[dTrack]);

		dTime = 0;
		do {
			if (pMt->dOffset >= pMt->dSize) {
				pI->dEndFlag &= ~(1L << dTrack);
				__AvMidRemoveFromOrderList(pI);
				return (-1);
			}
			dTemp = (UINT32)pMt->pbBase[pMt->dOffset++];
			dTime = (dTime << 7) + (dTemp & 0x7f);
		} while (dTemp >= 0x80);

		pMt->sdTicks += dTime;
		__AvMidSortOrderList(pI);
	}

	return (0);
}


/****************************************************************************
 *	__AvMidResetTimeInfo(PMIDINFO pI)
 *
 *	Description:
 *		Reset time info
 *	Param:
 *		pI			... pointer to the data info
 *	Return:
 *		none
 ****************************************************************************/
static void
__AvMidResetTimeInfo(PMIDINFO pI)
{
	UINT32		sdTrack;
	PTRACKINFO	pMt;

	pI->dEndFlag = 0;

	for (sdTrack = 0; sdTrack < (UINT32)pI->dNumOfTracks; sdTrack++) {
		pMt = &(pI->TrackInfo[sdTrack]);

		pMt->dSmfCmd = 0;
		pMt->dOffset = 0;
		pMt->sdTicks = 0;
		if (pMt->dSize > 0) pI->dEndFlag |= (1L << sdTrack);
	}

	__AvMidInitializeOrderList(pI);

	if ((UINT32)pI->dNumOfTracks > MAX_SMF_TRACKS) {
		debug_error("__AvMidResetTimeInfo:  Num of tracks is over MAX track number. !!\n");
		return;
	}

	for (sdTrack = 0; sdTrack < (UINT32)pI->dNumOfTracks; sdTrack++) {
		__AvMidGetTrackTime(pI, (UINT32)sdTrack);
		pMt = &(pI->TrackInfo[sdTrack]);
		__AvMidInsertOrderList(pI, (UINT32)sdTrack, pMt->sdTicks);
	}
}


/****************************************************************************
 *	__AvMidGetLeastTimeTrack(PMIDINFO pI)
 *
 *	Description:
 *		Get the track@LeasetTime
 *	Param:
 *		pI			... pointer to the setup storage
 *	Return:
 *		0 : NoError, < 0 : Error
 ****************************************************************************/
static SINT32
__AvMidGetLeastTimeTrack(PMIDINFO pI)
{
	PORDERLIST	pTerget;

	pTerget = (pI->pTopOrderList)->pNext;
	if (pTerget == pI->pBottomOrderList) return (-1);

	return ((UINT32)pTerget->dTrack);
}


/****************************************************************************
 *	__AvGetSizeOfFileInfo(PMIDINFO pI)
 *
 *	Description:
 *		Get SMF info from the file
 *	Param:
 *		pI					... pointer to the setup storage
 *	Return:
 *		0 : NoError, < 0 : Error
 ****************************************************************************/
static SINT32
__AvGetSizeOfFileInfo(PMIDINFO pI)
{
	UINT32		dCmd;
	UINT32		dCmd2;
	UINT32		dSize;

	UINT32		dTemp;
	UINT32		dTime;
	UINT32		sdTotalTicks;
	SINT32		sdCurrentTime;
	SINT32		sdDelta;
	PMIDCHINFO 	pCh;
	UINT32		dCh;

	UINT32		dSetup;			/* bit0:beat@0, bit1:tempo@0, bit2:GmOn@0, bit3:tempo@1 */
	PTRACKINFO	pMt;
	SINT32		sdTr;

	static UINT32	dBank[16];
	static UINT32	dCurrBank[16];

	SINT32		sdNonConductorTime;
	SINT32		sdNonConductorTicks;
	UINT32		dConductorNote;
	dSetup = 0;
	sdTotalTicks = 0;
	sdCurrentTime = 0;
	sdNonConductorTime = 0x7FFFFFFF;
	sdNonConductorTicks = 0;
	dConductorNote = 0;
	sdDelta = (UINT32)(500 << 10) / pI->dTimeResolution;	/* default=0.5sec */

	pI->pbText = NULL;
	pI->dSizeText = 0;
	pI->pbTitle = NULL;
	pI->dSizeTitle = 0;
	pI->pbCopyright = NULL;
	pI->dSizeCopyright = 0;
	pI->dStart = 0;
	pI->dSetupBar = 0;
	pI->dVibNoteVoice = 0;

	for (dCh = 0; dCh < NUM_OF_MAPS; dCh++) {
		for (dTemp = 0; dTemp < 128; dTemp++) {
			pI->bVoiceMap[dCh][dTemp] = 0;
		}
	}
	pI->bVoiceMap[MELODY_MAP][0] = 1;						/* GM Default Piano */

	for (dCh = 0; dCh < 16; dCh++) {
		dBank[dCh] = 0;
		dCurrBank[dCh] = 0;
		pCh = &pI->ChInfo[dCh];
		pCh->dKeyCon = 0;
		pCh->dVibSync = 0;
		pCh->dLedSync = 0;
	}

	__AvMidResetTimeInfo(pI);

	if (pI->dSmfFormat != 0) dSetup |= 0x20;

	while (pI->dEndFlag != 0) {
		if ((pI->dEndFlag == 1) && (sdNonConductorTime == 0x7FFFFFFF)) {
			sdNonConductorTime = sdCurrentTime;
			sdNonConductorTicks = sdTotalTicks;
			dConductorNote |= 2;
		}

		if (pI->dNumOfTracks == 1) {
			sdTr = 0;
		} else {
			sdTr = __AvMidGetLeastTimeTrack(pI);
			if (sdTr < 0) break;
		}
		pMt = &(pI->TrackInfo[sdTr]);

		dTime = pMt->sdTicks - sdTotalTicks;
		sdCurrentTime += dTime * sdDelta;
		sdTotalTicks = pMt->sdTicks;
		if ((sdCurrentTime < 0) || (sdTotalTicks > 0x07FFFFFFL)) {
			return (AvSMW_ERROR_LONG_LENGTH);
		}

		dCmd = (UINT32)pMt->pbBase[pMt->dOffset++];

		if (dCmd < 0xf0) {
			/*--- MidiMsg ---*/
			if (dCmd < 0x80) {
				dCmd = pMt->dSmfCmd;
				if (dCmd < 0x80) return (AvSMW_ERROR_SMF_CMD);
				pMt->dOffset--;
			} else {
				pMt->dSmfCmd = dCmd;
			}

			dCh = dCmd & 0x0f;

			switch (dCmd & 0xf0) {
				case 0x90:	/* NoteOn */
					/* Conductor Track Note Check */
					if (sdTr == 0) dConductorNote |= 1;
					switch (dCurrBank[dCh] >> 8) {
						case 0x79:
							/* Melody */
							break;

						case 0x78:
							/* Drum */
							pI->bVoiceMap[DRUM_MAP][pMt->pbBase[pMt->dOffset] & 0x7F] = 1;
							break;

						default:
							if (dCh == 9) {
								/* Unknown: default GM Drum */
								pI->bVoiceMap[DRUM_MAP][pMt->pbBase[pMt->dOffset] & 0x7F] = 1;
							}
					}
					pMt->dOffset += 2;
					break;

				case 0xC0:	/* Program change */
					switch (dBank[dCh] >> 8) {
						case 0x79:
							if (dBank[dCh] != 0x7906) {
								/* Melody */
								pI->bVoiceMap[MELODY_MAP][pMt->pbBase[pMt->dOffset] & 0x7F] = 1;
							} else {
								/* Vibration Note */
								pI->dVibNoteVoice = 1;
							}
							break;

						case 0x78:
							/* Drum */
							break;

						default:
							/* default GM Melody */
							if (dCh != 9) {
								pI->bVoiceMap[MELODY_MAP][pMt->pbBase[pMt->dOffset] & 0x7F] = 1;
							}
					}

					dCurrBank[dCh] = dBank[dCh];
					pMt->dOffset++;
					break;

				case 0xD0:	/* Channel pressure */
					pMt->dOffset++;
					break;

				case 0xB0:	/* Control Change */
					switch (pMt->pbBase[pMt->dOffset]) {
						case 0x00:	/* Bank select(MSB) */
							dBank[dCh] = (dBank[dCh] & 0x00FF) | (pMt->pbBase[pMt->dOffset + 1] << 8);
							break;

						case 0x20:	/* Bank select (LSB) */
							dBank[dCh] = (dBank[dCh] & 0xFF00) | pMt->pbBase[pMt->dOffset + 1];
							break;
						default:
							break;
					}
					pMt->dOffset += 2;
					break;

				default:
					pMt->dOffset += 2;
					break;
			}
		} else {
			switch (dCmd) {
				case 0xF0:			/* SysEx */
				case 0xF7:			/* SysEx */
					pMt->dSmfCmd = 0;
					dSize = 0;
					do {
						dTemp = (UINT32)pMt->pbBase[pMt->dOffset++];
						dSize = (dSize << 7) + (dTemp & 0x7f);
					} while (dTemp >= 0x80);

					if ((dSize == 5) &&
					    (pMt->pbBase[pMt->dOffset] == 0x7e) &&
					    (pMt->pbBase[pMt->dOffset + 1] == 0x7f) &&
					    (pMt->pbBase[pMt->dOffset + 2] == 0x09) &&
					    (pMt->pbBase[pMt->dOffset + 3] == 0x01)) {
						/* System On */
						if (sdTotalTicks == 0) {
							dSetup |= 0x04;
						}
					} else {
						if (pI->dSetupBar == 0) {
							if ((dSize == 22) &&
							    (pMt->pbBase[pMt->dOffset] == 0x43) &&
							    (pMt->pbBase[pMt->dOffset + 1] == 0x79) &&
							    (pMt->pbBase[pMt->dOffset + 2] == 0x06) &&
							    (pMt->pbBase[pMt->dOffset + 3] == 0x7C) &&
							    (pMt->pbBase[pMt->dOffset + 4] == 0x02)) {
								/* Channel status */
								for (dCh = 0; dCh < 16; dCh++) {
									pCh = &pI->ChInfo[dCh];
									dTemp = pMt->pbBase[pMt->dOffset + 5 + dCh];
									pCh->dKeyCon = (dTemp >> 2) & 0x03;
									pCh->dVibSync = (dTemp >> 1) & 0x01;
									pCh->dLedSync = dTemp & 0x01;
								}
							}
						}
					}

					pMt->dOffset += dSize;
					break;

				case 0xF1:			/* System Msg */
				case 0xF3:			/* System Msg */
					pMt->dOffset++;
					break;

				case 0xF2:			/* System Msg */
					pMt->dOffset += 2;
					break;

				case 0xFF:											/* Meta          */
					dCmd2 = (UINT32)pMt->pbBase[pMt->dOffset++];	/* Meta Command  */
					dSize = 0;										/* Size          */
					do {
						dTemp = (UINT32)pMt->pbBase[pMt->dOffset++];
						dSize = (dSize << 7) + (dTemp & 0x7f);
					} while (dTemp >= 0x80);

					switch (dCmd2) {
						case 0x01:	/* Text */
							if (pI->pbText == NULL) {
								pI->pbText = &pMt->pbBase[pMt->dOffset];
								pI->dSizeText = dSize;
							}
							break;

						case 0x02:	/* Copyright */
							if (pI->pbCopyright == NULL) {
								pI->pbCopyright = &pMt->pbBase[pMt->dOffset];
								pI->dSizeCopyright = dSize;
							}
							break;

						case 0x06:	/* Title */
							if (pI->pbTitle == NULL) {
								pI->pbTitle = &pMt->pbBase[pMt->dOffset];
								pI->dSizeTitle = dSize;
							}
							break;

						case 0x2f:		/* End */
							pI->dEndFlag &= ~(1L << sdTr);
							break;

						case 0x51:		/* Set Tempo */
							switch (dSize) {
								case 3:
								case 4:
									dTime = ((UINT32)pMt->pbBase[pMt->dOffset] << 16) +
									        ((UINT32)pMt->pbBase[pMt->dOffset + 1] << 8) +
									        (UINT32)pMt->pbBase[pMt->dOffset + 2];
									if ((sdTotalTicks == 0) && (dTime == 250000)) dSetup |= 0x02;
									if (sdTotalTicks == (UINT32)pI->dTimeResolution) dSetup |= 0x08;

									/*<== I Think that Below Code is Trash!! and Erase it! (Actually I Don Know ^^)
									dTime = (dTime << 7) / 125; */

									sdDelta = (UINT32)(dTime / (pI->dTimeResolution > 0 ? pI->dTimeResolution : 1));
									break;

								default:
									break;
							}
							break;

						case 0x58:		/* Set TimeSignature */
							if ((sdTotalTicks == 0) &&
							    (pMt->pbBase[pMt->dOffset] == 1) &&
							    (pMt->pbBase[pMt->dOffset + 1] == 2)) dSetup |= 0x01;
							break;
						default:
							break;
					}
					pMt->dOffset += dSize;
					break;
				default:
					break;
			}
		}

		if ((UINT32)sdTr >= MAX_SMF_TRACKS) {
			debug_error("__AvGetSizeOfFileInfo:  Num of tracks is over MAX track number. !!\n");
			return AvSMW_ERROR_SMF_CMD;
		}
		__AvMidUpdateTrackTime(pI, (UINT32)sdTr);

		if (dSetup == 0x0F) {
			dSetup |= 0x10;
			sdCurrentTime = 0;
			pI->dSetupBar = 1;
			pI->dStart = pI->TrackInfo[0].dOffset;
		}
	}

	if ((dConductorNote != 2) || (pI->dSmfFormat == 0)) {
		pI->sdTotalTicks = sdTotalTicks;
		pI->sdDataEndTime = sdCurrentTime;
	} else {
		pI->sdTotalTicks = sdNonConductorTicks;
		pI->sdDataEndTime = sdNonConductorTime;
	}

	if (pI->dSetupBar == 0) {
		for (dCh = 0; dCh < 16; dCh++) {
			pCh = &pI->ChInfo[dCh];
			pCh->dKeyCon = 0;
			pCh->dVibSync = 0;
			pCh->dLedSync = 0;
		}
	}
	if ((pI->sdDataEndTime >> 10) <= MINIMUM_LENGTH) return (AvSMW_ERROR_SHORT_LENGTH);

	/* debug_msg("__AvGetSizeOfFileInfo/Done\n"); */

	return pI->sdDataEndTime;
}


/****************************************************************************
 *	__AvCheckSizeOfMidFile(UINT8* fp, UINT32 dFsize)
 *
 *	Description:
 *		Check SMF structure
 *	Param:
 *		fp			... pointer to the data
 *		dFsize		... size fo the data
 *		dMode		... error check (0:No, 1:Yes, 2:ErrorCheck, 3:CNTI)
 *	Return:
 *		0 : NoError, < 0 : Error
 ****************************************************************************/
static SINT32
__AvCheckSizeOfMidFile(UINT8 *src_fp, UINT32 dFsize)
{
	UINT32	dTemp;
	UINT32	dSize;
	PMIDINFO pI = NULL;
	UINT32	dFormat;
	UINT32	dNumOfTracks;
	UINT32	i;
	UINT8 *fp = src_fp;
	/* debug_msg("input param: %p, %d\n", fp, dFsize); */
	while (dFsize >= 22) {
		dTemp = ((UINT32)fp[0] << 24) + ((UINT32)fp[1] << 16) +
		        ((UINT32)fp[2] << 8) + (UINT32)fp[3];
		if (dTemp == 0x4D546864)	break;		/* 'MThd' */
		fp++;
		dFsize--;
	}

	/* debug_msg("__AvCheckSizeOfMidFile(): MThd Position is dFsize(%d)\n", dFsize); */

	if (dFsize < 22) {
		debug_error("__AvCheckSizeOfMidFile Error / Too small size\n");
		return (AvSMW_ERROR_FILE);
	}

	fp += 4;
	dFsize -= 4;

	/*--- Check size ----------------------------------------------------*/
	dTemp = ((UINT32)fp[0] << 24) + ((UINT32)fp[1] << 16) +
	        ((UINT32)fp[2] << 8) + (UINT32)fp[3];

	if (dTemp != 6) {
		debug_error("__AvCheckSizeOfMidFile Error / Size != 6\n");
		return (AvSMW_ERROR_CHUNK_SIZE);
	}

	fp += 4;
	dFsize -= 4;

	if (gpMidInfo->DataInfo[1].dValid == 1) return (AvSMW_ERROR);
	pI = &gpMidInfo->DataInfo[1];

	/**
	 * set global val
	 */

	/*--- Check format -------------------------------------------------*/
	dFormat = ((UINT32)fp[0] << 8) + (UINT32)fp[1];
	if (dFormat > 1) {
		debug_error("__AvCheckSizeOfMidFile Error/ Not Format 0 or 1\n");
		return (AvSMW_ERROR_SMF_FORMAT);
	}

	/*--- Check number of tracks ---------------------------------------*/
	dNumOfTracks = (SINT32)((UINT32)fp[2] << 8) + (UINT32)fp[3];
	if (dNumOfTracks == 0) {
		debug_error("__AvCheckSizeOfMidFile Error/ Number of Tracks = 0\n");
		return (AvSMW_ERROR_SMF_TRACKNUM);
	}
	if ((dFormat == 0) && (dNumOfTracks != 1)) {
		debug_error("__AvCheckSizeOfMidFile Error/ Number of Tracks > 1\n");
		return (AvSMW_ERROR_SMF_TRACKNUM);
	}

	if (dNumOfTracks > MAX_SMF_TRACKS) dNumOfTracks = MAX_SMF_TRACKS;
	pI->dNumOfTracks = (UINT8)dNumOfTracks;

	/*--- Check Time unit --------------------------------------------*/
	dTemp = ((UINT32)fp[4] << 8) + (UINT32)fp[5];
	pI->dTimeResolution = dTemp & 0x7fff;
	if (((dTemp & 0x8000) != 0) || (pI->dTimeResolution == 0)) {
		debug_error("__AvCheckSizeOfMidFile Error/ Unknown TimeUnit\n");
		return (AvSMW_ERROR_SMF_TIMEUNIT);
	}
	fp += 6;
	dFsize -= 6;

	for (i = 0; i < dNumOfTracks; i++) {
		/*--- Check chunk name --------------------------------------------*/
		while (dFsize >= 8) {
			dTemp = ((UINT32)fp[0] << 24) + ((UINT32)fp[1] << 16) +
			        ((UINT32)fp[2] << 8) + (UINT32)fp[3];
			if (dTemp == 0x4D54726B)	break;	/* 'MTrk' */
			fp++;
			dFsize--;
		}

		if (dFsize < 8) {
			debug_error("__AvCheckSizeOfMidFile Error/ Bad size\n");
			return (AvSMW_ERROR_CHUNK_SIZE);
		}

		/*--- Check size ----------------------------------------------------*/
		dSize = ((UINT32)fp[4] << 24) + ((UINT32)fp[5] << 16) +
		        ((UINT32)fp[6] << 8) + (UINT32)fp[7];

		if (dFsize < (dSize + 8)) {
			debug_error("__AvCheckSizeOfMidFile Error/ Bad size [%ld] vs [%ld]\n", dFsize, dSize + 22);
			return (AvSMW_ERROR_CHUNK_SIZE);
		}
		pI->TrackInfo[i].pbBase = &fp[8];
		pI->TrackInfo[i].dSize = dSize;
		fp += (dSize + 8);
		dFsize -= (dSize + 8);
	}
	pI->dSmfFormat = dFormat;

	/**
	 * set global
	 */
	gPi = pI;

	return (__AvGetSizeOfFileInfo(pI));
}

static int
__AvParseSkipXmf2Mid(UINT8 *pbFile, UINT32 dFSize)
{
	UINT32 skipVal = 0, sizeOfpbFile = dFSize;
	while (1) {
		if (pbFile[skipVal] == 'M' && pbFile[skipVal + 1] == 'T' && pbFile[skipVal + 2] == 'h' && pbFile[skipVal + 3] == 'd') {
#ifdef __MMFILE_TEST_MODE__
			debug_msg("__AvParseSkipForXMF : MThd Header found!\n");
#endif
			break;
		} else {
			skipVal++;
			if (skipVal >= sizeOfpbFile) {
				debug_error("__AvParseSkipForXMF : MThd Header is not found!\n");
				debug_error("__AvParseSkipForXMF :skipVal(%d) sizeOfpbFile(%d) \n", skipVal, sizeOfpbFile);
				return -1;
			}
		}
	}

	/* debug_msg("__AvParseSkipForXMF : skip value(%d)\n", skipVal); */

	return skipVal;
}

