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

#ifndef _MM_FILE_FORMAT_AUDIO_H_
#define _MM_FILE_FORMAT_AUDIO_H_

#define MPEG_1_SIZE_LAYER_1		384
#define MPEG_1_SIZE_LAYER_2_3	1152

#define MPEG_2_SIZE_LAYER_1		(MPEG_1_SIZE_LAYER_1 / 2)
#define MPEG_2_SIZE_LAYER_2_3	(MPEG_1_SIZE_LAYER_2_3 / 2)

/* MP3 */
#define MP3TAGINFO_SIZE		128         /* file end 128 byte  */
#define FRAMES_FLAG			0x0001
#define BYTES_FLAG			0x0002
#define TOC_FLAG			0x0004
#define VBR_SCALE_FLAG		0x0008

#define FRAMES_AND_BYTES (FRAMES_FLAG | BYTES_FLAG)
#define VALID_SYNC(x) (((unsigned char *)(x))[0] == 0xFF && (((unsigned char *)(x))[1] & 0xE0) == 0xE0)

#define AV_MP3HDR_VERSION_OFS		1
#define AV_MP3HDR_VERSION_M			0x18
#define AV_MP3HDR_VERSION_SHIFT		3

#define AV_MP3HDR_LAYER_OFS			1
#define AV_MP3HDR_LAYER_M			0x06
#define AV_MP3HDR_LAYER_SHIFT		1

#define AV_MP3HDR_CRC_OFS			1
#define AV_MP3HDR_CRC_M				0x01
#define AV_MP3HDR_CRC_SHIFT			0

#define AV_MP3HDR_BITRATE_OFS		2
#define AV_MP3HDR_BITRATE_M			0xF0
#define AV_MP3HDR_BITRATE_SHIFT		4

#define AV_MP3HDR_SAMPLERATE_OFS	2
#define AV_MP3HDR_SAMPLERATE_M		0x0C
#define AV_MP3HDR_SAMPLERATE_SHIFT	2

#define AV_MP3HDR_PADDING_OFS		2
#define AV_MP3HDR_PADDING_M			0x02
#define AV_MP3HDR_PADDING_SHIFT		1

#define AV_MP3HDR_PRIVATE_OFS		2
#define AV_MP3HDR_PRIVATE_M			0x01
#define AV_MP3HDR_PRIVATE_SHIFT		0

#define AV_MP3HDR_CHANNEL_OFS		3
#define AV_MP3HDR_CHANNEL_M			0xC0
#define AV_MP3HDR_CHANNEL_SHIFT		6

#define AV_MP3HDR_CHANNEL_EXT_OFS	3
#define AV_MP3HDR_CHANNEL_EXT_M		0x30
#define AV_MP3HDR_CHANNEL_EXT_SHIFT	4

#define AV_MP3HDR_COPYRIGHT_OFS		3
#define AV_MP3HDR_COPYRIGHT_M		0x08
#define AV_MP3HDR_COPYRIGHT_SHIFT	3

#define AV_MP3HDR_ORIGINAL_OFS		3
#define AV_MP3HDR_ORIGINAL_M		0x06
#define AV_MP3HDR_ORIGINAL_SHIFT	2

#define AV_MP3HDR_EMPHASIS_OFS		3
#define AV_MP3HDR_EMPHASIS_M		0x03
#define AV_MP3HDR_EMPHASIS_SHIFT	0

#define MASK_MPEG		0x18	/* 00011000 */
#define MASK_MPEG_25	0x00	/* 00000000 */
#define MASK_MPEG_2		0x10	/* 00010000 */
#define MASK_MPEG_1		0x18	/* 00011000 */

#define MASK_LAYER		0x06	/* 00000110 */
#define MASK_LAYER_3	0x02	/* 00000010 */
#define MASK_LAYER_2	0x04	/* 00000100 */
#define MASK_LAYER_1	0x06	/* 00000110 */

#define MASK_CHANNEL	0xC0	/* 11000000 */
#define MASK_CHANNEL_ST	0x00	/* 00000000 */
#define MASK_CHANNEL_JS 0x40	/* 01000000 */
#define MASK_CHANNEL_DC	0x80	/* 10000000 */
#define MASK_CHANNEL_MN	0xC0	/* 11000000 */

#define MASK_SAMPLERATE	0x0C	/* 00001100 */

#define MASK_PADDING	0x02	/* 00000010 */

#define _AV_MP3_HEADER_POSITION_MAX		(50*1024) /* mp3 header should be exist inside this size */
#define AV_MP3_HEADER_READ_MAX			200000 /* mp3 header should be exist inside this size */
#define AV_WM_LOCALCODE_SIZE_MAX		2

/*
 *	Xing Header Information
 */
typedef struct {
	int hId;				/* from MPEG header, 0=MPEG2, 1=MPEG1 */
	int sampRate;			/* determined from MPEG header */
	int flags;				/* from Xing header data */
	int frames;				/* total bit stream frames from Xing header data */
	int bytes;				/* total bit stream bytes from Xing header data */
	int vbrScale;			/* encoded vbr scale from Xing header data */
	unsigned char *toc;		/* pointer to unsigned char toc_buffer[100] */
							/* may be NULL if toc not desired */
} AvXHeadData;

typedef struct {
	int hId;				/* from MPEG header, 0=MPEG2, 1=MPEG1 */
	int vID;				/* ver. ID */
	int sampRate;			/* determined from MPEG header */
	float delay;			/* delay */
	int qualityIndicator;	/* qualityIndicator */
	int bytes;				/* total bit stream bytes from Xing header data */
	int frames;				/* total bit stream frames from Xing header data */
	int numOfTOC;			/* numOfTOC */
	int vbriScale;			/* encoded vbri scale from VBRI header data */
	int sizePerTable;		/* encoded sizePerTable from VBRI header data */
	int framesPerTable;		/*encoded framesPerTable from VBRI header data */
	unsigned char *toc;		/* pointer to unsigned char toc_buffer[100] */
							/* may be NULL if toc not desired */
} AvVBRIHeadData;

typedef enum {

	AV_MPEG_VER_RESERVED,      /* Reserved                                      */
	AV_MPEG_VER_1,             /* MPEG Version 1.0                              */
	AV_MPEG_VER_2,             /* MPEG Version 2.0                              */
	AV_MPEG_VER_25,            /* MPEG Version 2.5                              */
	AV_MPEG_VER_4,             /* MPEG Version 4                              */
	AV_MPEG_VER_UNKNOWN        /* Unable to determine version information       */
} AvMp3VerEnumType;

typedef enum {
	AV_MP3_LAYER_RESERVED = 0,  /* Reserved                                    */
	AV_MPEG2_LAYER_AAC = AV_MP3_LAYER_RESERVED,  /* MPEG2 AAC compression     */
	AV_MP3_LAYER_1,             /* MPEG Layer 1 compression                    */
	AV_MP3_LAYER_2,             /* MPEG Layer 2 compression                    */
	AV_MP3_LAYER_3,             /* MPEG Layer 3 compression                    */
	AV_MP3_LAYER_UNKNOWN        /* Unable to determine layer information       */
} AvMpegLayerEnumType;

typedef enum {
	AV_MP3_BITRATE_FREE = 0,   /* Free bitrate (determined by software)        */
	AV_MP3_BITRATE_8K   = 8,   /* Fixed bitrates                               */
	AV_MP3_BITRATE_16K  = 16,  /*                                              */
	AV_MP3_BITRATE_24K  = 24,  /*                                              */
	AV_MP3_BITRATE_32K  = 32,  /*                                              */
	AV_MP3_BITRATE_40K  = 40,  /*                                              */
	AV_MP3_BITRATE_48K  = 48,  /*                                              */
	AV_MP3_BITRATE_56K  = 56,  /*                                              */
	AV_MP3_BITRATE_64K  = 64,  /*                                              */
	AV_MP3_BITRATE_80K  = 80,  /*                                              */
	AV_MP3_BITRATE_96K  = 96,  /*                                              */
	AV_MP3_BITRATE_112K = 112, /*                                              */
	AV_MP3_BITRATE_128K = 128, /*                                              */
	AV_MP3_BITRATE_144K = 144, /*                                              */
	AV_MP3_BITRATE_160K = 160, /*                                              */
	AV_MP3_BITRATE_176K = 176, /*                                              */
	AV__MP3_BITRATE_192K = 192, /*                                              */
	AV_MP3_BITRATE_224K = 224, /*                                              */
	AV_MP3_BITRATE_256K = 256, /*                                              */
	AV_MP3_BITRATE_288K = 288, /*                                              */
	AV_MP3_BITRATE_320K = 320, /*                                              */
	AV_MP3_BITRATE_352K = 352, /*                                              */
	AV_MP3_BITRATE_384K = 384, /*                                              */
	AV_MP3_BITRATE_416K = 416, /*                                              */
	AV_MP3_BITRATE_448K = 448, /*                                              */
	AV_MP3_BITRATE_VAR = 500,  /* Variable bitrate (Changes each frame)        */
	AV_MP3_BITRATE_UNK = 501   /* Unable to determine bitrate information      */
} AvMp3BitRateEnumType;

typedef enum {
	AV_SAMPLE_RATE_NONE,     /* Zero sampling rate, turn clocks off */

	AV_SAMPLE_RATE_8000 = 8000,      /* 8k      */
	AV_SAMPLE_RATE_11025 = 11025,    /* 11.025k */
	AV_SAMPLE_RATE_12000 = 12000,    /* 12k     */

	AV_SAMPLE_RATE_16000 = 16000,    /* 16k     */
	AV_SAMPLE_RATE_22050 = 22050,    /* 22.050k */
	AV_SAMPLE_RATE_24000 = 24000,    /* 24k     */

	AV_SAMPLE_RATE_32000 = 32000,    /* 32k     */
	AV_SAMPLE_RATE_44100 = 44100,    /* 44.1k   */
	AV_SAMPLE_RATE_48000 = 48000,    /* 48k     */

	AV_SAMPLE_RATE_64000 = 64000,    /* 64k     */
	AV_SAMPLE_RATE_88200 = 88200,    /* 88.2k   */
	AV_SAMPLE_RATE_96000 = 96000,    /* 96k     */

	AV_SAMPLE_RATE_MAX = 96001,      /* MAX     */
	AV_SAMPLE_RATE_UNKNOWN   = AV_SAMPLE_RATE_MAX /* Unknown rate */
} AvSampleRateType;

#endif /* _MM_FILE_FORMAT_AUDIO_H_ */
