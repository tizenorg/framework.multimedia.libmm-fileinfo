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

#ifndef __MM_FILE_PLUGIN_ID3TAG_H__
#define __MM_FILE_PLUGIN_ID3TAG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "mm_file_utils.h"

#define IS_ID3V2_TAG(x)	(((x))[0] == 0x49 && ((x))[1] == 0x44 && ((x))[2] == 0x33)
#define IS_INCLUDE_URL(x)	(((x))[0] == 0x2D && ((x))[1] == 0x2D && ((x))[2] == 0x3E)
#define IS_INCLUDE_URL_UTF16(x)	(((x))[0] == 0x2D && ((x))[1] == NULL && ((x))[2] == 0x2D && ((x))[3] == NULL && ((x))[4] == 0x3E && ((x))[5] == NULL)
#define IS_ENCODEDBY_UTF16(x)	(((x))[0] == 0xFF && ((x))[1] == 0xFE)
#define IS_ENCODEDBY_UTF16_R(x)	(((x))[0] == 0xFE && ((x))[1] == 0xFF)

#define NEWLINE_OF_UTF16(x)	(((x))[0] == 0xFF && ((x))[1] == 0xFE && ((x))[2] == 0x00 && ((x))[3] == 0x00)
#define NEWLINE_OF_UTF16_R(x)	(((x))[0] == 0xFE && ((x))[1] == 0xFF && ((x))[2] == 0x00 && ((x))[3] == 0x00)


#define AV_WM_LOCALCODE_SIZE_MAX		2
#define MP3_TAGv2_HEADER_LEN 10
#define MP3_TAGv2_23_TXT_HEADER_LEN 10
#define MP3_TAGv2_22_TXT_HEADER_LEN 6
#define TAGV1_SEEK_GAP 10


typedef enum {
	AV_ID3V2_PICTURE_TYPE_MIN,
	AV_ID3V2_PICTURE_TYPE_OTHER = AV_ID3V2_PICTURE_TYPE_MIN,
	AV_ID3V2_PICTURE_TYPE_PNG_ONLY_FILEICON,
	AV_ID3V2_PICTURE_TYPE_OTHER_FILEICON,
	AV_ID3V2_PICTURE_TYPE_FRONT_COVER,
	AV_ID3V2_PICTURE_TYPE_BACK_COVER,
	AV_ID3V2_PICTURE_TYPE_LEAFLET_PAGE,
	AV_ID3V2_PICTURE_TYPE_MEDIA_SIDEOFCD,
	AV_ID3V2_PICTURE_TYPE_LEAD_ARTIST,
	AV_ID3V2_PICTURE_TYPE_ARTIST_PERFORMER,
	AV_ID3V2_PICTURE_TYPE_CONDUCTOR,
	AV_ID3V2_PICTURE_TYPE_BAND_ORCHESTRA,
	AV_ID3V2_PICTURE_TYPE_COMPOSER,
	AV_ID3V2_PICTURE_TYPE_LYRICIST_TEXTWRITER,
	AV_ID3V2_PICTURE_TYPE_RECORDING_LOCATION,
	AV_ID3V2_PICTURE_TYPE_DURING_RECORDING,
	AV_ID3V2_PICTURE_TYPE_DURING_PERFORMANCE,
	AV_ID3V2_PICTURE_TYPE_MOVIE_VIDEO_SCREEN_CAPTURE,
	AV_ID3V2_PICTURE_TYPE_BRIGHT_COLOURED_FISH,
	AV_ID3V2_PICTURE_TYPE_ILLUSTRATION,
	AV_ID3V2_PICTURE_TYPE_BAND_ARTIST_LOGOTYPE,
	AV_ID3V2_PICTURE_TYPE_PUBLISHER_STUDIO_LOGOTYPE,

	AV_ID3V2_PICTURE_TYPE_MAX,
	AV_ID3V2_PICTURE_TYPE_UNKNOWN   = AV_ID3V2_PICTURE_TYPE_MAX /* Unknown picture type */

} AvID3v2PictureType;


#define MP3TAGINFO_SIZE							128         // file end 128 byte 
#define MP3_ID3_TITLE_LENGTH					30
#define MP3_ID3_ARTIST_LENGTH					30
#define MP3_ID3_ALBUM_LENGTH					30
#define MP3_ID3_YEAR_LENGTH						4
#define MP3_ID3_DESCRIPTION_LENGTH				30
#define MP3_ID3_GENRE_LENGTH					30

#define MP3_ID3_TRACKNUM_LENGTH					30
#define MP3_ID3_ENCBY_LENGTH					30
#define MP3_ID3_URL_LENGTH						100
#define MP3_ID3_FRAMEID_LENGTH					30
#define MP3_ID3_ORIGINARTIST_LENGTH				30
#define MP3_ID3_COMPOSER_LENGTH					100
#define MP3_ID3_IMAGE_DESCRIPTION_MAX_LENGTH	65
#define MP3_ID3_IMAGE_MIME_TYPE_MAX_LENGTH		31
#define MP3_ID3_IMAGE_EXT_MAX_LENGTH			4
#define TCC_FM_PATH_MOUNT_MMC					"/Mount/Mmc"



typedef enum {
	AV_ID3V2_ISO_8859 = 0,
	AV_ID3V2_UTF16,
	AV_ID3V2_UTF16_BE,
	AV_ID3V2_UTF8,
	AV_ID3V2_MAX
	
} AvID3v2EncodingType;


typedef struct{
	char	*pImageBuf;
	char	imageDescription[MP3_ID3_IMAGE_DESCRIPTION_MAX_LENGTH];
	char	imageMIMEType[MP3_ID3_IMAGE_MIME_TYPE_MAX_LENGTH];
	char	imageExt[MP3_ID3_IMAGE_EXT_MAX_LENGTH];
	int		pictureType;
	int		imageLen;
	int		imgDesLen;
	int 	imgMimetypeLen;
	bool	bURLInfo;
	
} AvTagVer2ImageInfo;

 typedef struct{
	int		tagLen;
	char	tagVersion;

	bool	bTagVer2Found;

	bool	bTitleMarked;
	bool	bArtistMarked;
	bool	bAlbumMarked;
	bool	bAlbum_ArtistMarked;
	bool	bYearMarked;
	bool	bDescriptionMarked;
	bool	bGenreMarked;

	bool	bTrackNumMarked;
	bool	bEncByMarked;
	bool	bURLMarked;
	bool	bCopyRightMarked;
	bool	bOriginArtistMarked;
	bool	bComposerMarked;
	bool	bImageMarked;
	bool	bRecDateMarked;
	bool	bContentGroupMarked;

	bool	bGenreUTF16;

} AvTagVer2AdditionalData;

 
typedef struct
{
	int		titleLen;
	int		artistLen;
	int		authorLen;
	int		copyrightLen;
	//int		descriptionLen;	/*ID3tag official tag name is "COMM" and meaning "Comment"*/
	int		commentLen;
	int		ratingLen;
	int		albumLen;
	int		yearLen;
	int		genreLen;
	int		tracknumLen;
	int		recdateLen;
	
// for PC Studio Podcast
	int 	contentGroupLen;
	
// for ID3V2 Tag
	int		encbyLen;
	int		urlLen;
	int		originartistLen;
	int		composerLen;

// To send resolution info to appl from OEM
	int 	width;                
	int 	height;
	
	unsigned int	bitRate;
	unsigned int	sampleRate;
	unsigned int	channels;
//	unsigned long	creationTime;       
	unsigned long	duration;

// for mp3 Info
	char			*pToc;			// VBR�϶� SeekPosition�� ���ϱ� ���� TOC ���̺��� ������ ��� �ִ� char �迭 , 100 ����Ʈ ����
	unsigned int	mpegVersion;	// 1 : mpeg 1,    2 : mpeg 2, 3 : mpeg2.5
	unsigned int	layer;			// 1 : layer1, 2 : layer2, 3 : layer3
	unsigned int	channelIndex;	// 0 : stereo, 1 : joint_stereo, 2 : dual_channel, 3 : mono
	unsigned int	objectType;
	unsigned int	headerType;
	long			fileLen;		// mp3 ������ ��ü ����
	long			headerPos;		// mp3 ����� ó������ ��Ÿ���� ��ġ
	long			datafileLen;	// ID3Tag���� �����ϰ� ���� mp3 frame���� ���� ,  VBR�϶� XHEADDATA �� bytes �� �ش��Ѵ�
	int				frameSize;		// mp3 frame �� ���� ũ��
	int				frameNum;		// mp3 ���Ͽ� �������� � ����ִ°�?
	bool			bVbr;			// VBR mp3?
	bool			bPadding;		// Padding?
	bool			bV1tagFound;

	char			*pTitle;		//Title/songname/
	char			*pArtist;		//Lead performer(s)/Soloist(s), 
	char			*pAuthor;		//Author
	char			*pCopyright;
	//char			*pDescription;	/*ID3tag official tag name is "COMM" and meaning "Comment"*/
	char			*pComment;
	char			*pRating;
	char			*pAlbum;		//Album/Movie/
	char			*pAlbum_Artist;
	char			*pYear;
	char			*pGenre; 
	char			*pTrackNum;		//Track number/Position in set
	char			*pRecDate; 		//Recording dates
	
// for PC Studio Podcast
	char			*pContentGroup;

// for ID3V2 Tag
	char			*pEncBy;				//Encoded by
	char			*pURL;					//User defined URL link frame for ID3V2 Tag
	char			*pOriginArtist;			//Original artist(s)/performer(s)
	char			*pComposer;				//Composer
	AvTagVer2ImageInfo			imageInfo;	//Album art   attached feature
	AvTagVer2AdditionalData		tagV2Info; //Needed data for ID3 tag parsing

// for DRM 2.0
	char			*pTransactionID;

//for ID3V1 Tag
	unsigned char	genre; 

} AvFileContentInfo;

typedef struct {
	int		videocodec;
	int		audiocodec;
	int 	width;
	int 	height;
} AvExtraInfo;

inline static void mm_file_free_AvFileContentInfo (AvFileContentInfo *pInfo)
{
	if (pInfo) {
		if (pInfo->pToc) mmfile_free (pInfo->pToc);
		if (pInfo->pTitle) mmfile_free (pInfo->pTitle);
		if (pInfo->pArtist) mmfile_free (pInfo->pArtist);
		if (pInfo->pAuthor) mmfile_free (pInfo->pAuthor);
		if (pInfo->pCopyright) mmfile_free (pInfo->pCopyright);
		//if (pInfo->pDescription) mmfile_free (pInfo->pDescription);
		if (pInfo->pComment) mmfile_free (pInfo->pComment);
		if (pInfo->pRating) mmfile_free (pInfo->pRating);
		if (pInfo->pAlbum) mmfile_free (pInfo->pAlbum);
		if (pInfo->pAlbum_Artist) mmfile_free (pInfo->pAlbum_Artist);
		if (pInfo->pYear) mmfile_free (pInfo->pYear);
		if (pInfo->pGenre) mmfile_free (pInfo->pGenre); 
		if (pInfo->pTrackNum) mmfile_free (pInfo->pTrackNum);
		if (pInfo->pRecDate) mmfile_free (pInfo->pRecDate);

		if (pInfo->pContentGroup) mmfile_free (pInfo->pContentGroup);

		if (pInfo->pEncBy) mmfile_free (pInfo->pEncBy);
		if (pInfo->pURL) mmfile_free (pInfo->pURL);
		if (pInfo->pOriginArtist) mmfile_free (pInfo->pOriginArtist);
		if (pInfo->pComposer) mmfile_free (pInfo->pComposer);

		if (pInfo->imageInfo.pImageBuf) mmfile_free (pInfo->imageInfo.pImageBuf);

		if (pInfo->pTransactionID) mmfile_free (pInfo->pTransactionID);
	}
}


bool mm_file_id3tag_parse_v110 (AvFileContentInfo* pInfo, unsigned char *buffer); //20050401 Condol : for MP3 content Info.
bool	mm_file_id3tag_parse_v222 (AvFileContentInfo* pInfo, unsigned char *buffer);
bool	mm_file_id3tag_parse_v223 (AvFileContentInfo* pInfo, unsigned char *buffer);
bool	mm_file_id3tag_parse_v224 (AvFileContentInfo* pInfo, unsigned char *buffer);
void mm_file_id3tag_restore_content_info (AvFileContentInfo* pInfo);

#ifdef __cplusplus
}
#endif

#endif /*__MM_FILE_PLUGIN_ID3TAG_H__*/

