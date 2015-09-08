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

#ifndef _MM_FILE_UTILS_H_
#define _MM_FILE_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>

#include "mm_file_formats.h"
#include "mm_file_codecs.h"

#define MMFILE_UTIL_FAIL		-1
#define MMFILE_UTIL_SUCCESS	0

#define MMFILE_IO_FAILED		MMFILE_UTIL_FAIL
#define MMFILE_IO_SUCCESS		MMFILE_UTIL_SUCCESS
#define CAST_MM_HANDLE(x)			(MMHandleType)(x)

#ifndef TRUE
#define TRUE	(1==1)
#endif
#ifndef FALSE
#define FALSE	(!TRUE)
#endif


////////////////////////////////////////////////////////////////////////
//                     ENDIAN UTIL API                                //
////////////////////////////////////////////////////////////////////////
inline unsigned int     mmfile_io_be_uint32 (unsigned int value);
inline unsigned int     mmfile_io_le_uint32 (unsigned int value);
inline int              mmfile_io_be_int32  (unsigned int value);
inline int              mmfile_io_le_int32  (unsigned int value);
inline unsigned short   mmfile_io_be_uint16 (unsigned short value);
inline unsigned short   mmfile_io_le_uint16 (unsigned short value);
inline short            mmfile_io_be_int16  (unsigned short value);
inline short            mmfile_io_le_int16  (unsigned short value);


////////////////////////////////////////////////////////////////////////
//                     FILE HEADER CHECK API                          //
////////////////////////////////////////////////////////////////////////
int MMFileFormatIsValidMP3 (const char *mmfileuri, int frameCnt);
int MMFileFormatIsValidAAC (const char *mmfileuri);
int MMFileFormatIsValidASF (const char *mmfileuri);
int MMFileFormatIsValidMP4 (const char *mmfileuri);
int MMFileFormatIsValidAVI (const char *mmfileuri);
int MMFileFormatIsValidAMR (const char *mmfileuri);
int MMFileFormatIsValidWAV (const char *mmfileuri);
int MMFileFormatIsValidMMF (const char *mmfileuri);
int MMFileFormatIsValidMID (const char *mmfileuri);
int MMFileFormatIsValidIMY (const char *mmfileuri);
int MMFileFormatIsValidWMA (const char *mmfileuri);
int MMFileFormatIsValidWMV (const char *mmfileuri);
int MMFileFormatIsValidOGG (const char *mmfileuri);
int MMFileFormatIsValidMatroska (const char *mmfileuri);
int MMFileFormatIsValidQT (const char *mmfileuri);
int MMFileFormatIsValidFLAC (const char *mmfileuri);
int MMFileFormatIsValidFLV (const char *mmfileuri);


////////////////////////////////////////////////////////////////////////
//                       IO HANDLER API                               //
////////////////////////////////////////////////////////////////////////
#define MMFILE_URI_MAX_LEN     512
#define MMFILE_FILE_URI        "file://"
#define MMFILE_FILE_URI_LEN    7
#define MMFILE_DRM_URI         "drm://"
#define MMFILE_DRM_URI_LEN     6
#define MMFILE_MEM_URI         "mem://"
#define MMFILE_MEM_URI_LEN     6
#define MMFILE_MMAP_URI        "mmap://"
#define MMFILE_MMAP_URI_LEN    7

#define MMFILE_RDONLY		O_RDONLY
#define MMFILE_WRONLY		O_WRONLY
#define MMFILE_RDWR			O_RDWR

#define MMFILE_SEEK_SET		SEEK_SET
#define MMFILE_SEEK_CUR		SEEK_CUR
#define MMFILE_SEEK_END		SEEK_END

typedef struct MMFileIOHandle
{
    struct MMFileIOFunc *iofunc;
    int     flags;         /* file flags */
    void   *privateData;
    char   *fileName;
} MMFileIOHandle;

typedef struct MMFileIOFunc
{
	const char	*handleName;
	int			(*mmfile_open) (MMFileIOHandle *h, const char *filename, int flags);
	int			(*mmfile_read) (MMFileIOHandle *h, unsigned char *buf, int size);
	int			(*mmfile_write)(MMFileIOHandle *h, unsigned char *buf, int size);
	long long	(*mmfile_seek) (MMFileIOHandle *h, long long pos, int whence);
	long long	(*mmfile_tell) (MMFileIOHandle *h);
	int			(*mmfile_close)(MMFileIOHandle *h);
	struct MMFileIOFunc *next;
} MMFileIOFunc;


int mmfile_register_io_func (MMFileIOFunc *iofunc);
int mmfile_register_io_all ();

int			mmfile_open (MMFileIOHandle **h, const char *filename, int flags);
int			mmfile_read (MMFileIOHandle *h, unsigned char *buf, int size);
int			mmfile_write(MMFileIOHandle *h, unsigned char *buf, int size);
long long	mmfile_seek (MMFileIOHandle *h, long long pos, int whence);
long long	mmfile_tell (MMFileIOHandle *h);
int			mmfile_close(MMFileIOHandle *h);



////////////////////////////////////////////////////////////////////////
//                            MIME  API                               //
////////////////////////////////////////////////////////////////////////
#define MMFILE_FILE_FMT_MAX_LEN 25
#define MMFILE_MIMETYPE_MAX_LEN 40
#define MMFILE_FILE_EXT_MAX_LEN 7

int mmfile_util_get_ffmpeg_format (const char *mime, char *ffmpegFormat);
int mmfile_util_get_file_ext (const char *mime, char *ext);



////////////////////////////////////////////////////////////////////////
//                            PRINT API                               //
////////////////////////////////////////////////////////////////////////
void mmfile_format_print_contents (MMFileFormatContext*in);
void mmfile_format_print_tags (MMFileFormatContext*in);
void mmfile_codec_print (MMFileCodecContext *in);
void mmfile_format_print_frame (MMFileFormatFrame *in);




////////////////////////////////////////////////////////////////////////
//                            STRING API                              //
////////////////////////////////////////////////////////////////////////
char **mmfile_strsplit (const char *string, const char *delimiter);
void mmfile_strfreev (char **str_array);
int  mmfile_util_wstrlen (unsigned short *wText);
short* mmfile_swap_2byte_string (short* mszOutput, short* mszInput, int length);
char *mmfile_get_charset(const char *str);
char *mmfile_string_convert (const char *str, unsigned int len,
                             const char *to_codeset, const char *from_codeset,
                             unsigned int *bytes_read,
                             unsigned int *bytes_written);
char *mmfile_strdup (const char *str);



////////////////////////////////////////////////////////////////////////
//                            LOCALE API                              //
////////////////////////////////////////////////////////////////////////
char *MMFileUtilGetLocale (int *error);




////////////////////////////////////////////////////////////////////////
//                            IMAGE API                               //
////////////////////////////////////////////////////////////////////////
typedef enum
{
    MMFILE_PIXEL_FORMAT_YUV420 = 0,
    MMFILE_PIXEL_FORMAT_YUV422 = 1,
    MMFILE_PIXEL_FORMAT_RGB565 = 2,
    MMFILE_PIXEL_FORMAT_RGB888 = 3,
    MMFILE_PIXEL_FORMAT_MAX,
} eMMFilePixelFormat;

int mmfile_util_image_convert (unsigned char *src, eMMFilePixelFormat src_fmt, int src_width, int src_height,
                               unsigned char *dst, eMMFilePixelFormat dst_fmt, int dst_width, int dst_height);



////////////////////////////////////////////////////////////////////////
//                            MEMORY API                              //
////////////////////////////////////////////////////////////////////////
void *mmfile_malloc (unsigned int size);
#define mmfile_free(ptr)  do { if((ptr)) { mmfile_free_r((ptr)); (ptr) = NULL;} } while (0)
void  mmfile_free_r (void *ptr);
void *mmfile_realloc(void *ptr, unsigned int size);
void *mmfile_memset (void *s, int c, unsigned int n);
void *mmfile_memcpy (void *dest, const void *src, unsigned int n);

////////////////////////////////////////////////////////////////////////
//                        DATA STRUCTURE API                          //
////////////////////////////////////////////////////////////////////////
typedef void* MMFileList;
MMFileList mmfile_list_alloc ();
MMFileList mmfile_list_append (MMFileList list, void* data);
MMFileList mmfile_list_prepend (MMFileList list, void* data);
MMFileList mmfile_list_find (MMFileList list, void* data);
MMFileList mmfile_list_first (MMFileList list);
MMFileList mmfile_list_last (MMFileList list);
MMFileList mmfile_list_nth (MMFileList list, unsigned int n);
MMFileList mmfile_list_next (MMFileList list);
MMFileList mmfile_list_previous (MMFileList list);
unsigned int mmfile_list_length (MMFileList list);
MMFileList mmfile_list_remove (MMFileList list, void *data);
MMFileList mmfile_list_remove_all (MMFileList list, void *data);
MMFileList mmfile_list_reverse (MMFileList list);
void mmfile_list_free (MMFileList list);



////////////////////////////////////////////////////////////////////////
//                            MEMORY DEBUG API                        //
////////////////////////////////////////////////////////////////////////
#ifdef __MMFILE_MEM_TRACE__
void *mmfile_malloc_debug (unsigned int size, const char *func, unsigned int line);
void *mmfile_calloc_debug (unsigned int nmemb, unsigned int size, const char *func, unsigned int line);
void  mmfile_free_debug (void *ptr, const char *func, unsigned int line);
void *mmfile_realloc_debug (void *ptr, unsigned int size, const char *func, unsigned int line);
void *mmfile_memset_debug (void *s, int c, unsigned int n, const char *func, unsigned int line);
void *mmfile_memcpy_debug (void *dest, const void *src, unsigned int n, const char *func, unsigned int line);

char *mmfile_string_convert_debug (const char *str, unsigned int len,
                                   const char *to_codeset, const char *from_codeset,
                                   int *bytes_read,
                                   int *bytes_written,
                                   const char *func, 
                                   unsigned int line);
char *mmfile_strdup_debug (const char *str, const char *func, unsigned int line);

#define mmfile_malloc(size)         mmfile_malloc_debug((size), __func__, __LINE__)
#define mmfile_calloc(size)         mmfile_calloc_debug((size), __func__, __LINE__)
#define mmfile_free(ptr)            do { mmfile_free_debug((ptr), __func__, __LINE__); (ptr) = NULL; } while (0)
#define mmfile_realloc(ptr, size)   mmfile_realloc_debug((ptr), (size), __func__, __LINE__)
#define mmfile_memset(ptr, c, n)    mmfile_memset_debug((ptr), (c), (n),  __func__, __LINE__)
#define mmfile_memcpy(dest, src, n) mmfile_memcpy_debug((ptr), (src), (n), __func__, __LINE__)

#define mmfile_string_convert(str,len,to_codeset,from_codeset,bytes_read,bytes_written) mmfile_string_convert_debug((str),(len),(to_codeset),(from_codeset),(bytes_read),(bytes_written), __func__,__LINE__)
#define mmfile_strdup(x)   mmfile_strdup_debug((x),__func__,__LINE__)    
    
#endif



////////////////////////////////////////////////////////////////////////
//                            TAG API                                 //
////////////////////////////////////////////////////////////////////////

#define MM_FILE_REVERSE_BYTE_ORDER_INT(i) ((int)((((i)&0xFF000000)>>24) | (((i)&0x00FF0000)>>8) | (((i)&0x0000FF00)<<8) | (((i)&0x000000FF)<<24)))
#define MM_FILE_REVERSE_BYTE_ORDER_SHORT(s) ((short)((((s)&0xFF00)>>8) | (((s)&0x00FF)<<8)))

#define _FREE_EX(ptr)               { if ((ptr)) {mmfile_free ((ptr)); (ptr) = NULL;} }
#define _STRNCPY_EX(dst,src,size)   { if ((size>0)) {strncpy((char*)(dst), (char*)(src),(size)); *((dst) + (size)) = '\0';}}

inline static int __AvMemstr (unsigned char* mem, unsigned char* str, int str_len, int search_range)
{
	int offset = 0;
	unsigned char *pSrc = mem;

	for (offset = 0; offset < search_range; offset++ )
	{
		pSrc = mem + offset;
		if (memcmp(pSrc, str, str_len) == 0)
			return offset;
	}

	return -1;
}

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
	char	*imageDescription;
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
	bool	bUnsyncLyricsMarked;
	bool	bSyncLyricsMarked;

	bool	bConductorMarked;

	bool	bGenreUTF16;

} AvTagVer2AdditionalData;

 
typedef struct
{
	int		titleLen;
	int		artistLen;
	int		authorLen;
	int		copyrightLen;
	int		descriptionLen;
	int		commentLen;
	int		ratingLen;
	int		albumLen;
	int		yearLen;
	int		genreLen;
	int		tracknumLen;
	int		recdateLen;
	int		conductorLen;
	int		album_artistLen;
	
// for PC Studio Podcast
	int 	contentGroupLen;
	
// for ID3V2 Tag
	int		encbyLen;
	int		urlLen;
	int		originartistLen;
	int		composerLen;
	int 		unsynclyricsLen;
	int	 	syncLyricsNum;

// To send resolution info to appl from OEM
	int 	width;                
	int 	height;
	
	unsigned int	bitRate;
	unsigned int	sampleRate;
	unsigned int	channels;
//	unsigned long	creationTime;       
	long long		duration;

// for mp3 Info
	char			*pToc;			// VBR�϶� SeekPosition�� ���ϱ� ���� TOC ���̺��� ������ ��\EF\BF?�ִ� char �迭 , 100 ����Ʈ ����
	unsigned int	mpegVersion;	// 1 : mpeg 1,    2 : mpeg 2, 3 : mpeg2.5
	unsigned int	layer;			// 1 : layer1, 2 : layer2, 3 : layer3
	unsigned int	channelIndex;	// 0 : stereo, 1 : joint_stereo, 2 : dual_channel, 3 : mono
	unsigned int	objectType;
	unsigned int	headerType;
	long long		fileLen;		// mp3 ������ ��ü ����
	long			headerPos;		// mp3 ����\EF\BF?ó������ ��Ÿ���� ��ġ
	long long		datafileLen;	// ID3Tag���� �����ϰ� ���� mp3 frame���� ���� ,  VBR�϶� XHEADDATA �� bytes �� �ش��Ѵ�
	int				frameSize;		// mp3 frame �� ���� ũ��
	int				frameNum;		// mp3 ���Ͽ� �������� � ����ִ°\EF\BF?
	bool			bVbr;			// VBR mp3?
	bool			bPadding;		// Padding?
	bool			bV1tagFound;

	char			*pTitle;		//Title/songname/
	char			*pArtist;		//Lead performer(s)/Soloist(s), 
	char			*pAuthor;		//Author
	char			*pCopyright;
	char			*pDescription;
	char			*pComment;
	char			*pRating;
	char			*pAlbum;		//Album/Movie/
	char			*pAlbum_Artist;
	char			*pYear;
	char			*pGenre; 
	char			*pTrackNum;		//Track number/Position in set
	char			*pRecDate; 		//Recording dates
	
	char			*pConductor;		/*[#TPE3 Conductor/performer refinement], ADDED: 2010-01-xx*/
	
// for PC Studio Podcast
	char			*pContentGroup;

// for ID3V2 Tag
	char			*pEncBy;				//Encoded by
	char			*pURL;					//User defined URL link frame for ID3V2 Tag
	char			*pOriginArtist;			//Original artist(s)/performer(s)
	char			*pComposer;				//Composer
	char			*pUnsyncLyrics;			//Unsychronised lyrics/text transcription
	GList		*pSyncLyrics;				//Sychronised lyrics/text
	
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

typedef struct {
	unsigned long 	time_info;
	char 			*lyric_info;
}AvSynclyricsInfo;

void mm_file_free_synclyrics_list(GList * synclyrics_list);

inline static void mm_file_free_AvFileContentInfo (AvFileContentInfo *pInfo)
{
	if (pInfo) {
		if (pInfo->pToc) mmfile_free (pInfo->pToc);
		if (pInfo->pTitle) mmfile_free (pInfo->pTitle);
		if (pInfo->pArtist) mmfile_free (pInfo->pArtist);
		if (pInfo->pAuthor) mmfile_free (pInfo->pAuthor);
		if (pInfo->pCopyright) mmfile_free (pInfo->pCopyright);
		if (pInfo->pDescription) mmfile_free (pInfo->pDescription);
		if (pInfo->pComment) mmfile_free (pInfo->pComment);
		if (pInfo->pRating) mmfile_free (pInfo->pRating);
		if (pInfo->pAlbum) mmfile_free (pInfo->pAlbum);
		if (pInfo->pAlbum_Artist) mmfile_free (pInfo->pAlbum_Artist);
		if (pInfo->pYear) mmfile_free (pInfo->pYear);
		if (pInfo->pGenre) mmfile_free (pInfo->pGenre); 
		if (pInfo->pTrackNum) mmfile_free (pInfo->pTrackNum);
		if (pInfo->pRecDate) mmfile_free (pInfo->pRecDate);
		if (pInfo->pConductor) mmfile_free (pInfo->pConductor);
		if (pInfo->pContentGroup) mmfile_free (pInfo->pContentGroup);
		if (pInfo->pEncBy) mmfile_free (pInfo->pEncBy);
		if (pInfo->pURL) mmfile_free (pInfo->pURL);
		if (pInfo->pOriginArtist) mmfile_free (pInfo->pOriginArtist);
		if (pInfo->pComposer) mmfile_free (pInfo->pComposer);
		if (pInfo->pUnsyncLyrics) mmfile_free (pInfo->pUnsyncLyrics);
		if (pInfo->imageInfo.pImageBuf) mmfile_free (pInfo->imageInfo.pImageBuf);
		if (pInfo->imageInfo.imageDescription) mmfile_free (pInfo->imageInfo.imageDescription);
		if (strlen(pInfo->imageInfo.imageMIMEType)>0) memset(pInfo->imageInfo.imageMIMEType, 0, MP3_ID3_IMAGE_MIME_TYPE_MAX_LENGTH);
		if (pInfo->pTransactionID) mmfile_free (pInfo->pTransactionID);

	}
}


bool	mm_file_id3tag_parse_v110 (AvFileContentInfo* pInfo, unsigned char *buffer); //20050401 Condol : for MP3 content Info.
bool	mm_file_id3tag_parse_v222 (AvFileContentInfo* pInfo, unsigned char *buffer);
bool	mm_file_id3tag_parse_v223 (AvFileContentInfo* pInfo, unsigned char *buffer);
bool	mm_file_id3tag_parse_v224 (AvFileContentInfo* pInfo, unsigned char *buffer);
void	mm_file_id3tag_restore_content_info (AvFileContentInfo* pInfo);
int		MMFileUtilGetMetaDataFromMP4 (MMFileFormatContext *formatContext);


#ifdef __cplusplus
}
#endif

#endif /*_MMFILE_UTILS_H_*/

