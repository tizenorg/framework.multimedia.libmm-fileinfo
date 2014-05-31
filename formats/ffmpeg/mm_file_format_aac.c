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

#include <mm_error.h>

#include "mm_debug.h"
#include "mm_file_utils.h"
#include "mm_file_format_private.h"
#include "mm_file_format_aac.h"


// Internal Error Type
#define MMFILE_AAC_PARSER_FILE_END 2
  
// Media specific definations
#define MMFILE_AAC_ADIF_HEADER_MAX_SIZE 30
#define MMFILE_AAC_ADTS_HEADER_MAX_SIZE 7
#define AAC_ADTS_FRAME_LEN_OFFSET 30
#define AAC_ADTS_SAMPLES_PER_FRAME 1024

#define IS_AAC_ADIF_HEADER(buff) (!(memcmp((buff), "ADIF", 4)))
#define IS_AAC_ADTS_HEADER(buff) (((buff)[0] == 0xff) && (((buff)[1] & 0xf0) == 0xf0))


// Array to Number conversions
#define GET_INT_NUMBER(buff) (int)( (((int)(buff)[0]) << 24) | \
                                    (((int)(buff)[1]) << 16) | \
                                    (((int)(buff)[2]) << 8) | \
                                    (((int)(buff)[3])))

#define GET_SHORT_NUMBER(buff) (short)( ((short)(buff)[0] << 8) | \
                                        ((short)(buff)[1]) )
                                        
                                        
 
typedef enum _mmfile_aac_format_type {                                        
  AAC_FORMAT_ADIF,
  AAC_FORMAT_ADTS,
  AAC_FORMAT_UNKNOWN
}TAacFormatType;

typedef enum _mmfile_aac_bitstream_type {
  AAC_STREAM_CONSTANT,
  AAC_STREAM_VARIABLE
}TAacStreamType;

typedef enum _mmfile_aac_mpeg_type {
  AAC_MPEG_4,
  AAC_MPEG_2
}TAacMpegType;

typedef struct _mmfile_aac_handle {
  MMFileIOHandle*         hFile;
  AvFileContentInfo       id3Handle;
  unsigned int            streamOffset;
  unsigned int            tagOffset; 
  char                    isTagPresent;  
  unsigned int            tagInfoSize;
  unsigned char           tagVersion;
  TAacFormatType          formatType;
  TAacStreamType          streamType;
  TAacMpegType            mpegType;
  tMMFILE_AAC_STREAM_INFO streamInfo;
  tMMFILE_AAC_TAG_INFO    tagInfo;
}tMMFILE_AAC_HANDLE;


/*Index table for Sampling frequency */
const int Sampling_freq_table[16] = { 96000, 88200,  64000, 48000, 
                                      44100, 32000,  24000, 22050,
                                      16000, 12000,  11025, 8000,
                                      0,     0,      0,     0 };
                                    
/* internal APIs */                                    
void _aac_init_handle(tMMFILE_AAC_HANDLE* privateData);
int _search_id3tag(tMMFILE_AAC_HANDLE* pData);
int _parse_id3_tag(tMMFILE_AAC_HANDLE* pData);
int _get_range_bits_value (unsigned char* buff, int fieldOffset, int fieldSize);
int _parse_aac_adif_header (tMMFILE_AAC_HANDLE* pData);
int _get_next_adts_frame_length(tMMFILE_AAC_HANDLE* pData, int* frameLen);
int _parse_aac_adts_header(tMMFILE_AAC_HANDLE* pData);

                                    
void _aac_init_handle(tMMFILE_AAC_HANDLE* privateData)
{
  /* Default Initializations */
  privateData->streamOffset = 0;
  privateData->isTagPresent = FALSE;
  privateData->streamOffset = 0;
  privateData->tagOffset = 0;

  privateData->streamInfo.fileSize = 0;
  privateData->streamInfo.duration = 0; 
  privateData->streamInfo.bitRate = 0;
  privateData->streamInfo.samplingRate = 0;
  privateData->streamInfo.frameRate = 0;
  privateData->streamInfo.numAudioChannels = 0;
  privateData->streamInfo.numTracks = 1;
  privateData->streamInfo.profileType = 0;
    
  privateData->tagInfo.title = NULL;
  privateData->tagInfo.author = NULL;
  privateData->tagInfo.artist = NULL;
  privateData->tagInfo.album = NULL;
  privateData->tagInfo.album_artist = NULL;
  privateData->tagInfo.year = NULL;
  privateData->tagInfo.copyright = NULL;
  privateData->tagInfo.comment = NULL;
  privateData->tagInfo.genre = NULL;
  privateData->tagInfo.composer = NULL;
  privateData->tagInfo.classification = NULL;
  privateData->tagInfo.rating = NULL;
  privateData->tagInfo.recordDate = NULL;
  privateData->tagInfo.conductor = NULL;
  privateData->tagInfo.artwork = NULL;
  privateData->tagInfo.artworkSize = 0;
  privateData->tagInfo.artworkMime = NULL;
}


int _search_id3tag(tMMFILE_AAC_HANDLE* pData)
{
  unsigned char tagHeader[MP3_TAGv2_HEADER_LEN] = {0,};
  int encSize = 0;
  int readed = 0;
  
  mmfile_seek(pData->hFile, 0, MMFILE_SEEK_SET);
  readed = mmfile_read (pData->hFile, tagHeader, MP3_TAGv2_HEADER_LEN);
  if (MP3_TAGv2_HEADER_LEN != readed) {
#ifdef __MMFILE_TEST_MODE__
	debug_msg("Read Fail");
#endif
    return MMFILE_AAC_PARSER_FAIL;
  }
  
  if (!IS_ID3V2_TAG(tagHeader)) {
#ifdef __MMFILE_TEST_MODE__
	debug_msg("No ID3 Tag");
#endif
    goto search_end;
  }

  if (tagHeader[3] == 0xFF ||  tagHeader[4] == 0xFF ||
      tagHeader[6] >= 0x80 ||  tagHeader[7] >= 0x80 ||
	    tagHeader[8] >= 0x80 ||  tagHeader[9] >= 0x80) {
#ifdef __MMFILE_TEST_MODE__
	debug_msg("Read Fail");
#endif
    return MMFILE_AAC_PARSER_FAIL;
  }
      
  pData->tagVersion = tagHeader[3];

  if(pData->tagVersion > 4) {
#ifdef __MMFILE_TEST_MODE__
	debug_msg("\nTag version not supported");
#endif
    return MMFILE_AAC_PARSER_FAIL;
  }

  encSize = GET_INT_NUMBER(&tagHeader[6]);
  pData->tagInfoSize = MP3_TAGv2_HEADER_LEN;

  pData->tagInfoSize += (((encSize & 0x0000007F) >> 0) | ((encSize & 0x00007F00) >> 1) |  \
                         ((encSize & 0x007F0000) >> 2) | ((encSize & 0x7F000000) >> 3));                             

  if(pData->tagInfoSize > pData->streamInfo.fileSize) {
#ifdef __MMFILE_TEST_MODE__
	debug_msg("Invalid size");
#endif
    return MMFILE_AAC_PARSER_FAIL;
  }
            
  pData->isTagPresent = TRUE;
  pData->tagOffset = 0;
  pData->streamOffset = pData->tagInfoSize;
  
  /* Filling the information in id3Handle for tag parsing */
  pData->id3Handle.fileLen = pData->streamInfo.fileSize;
  pData->id3Handle.tagV2Info.tagLen = pData->tagInfoSize;
  pData->id3Handle.tagV2Info.tagVersion = pData->tagVersion;
  pData->id3Handle.tagV2Info.tagLen = pData->tagInfoSize;

search_end:
  return MMFILE_AAC_PARSER_SUCCESS;
}


int _parse_id3_tag(tMMFILE_AAC_HANDLE* pData)
{
  unsigned char* tagBuff = NULL;
  AvFileContentInfo* hTag = &pData->id3Handle;
  int ret = FALSE;
  int readed = 0;

  mmfile_seek(pData->hFile, pData->tagOffset, MMFILE_SEEK_SET);
  tagBuff = (unsigned char*) mmfile_malloc(hTag->fileLen);
  if(tagBuff == NULL) {
    ret = MMFILE_AAC_PARSER_FAIL;
    debug_error ("failed to memory allocation. %d\n", hTag->fileLen);
    goto failure;
  }

  readed = mmfile_read(pData->hFile, tagBuff, hTag->fileLen);
  if (readed != hTag->fileLen) {
    debug_error ("failed to read. %d, %lld\n", readed, hTag->fileLen);
    goto failure;
  }
 
  switch(hTag->tagV2Info.tagVersion) {
	case 1:
		ret = mm_file_id3tag_parse_v110(hTag, tagBuff);
		break;
	case 2:
		ret = mm_file_id3tag_parse_v222(hTag, tagBuff);
		break;
	case 3:
		ret = mm_file_id3tag_parse_v223(hTag, tagBuff);
		break;
	case 4:
		ret = mm_file_id3tag_parse_v224(hTag, tagBuff);
		break;
	default:
		debug_error ("Invalid Tag version [%d]\n", hTag->tagV2Info.tagVersion);
		break;
  }

  if(ret == FALSE) {
    ret = MMFILE_AAC_PARSER_FAIL;
    debug_warning ("failed to parse\n");
    goto failure;
  }

  mm_file_id3tag_restore_content_info(hTag);

  pData->tagInfo.title = hTag->pTitle;
  pData->tagInfo.author = hTag->pAuthor;
  pData->tagInfo.artist = hTag->pArtist;
  pData->tagInfo.album = hTag->pAlbum;
  pData->tagInfo.album_artist = hTag->pAlbum_Artist;
  pData->tagInfo.year = hTag->pYear;
  pData->tagInfo.copyright = hTag->pCopyright;
  pData->tagInfo.comment = hTag->pComment;
  pData->tagInfo.genre = hTag->pGenre;
  pData->tagInfo.tracknum = hTag->pTrackNum;
  pData->tagInfo.composer = hTag->pComposer;
  pData->tagInfo.classification = hTag->pContentGroup;
  pData->tagInfo.rating = hTag->pRating;
  pData->tagInfo.recordDate = hTag->pRecDate;  
  pData->tagInfo.conductor = hTag->pConductor;  
  pData->tagInfo.artworkMime = hTag->imageInfo.imageMIMEType;  
  pData->tagInfo.artworkSize = hTag->imageInfo.imageLen;
  pData->tagInfo.artwork = hTag->imageInfo.pImageBuf;

  ret = MMFILE_AAC_PARSER_SUCCESS;

  
failure:
  if(tagBuff) {
    mmfile_free(tagBuff);
    tagBuff = NULL;
  }
  
  return ret;
  
}


int _get_range_bits_value (unsigned char* buff, int fieldOffset, int fieldSize)
{
  int pos = 0;
  unsigned int srcByteStartOff = 0;
  unsigned int srcByteEndOff = 0;  
  unsigned int srcBitStartOff = 0;
  unsigned int srcBitEndOff = 0;  
  unsigned char dest[4] = {0,};
  unsigned int res = 0;
  unsigned int i,j, temp;
  unsigned char extraByteFlag = 0;
  unsigned int occupiedBytes = 0;
  unsigned char mask = 0, maskBit = 0x01;
  

  srcByteStartOff = (fieldOffset / 8);
  srcBitStartOff = (fieldOffset % 8);
  
  srcByteEndOff = ((fieldOffset + fieldSize - 1) / 8);
  srcBitEndOff =  ((fieldOffset + fieldSize - 1) % 8);
  
  occupiedBytes = srcByteEndOff - srcByteStartOff + 1;
  
  for(i = srcByteStartOff, j = 0; i <= srcByteEndOff && j <= 3; i++,j++) {
  	dest[j] = buff[i];
  }

  for(pos = 7; pos>= (char)srcBitStartOff; pos--) {
    mask = mask | maskBit;
	  maskBit <<= 1;
  }
  	
  dest[0] = dest[0] & mask;   
 
  if(i <= srcByteEndOff) {
  	extraByteFlag = 1;
  }
  
  res = GET_INT_NUMBER(dest);
   
  if(!extraByteFlag) {
    temp = (4 - occupiedBytes) * 8 + (7 - srcBitEndOff);
    res >>= temp;
  }
  
  if(extraByteFlag) {
  	res <<= srcBitStartOff;
  	temp = buff[srcByteEndOff] >> (7 - srcBitEndOff);
  	res = res | (unsigned int)temp;
  }
  
  return res;  
}


int _parse_aac_adif_header (tMMFILE_AAC_HANDLE* pData)
{
  unsigned char adifHeader[MMFILE_AAC_ADIF_HEADER_MAX_SIZE] = {0,};
  int currentBitOffset = 0;
  unsigned int fieldValue = 0;
  int copyRightStatus = 0;
  int readed = 0;

  mmfile_seek(pData->hFile, pData->streamOffset, MMFILE_SEEK_SET);
  readed = mmfile_read(pData->hFile, adifHeader, MMFILE_AAC_ADIF_HEADER_MAX_SIZE);
  if (readed < 0) {
    return MMFILE_AAC_PARSER_FAIL;
  }

  if(memcmp(adifHeader, "ADIF", 4) != 0) {
    return MMFILE_AAC_PARSER_FAIL;
  }
  currentBitOffset += 32;
  
  copyRightStatus = _get_range_bits_value(adifHeader, currentBitOffset, 1);  
  currentBitOffset += 1;

  if(copyRightStatus) {
    //skipping Copyright info
    currentBitOffset += 72;
  }
  
  //Original/copy
  fieldValue = _get_range_bits_value(adifHeader, currentBitOffset, 1);
  currentBitOffset += 1;
  
  //skipping Home status
  currentBitOffset += 1;
  
  //Bit stream type
  fieldValue = _get_range_bits_value(adifHeader, currentBitOffset, 1);
  currentBitOffset += 1;
  if(!fieldValue) {
    pData->streamType = AAC_STREAM_CONSTANT;
  }
  else {
    pData->streamType = AAC_STREAM_VARIABLE;
  }
    
  //Bit-rate
  pData->streamInfo.bitRate = _get_range_bits_value(adifHeader, currentBitOffset, 23);
  currentBitOffset += 23;
    
  //Num of program config elements
  fieldValue = _get_range_bits_value(adifHeader, currentBitOffset, 4);
  currentBitOffset += 4;
    
  //skipping adif buffer fullness
  currentBitOffset += 20;
  
  //skipping element instance tag
  currentBitOffset += 4;
  
  //Profile
  pData->streamInfo.profileType = _get_range_bits_value(adifHeader, currentBitOffset, 2);
  currentBitOffset += 2;
  
  //sampling freq index
  fieldValue = _get_range_bits_value(adifHeader, currentBitOffset, 4);
  currentBitOffset += 4;
  pData->streamInfo.samplingRate = Sampling_freq_table[fieldValue];
  
  //num_front_channel_elements
  pData->streamInfo.numAudioChannels = _get_range_bits_value(adifHeader, currentBitOffset, 4);
  currentBitOffset += 4;

  //num_side_channel_elements
  pData->streamInfo.numAudioChannels += _get_range_bits_value(adifHeader, currentBitOffset, 4);
  currentBitOffset += 4;

  //num_back_channel_elements
  pData->streamInfo.numAudioChannels += _get_range_bits_value(adifHeader, currentBitOffset, 4);
  currentBitOffset += 4;

  //num_lfe_channel_elements
  pData->streamInfo.numAudioChannels += _get_range_bits_value(adifHeader, currentBitOffset, 2);
  currentBitOffset += 2;

  return MMFILE_AAC_PARSER_SUCCESS;
  
}


int _parse_aac_adts_header(tMMFILE_AAC_HANDLE* pData)
{
  unsigned char adtsHeader[MMFILE_AAC_ADTS_HEADER_MAX_SIZE] = {0,};
  int currentBitOffset = 0;
  unsigned int fieldValue = 0;
  int readed = 0;

  mmfile_seek(pData->hFile, pData->streamOffset, MMFILE_SEEK_SET);
  readed = mmfile_read(pData->hFile, adtsHeader, MMFILE_AAC_ADTS_HEADER_MAX_SIZE);
  if (readed < 0) {
    return MMFILE_AAC_PARSER_FAIL;
  }

  if(!IS_AAC_ADTS_HEADER(adtsHeader)) {
    return MMFILE_AAC_PARSER_FAIL;
  }
  currentBitOffset += 12;
  
  //adtsId
  fieldValue = _get_range_bits_value(adtsHeader, currentBitOffset, 1);
  currentBitOffset += 1;
  pData->mpegType = (fieldValue != 0);
  
  //LayerType
  fieldValue = _get_range_bits_value(adtsHeader, currentBitOffset, 2);
  currentBitOffset += 2;
  
  //skipping Protection Absent
  currentBitOffset += 1;
  
  //ProfileType
  fieldValue = _get_range_bits_value(adtsHeader, currentBitOffset, 2);
  currentBitOffset += 2;
  pData->streamInfo.profileType = fieldValue;
  
  //SamplingrateIndex
  fieldValue = _get_range_bits_value(adtsHeader, currentBitOffset, 4);
  currentBitOffset += 4;
  pData->streamInfo.samplingRate = Sampling_freq_table[fieldValue];
  
  //skipping PrivateBit
  currentBitOffset += 1;
  
  //ChannelConfig
  pData->streamInfo.numAudioChannels = _get_range_bits_value(adtsHeader, currentBitOffset, 3);
  currentBitOffset += 3;
  
  //Original/copy status
  fieldValue = _get_range_bits_value(adtsHeader, currentBitOffset, 1);
  currentBitOffset += 1;
  
  //skipping Home status
  fieldValue = _get_range_bits_value(adtsHeader, currentBitOffset, 1);
  currentBitOffset += 1;
  
  //copy right Id status bit
  currentBitOffset += 1;
  
  return MMFILE_AAC_PARSER_SUCCESS;
}


int _get_next_adts_frame_length(tMMFILE_AAC_HANDLE* pData, int* frameLen)
{
  unsigned char adtsHeader[MMFILE_AAC_ADTS_HEADER_MAX_SIZE] = {0,};
  int ret = MMFILE_AAC_PARSER_SUCCESS;
  long long filePosBefore = mmfile_tell(pData->hFile);
  int readed = 0;

  readed = mmfile_read(pData->hFile, adtsHeader, MMFILE_AAC_ADTS_HEADER_MAX_SIZE);
  if (readed < 0)
	return MMFILE_AAC_PARSER_FAIL;
  
#ifdef __MMFILE_TEST_MODE__
  debug_msg("\nFILE POS: %lld\n", filePosBefore);
  debug_msg("\nADTS HEADER: [%2x] [%2x] [%2x] [%2x] [%2x] [%2x]\n",
               adtsHeader[0], adtsHeader[1], adtsHeader[2], adtsHeader[3], adtsHeader[4], adtsHeader[5]);
#endif
             
  if(mmfile_tell(pData->hFile) >= pData->streamInfo.fileSize) {
  	*frameLen = 0;
    ret = MMFILE_AAC_PARSER_FILE_END;  	
    goto function_end;
  }

  if(!IS_AAC_ADTS_HEADER(adtsHeader)) {
  	*frameLen = 0;
    ret = MMFILE_AAC_PARSER_FAIL;
    goto function_end;
  }
  
  *frameLen = _get_range_bits_value(adtsHeader, AAC_ADTS_FRAME_LEN_OFFSET, 13);

  if(*frameLen == 0 || *frameLen > (pData->streamInfo.fileSize - filePosBefore)) {
    *frameLen = 0;
  	ret = MMFILE_AAC_PARSER_FAIL;
  	goto function_end;
  }

function_end:
  
  mmfile_seek(pData->hFile, filePosBefore + *frameLen, MMFILE_SEEK_SET);
    
  return ret;
}


int mmfile_aacparser_open (MMFileAACHandle *handle, const char *filenamec)
{
  tMMFILE_AAC_HANDLE *privateData = NULL;
  int ret = 0;
  unsigned char header[4] = {0,};
  int firstFrameLen = 0;
  int readed = 0;
  
  if (NULL == filenamec) {
    debug_error ("file source is NULL\n");
    return MMFILE_AAC_PARSER_FAIL;
  }

  privateData = mmfile_malloc (sizeof(tMMFILE_AAC_HANDLE));
  if (NULL == privateData) {
    debug_error ("file source is NULL\n");
    return MMFILE_AAC_PARSER_FAIL;
  }

  ret = mmfile_open (&privateData->hFile, filenamec, MMFILE_RDONLY);
  if(ret == MMFILE_UTIL_FAIL) {    	
    debug_error ("error: mmfile_open\n");
    goto exception;        
  }

   /* Initialize the members of handle */
  _aac_init_handle(privateData);
 
  mmfile_seek (privateData->hFile, 0, MMFILE_SEEK_END);
  privateData->streamInfo.fileSize= mmfile_tell(privateData->hFile);

  mmfile_seek (privateData->hFile, 0, MMFILE_SEEK_SET);
 
  /* Search the existance of ID3 tag */
  ret = _search_id3tag(privateData);
  if(ret == MMFILE_AAC_PARSER_FAIL) {
  	debug_error("Error in searching the ID3 tag\n");
  	goto exception;
  }

  mmfile_seek (privateData->hFile, privateData->streamOffset, MMFILE_SEEK_SET);
  readed = mmfile_read (privateData->hFile, header, 4);
  if (readed != 4)
    goto exception;

  if(IS_AAC_ADIF_HEADER(header)) {
    privateData->formatType = AAC_FORMAT_ADIF;
    
#ifdef __MMFILE_TEST_MODE__    
	  debug_msg("AAC Format: ADIF\n");
#endif    
    
  }
  else if(IS_AAC_ADTS_HEADER(header)) {
    privateData->formatType = AAC_FORMAT_ADTS;
    
#ifdef __MMFILE_TEST_MODE__    
    debug_msg("AAC Format: ADTS\n");
#endif    
    
    /* Verify whether the first frame size is proper */
    mmfile_seek (privateData->hFile, privateData->streamOffset, MMFILE_SEEK_SET);
    ret = _get_next_adts_frame_length(privateData, &firstFrameLen);
    if(ret == MMFILE_AAC_PARSER_FAIL) {
      debug_error("Invalid Frame length in ADTS header\n");
      goto exception;
    }
  }
  else {
    privateData->formatType = AAC_FORMAT_UNKNOWN;
    debug_error("AAC Format: UNKNOWN\n");
    goto exception;
  }
  
  *handle = privateData;

  return MMFILE_AAC_PARSER_SUCCESS;

exception:
  if (privateData) { 
    mmfile_close (privateData->hFile);
    mmfile_free (privateData);
    *handle = NULL;
  }
  return MMFILE_AAC_PARSER_FAIL;
  
}


int mmfile_aacparser_get_stream_info (MMFileAACHandle handle, tMMFILE_AAC_STREAM_INFO *aacinfo)
{
  tMMFILE_AAC_HANDLE *privateData = NULL;
  int frameLen = 0;
  long long totalFrames = 0, totalFrameLength = 0;
  unsigned long long streamDataSize = 0;
  int ret = MMFILE_AAC_PARSER_SUCCESS;

  if (NULL == handle || NULL == aacinfo) {
    debug_error ("handle is NULL\n");
    return MMFILE_AAC_PARSER_FAIL;
  }

  privateData = (tMMFILE_AAC_HANDLE *) handle;
  
  if(privateData->formatType == AAC_FORMAT_ADIF) {
    ret = _parse_aac_adif_header(privateData);
    aacinfo->iseekable = 0;
  }
  else {
    ret = _parse_aac_adts_header(privateData);
    aacinfo->iseekable = 1;
  }
  
  if(ret == MMFILE_AAC_PARSER_FAIL) {
  	debug_error("Error in parsing the stream header\n");
    return ret;
  }
  
  mmfile_seek(privateData->hFile, privateData->streamOffset, MMFILE_SEEK_SET);
  
  if(privateData->formatType == AAC_FORMAT_ADTS) {

    while(TRUE) {
      ret = _get_next_adts_frame_length(privateData, &frameLen);
      if(ret != MMFILE_AAC_PARSER_SUCCESS) {
        break;
      }
      totalFrameLength += frameLen - MMFILE_AAC_ADTS_HEADER_MAX_SIZE;
      totalFrames++;
    }
    
    if(ret == MMFILE_AAC_PARSER_FAIL) {
      debug_error("Found corrupted frames!!! Ignoring\n");
    }

#ifdef __MMFILE_TEST_MODE__    
    debug_msg("No of ADTS frames: %d\n", totalFrames);
#endif    
    privateData->streamInfo.frameRate = privateData->streamInfo.samplingRate / AAC_ADTS_SAMPLES_PER_FRAME;
    
    if(privateData->streamInfo.frameRate)
      privateData->streamInfo.duration = (totalFrames * 1000) / privateData->streamInfo.frameRate;
    else privateData->streamInfo.duration = 0;
    
    if(privateData->streamInfo.duration)
      privateData->streamInfo.bitRate =  (totalFrameLength * 8 * 1000) / (privateData->streamInfo.duration);
    else privateData->streamInfo.bitRate = 0;
    
  }   
  else {
	streamDataSize = (unsigned long long)privateData->streamInfo.fileSize - privateData->tagInfoSize;
	privateData->streamInfo.duration = streamDataSize * 8 * 1000 / privateData->streamInfo.bitRate;
  }

  // Return the stream info structure
  memcpy(aacinfo, &(privateData->streamInfo), sizeof(tMMFILE_AAC_STREAM_INFO));
  
  return MMFILE_AAC_PARSER_SUCCESS;
}


int mmfile_aacparser_get_tag_info (MMFileAACHandle handle, tMMFILE_AAC_TAG_INFO *tagInfo)
{
	tMMFILE_AAC_HANDLE *privateData = NULL;
	int ret = 0;

	if (NULL == handle || NULL == tagInfo) {
		debug_error ("handle is NULL\n");
		return MMFILE_AAC_PARSER_FAIL;
	}

	privateData = (tMMFILE_AAC_HANDLE *) handle;
	if(privateData->id3Handle.tagV2Info.tagVersion == 0)
	{
		debug_warning ("There is no Tag info\n");
		return MMFILE_AAC_PARSER_SUCCESS;
	}

	ret = _parse_id3_tag(privateData);
	if(ret == MMFILE_AAC_PARSER_FAIL) {
		debug_warning ("Error in parsing the Tag info\n");
		return ret;
	}

	// Return the tag info structure
	memcpy(tagInfo, &(privateData->tagInfo), sizeof(tMMFILE_AAC_TAG_INFO));

	return MMFILE_AAC_PARSER_SUCCESS;
}


int mmfile_aacparser_close (MMFileAACHandle handle)
{
  tMMFILE_AAC_HANDLE *privateData = NULL;

  if (NULL == handle) {
    debug_error ("handle is NULL\n");
    return MMFILE_AAC_PARSER_FAIL;
  }

  privateData = (tMMFILE_AAC_HANDLE *) handle;
  mm_file_free_AvFileContentInfo(&privateData->id3Handle);
  
  mmfile_close(privateData->hFile);

  return MMFILE_AAC_PARSER_SUCCESS;
}
                                    
 
/* mm plugin interface */
int mmfile_format_read_stream_aac (MMFileFormatContext *formatContext);
int mmfile_format_read_frame_aac  (MMFileFormatContext *formatContext, unsigned int timestamp, MMFileFormatFrame *frame);
int mmfile_format_read_tag_aac    (MMFileFormatContext *formatContext);
int mmfile_format_close_aac       (MMFileFormatContext *formatContext);


EXPORT_API
int mmfile_format_open_aac (MMFileFormatContext *formatContext)
{
	MMFileAACHandle handle = NULL;
	int res = MMFILE_FORMAT_FAIL;

	if (NULL == formatContext || NULL == formatContext->uriFileName) {
		debug_error ("error: mmfile_format_open_aac\n");
		return MMFILE_FORMAT_FAIL;
	}

	if (formatContext->pre_checked == 0) {
		res = MMFileFormatIsValidAAC (formatContext->uriFileName);
		if (res == 0) {
			debug_error("It is not AAC file\n");
			return MMFILE_FORMAT_FAIL;        
		}
	}

	formatContext->ReadStream   = mmfile_format_read_stream_aac;
	formatContext->ReadFrame    = mmfile_format_read_frame_aac;
	formatContext->ReadTag      = mmfile_format_read_tag_aac;
	formatContext->Close        = mmfile_format_close_aac;

	formatContext->videoTotalTrackNum = 0;
	formatContext->audioTotalTrackNum = 1;

	res = mmfile_aacparser_open (&handle, formatContext->uriFileName);
	if (MMFILE_AAC_PARSER_FAIL == res) {
		debug_error ("mmfile_aacparser_open\n");
		return MMFILE_FORMAT_FAIL;
	}

	formatContext->privateFormatData = handle;

	return MMFILE_FORMAT_SUCCESS;
}

EXPORT_API
int mmfile_format_read_stream_aac (MMFileFormatContext *formatContext)
{
  MMFileAACHandle     handle = NULL;
  tMMFILE_AAC_STREAM_INFO  aacinfo = {0,};
  MMFileFormatStream  *audioStream = NULL;
    
  int ret = MMFILE_FORMAT_FAIL;

  if (NULL == formatContext ) {
    debug_error ("error: invalid params\n");
    ret = MMFILE_FORMAT_FAIL;
    goto exception;
  }

  handle = formatContext->privateFormatData;

  ret = mmfile_aacparser_get_stream_info (handle, &aacinfo);
  if (MMFILE_FORMAT_SUCCESS != ret) {
    debug_error ("error: mmfile_aacparser_get_stream_info\n");
    ret = MMFILE_FORMAT_FAIL;
    goto exception;
  }

  formatContext->isseekable = aacinfo.iseekable;
  formatContext->duration = aacinfo.duration;
  formatContext->videoStreamId = -1;
  formatContext->videoTotalTrackNum = 0;
  formatContext->audioTotalTrackNum = aacinfo.numTracks;
  formatContext->nbStreams = 1;

  audioStream = mmfile_malloc (sizeof(MMFileFormatStream));
  if (NULL == audioStream) {
    debug_error ("error: calloc_audiostream\n");
    ret = MMFILE_FORMAT_FAIL;
    goto exception;
  }

  audioStream->streamType = MMFILE_AUDIO_STREAM;
  audioStream->codecId = MM_AUDIO_CODEC_AAC;
  audioStream->bitRate = aacinfo.bitRate;
  audioStream->framePerSec = aacinfo.frameRate;
  audioStream->width = 0;
  audioStream->height = 0;
  audioStream->nbChannel = aacinfo.numAudioChannels;
  audioStream->samplePerSec = aacinfo.samplingRate;
  formatContext->streams[MMFILE_AUDIO_STREAM] = audioStream;
    
#ifdef  __MMFILE_TEST_MODE__
  mmfile_format_print_contents (formatContext);
#endif

  return MMFILE_FORMAT_SUCCESS;

exception:
    return ret;
}

EXPORT_API
int mmfile_format_read_tag_aac (MMFileFormatContext *formatContext)
{
  MMFileAACHandle     handle = NULL;  
  tMMFILE_AAC_TAG_INFO  aacinfo = {0,};
  int ret= MMFILE_FORMAT_FAIL;

  if (NULL == formatContext) {
    debug_error ("error: invalid params\n");
    ret = MMFILE_FORMAT_FAIL;
    goto exception;
  }
    
  handle = formatContext->privateFormatData;
  
  ret = mmfile_aacparser_get_tag_info (handle, &aacinfo);
  if (MMFILE_FORMAT_SUCCESS != ret) {
    debug_warning ("error: mmfile_aacparser_get_tag_info\n");
    ret = MMFILE_FORMAT_FAIL;
    goto exception;
  }

  if(aacinfo.title)
    formatContext->title = mmfile_strdup(aacinfo.title);
  if(aacinfo.author)
    formatContext->author = mmfile_strdup(aacinfo.author);
  if(aacinfo.artist)
    formatContext->artist = mmfile_strdup(aacinfo.artist);
  if(aacinfo.album)
    formatContext->album = mmfile_strdup(aacinfo.album);
  if(aacinfo.album_artist)
    formatContext->album_artist = mmfile_strdup(aacinfo.album_artist);
  if(aacinfo.year)
    formatContext->year = mmfile_strdup(aacinfo.year);
  if(aacinfo.copyright)
    formatContext->copyright = mmfile_strdup(aacinfo.copyright);
  if(aacinfo.comment)
    formatContext->comment = mmfile_strdup(aacinfo.comment);
  if(aacinfo.genre)
    formatContext->genre = mmfile_strdup(aacinfo.genre);
  if(aacinfo.tracknum)
    formatContext->tagTrackNum= mmfile_strdup(aacinfo.tracknum);
  if(aacinfo.composer)
    formatContext->composer = mmfile_strdup(aacinfo.composer);
  if(aacinfo.classification)
    formatContext->classification = mmfile_strdup(aacinfo.classification);
  if(aacinfo.rating)
    formatContext->rating = mmfile_strdup(aacinfo.rating);	/*not exist rating tag in id3*/
  if(aacinfo.conductor)
    formatContext->conductor = mmfile_strdup(aacinfo.conductor);
  if(aacinfo.artworkMime)
    formatContext->artworkMime = mmfile_strdup(aacinfo.artworkMime);
  if(aacinfo.artwork) {
    formatContext->artworkSize = aacinfo.artworkSize;
    formatContext->artwork = mmfile_malloc(aacinfo.artworkSize);
    if(formatContext->artwork == NULL) {
      ret = MMFILE_FORMAT_FAIL;
      goto exception;
    }
    memcpy(formatContext->artwork, aacinfo.artwork, aacinfo.artworkSize);
  }

#ifdef  __MMFILE_TEST_MODE__
  mmfile_format_print_contents (formatContext);
#endif

  return MMFILE_FORMAT_SUCCESS;

exception:
  return ret;
}


EXPORT_API
int mmfile_format_read_frame_aac (MMFileFormatContext *formatContext, 
                                  unsigned int timestamp, MMFileFormatFrame *frame)
{
  debug_error ("error: mmfile_format_read_frame_aac, no handling\n");

  return MMFILE_FORMAT_FAIL;
}


EXPORT_API
int mmfile_format_close_aac (MMFileFormatContext *formatContext)
{
  MMFileAACHandle  handle = NULL;
  int ret = MMFILE_FORMAT_FAIL;
   
  if (NULL == formatContext ) {
    debug_error ("error: invalid params\n");
    return MMFILE_FORMAT_FAIL;
  }
    
  handle = formatContext->privateFormatData;
   
  if(NULL != handle) {
    ret = mmfile_aacparser_close(handle);
    if(ret == MMFILE_AAC_PARSER_FAIL) {
      debug_error("error: mmfile_format_close_aac\n");
    }
  }
  
  if(formatContext->streams[MMFILE_AUDIO_STREAM]) {
    mmfile_free(formatContext->streams[MMFILE_AUDIO_STREAM]);
    formatContext->streams[MMFILE_AUDIO_STREAM] = NULL;
  }
  
  formatContext->ReadStream   = NULL;
  formatContext->ReadFrame    = NULL;
  formatContext->ReadTag      = NULL;
  formatContext->Close        = NULL;

  return MMFILE_FORMAT_SUCCESS;
}


