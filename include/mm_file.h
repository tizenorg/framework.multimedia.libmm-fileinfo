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

/**
 * This file declares data structures and functions of flie library.
 *
 * @file		mm_file.h
 * @author
 * @version 		1.0
 * @brief		This file declares data structures and functions of flie
 *				library.
 */

#ifndef __MM_FILE_H__
#define __MM_FILE_H__

#include <glib.h>

#include <mm_types.h>

/**
	@addtogroup FILEINFO
	@{

	@par
	This part describes the APIs with respect to extract meta data or media
	information directly from file.

*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * content attributes.
 */
#define MM_FILE_CONTENT_DURATION				"content-duration" 		/**< Duration of media */
#define MM_FILE_CONTENT_VIDEO_CODEC			"content-video-codec"		/**< Used video codec */
#define MM_FILE_CONTENT_VIDEO_BITRATE			"content-video-bitrate"		/**< Bitrate of video stream */
#define MM_FILE_CONTENT_VIDEO_FPS				"content-video-fps" 		/**< Frames per second of video stream */
#define MM_FILE_CONTENT_VIDEO_WIDTH			"content-video-width" 		/**< Width of video stream */
#define MM_FILE_CONTENT_VIDEO_HEIGHT			"content-video-height"		/**< Height of video stream */
#define MM_FILE_CONTENT_VIDEO_THUMBNAIL		"content-video-thumbnail"	/**< Thumbnail of video stream */
#define MM_FILE_CONTENT_VIDEO_TRACK_INDEX	"content-video-track-index" /**< Current stream of video */
#define MM_FILE_CONTENT_VIDEO_TRACK_COUNT	"content-video-track-count"/**< Number of video streams */
#define MM_FILE_CONTENT_AUDIO_CODEC			"content-audio-codec"		/**< Used audio codec */
#define MM_FILE_CONTENT_AUDIO_BITRATE		"content-audio-bitrate" 	/**< Bitrate of audio stream */
#define MM_FILE_CONTENT_AUDIO_CHANNELS		"content-audio-channels" 	/**< Channels of audio stream */
#define MM_FILE_CONTENT_AUDIO_SAMPLERATE	"content-audio-samplerate" /**< Sampling rate of audio stream */
#define MM_FILE_CONTENT_AUDIO_TRACK_INDEX	"content-audio-track-index"	/**< Current stream of audio */
#define MM_FILE_CONTENT_AUDIO_TRACK_COUNT	"content-audio-track-count"/**< Number of audio streams */

/**
 * tag attributes.
 */
#define 	MM_FILE_TAG_ARTIST 			"tag-artist"			/**< Artist */
#define	MM_FILE_TAG_TITLE				"tag-title"			/**< Title */
#define	MM_FILE_TAG_ALBUM			"tag-album"			/**< Album */
#define	MM_FILE_TAG_ALBUM_ARTIST	"tag-album-artist"			/**< Album_Artist */
#define	MM_FILE_TAG_GENRE				"tag-genre"			/**< Genre */
#define	MM_FILE_TAG_AUTHOR			"tag-author"			/**< Author / Composer */
#define	MM_FILE_TAG_COPYRIGHT		"tag-copyright"		/**< Copyright */
#define	MM_FILE_TAG_DATE				"tag-date"			/**< Year */
#define	MM_FILE_TAG_DESCRIPTION		"tag-description"		/**< Description */
#define	MM_FILE_TAG_COMMENT			"tag-comment"		/**< Comment */
#define	MM_FILE_TAG_ARTWORK			"tag-artwork"			/**< Artwork */
#define	MM_FILE_TAG_ARTWORK_SIZE		"tag-artwork-size"		/**< Artwork size */
#define	MM_FILE_TAG_ARTWORK_MIME	"tag-artwork-mime"	/**< Artwork mime type */
#define	MM_FILE_TAG_TRACK_NUM		"tag-track-num"		/**< Number of tracks */
#define	MM_FILE_TAG_CLASSIFICATION	"tag-classification"		/**< Classification Information */
#define	MM_FILE_TAG_RATING            	"tag-rating"			/**< Rating Information */
#define	MM_FILE_TAG_LONGITUDE         	"tag-longitude"		/**< location Information*/
#define	MM_FILE_TAG_LATIDUE           	"tag-latitude"			/**< location Information*/
#define	MM_FILE_TAG_ALTIDUE           	"tag-altitude"			/**< location Information*/
#define	MM_FILE_TAG_CONDUCTOR         	"tag-conductor"  		/**< Conductor Information*/
#define	MM_FILE_TAG_UNSYNCLYRICS      	"tag-unsynclyrics"  	/**< Unsynchronized Lyrics Information*/
#define	MM_FILE_TAG_SYNCLYRICS_NUM  	"tag-synclyrics-num"  	/**< Synchronized Lyrics Information*/
#define	MM_FILE_TAG_RECDATE			"tag-recdate"			/**< Recoding date */
#define	MM_FILE_TAG_ROTATE			"tag-rotate"			/**< Rotate(Orientation) Information*/
#define	MM_FILE_TAG_CDIS			"tag-cdis"				/**< CDIS in User Data Information*/
#define	MM_FILE_TAG_SMTA			"tag-smta"				/**< SMTA in User Data Information*/



/**
  * This function is to create tag attribute from given media file path.<BR>
  * Handle can be used to get actual tag information by mm_file_get_attrs() after this function.<BR>
  * Handle should be destroyed using mm_file_destroy_tag_attrs() after use.<BR>
  *
  * @param	 tag_attrs	[out]	tag attribute handle
  * @param	 filename	[in]	media file path
  *
  * @return	This function returns MM_ERROR_NONE on success, or negative value with error code.
  *
  * @remark	Filename must be UTF-8 format.
  *
  * @pre		File should be exists.
  * @post	Handle is ready to use.
  * @see 	mm_file_destroy_tag_attrs, mm_file_get_attrs
  * @par Example:
  * @code
#include <mm_file.h>

MMHandleType tag_attrs = NULL;
// get tag handle
mm_file_create_tag_attrs(&tag_attrs, filename);

// get attributes of tag
ret = mm_file_get_attrs( tag_attrs,
							&err_attr_name,
							MM_FILE_TAG_ARTIST, &ctag.artist.value.s_val, &ctag.artist.len,
							MM_FILE_TAG_ALBUM, &ctag.album.value.s_val, &ctag.album.len,
							MM_FILE_TAG_TITLE, &ctag.title.value.s_val, &ctag.title.len,
							MM_FILE_TAG_ALBUM_ARTIST, &ctag.album_artist.value.s_val, &ctag.album_artist.len,
							MM_FILE_TAG_GENRE, &ctag.genre.value.s_val, &ctag.genre.len,
							NULL);
if (ret != MM_ERROR_NONE)
{
	printf("failed to get %s attrs\n", *err_attr_name);
	return -1;
}

// Use tag information

// Release tag
mm_file_destroy_tag_attrs(tag_attrs);

 *	@endcode
 */
int mm_file_create_tag_attrs(MMHandleType * tag_attrs, const char * filename);

/**
 * This function is to destory the tag attribute handle which is created by mm_file_create_tag_attrs().<BR>
 * Handle should be destroyed using mm_file_destroy_tag_attrs() after use.
 *
 * @param	 tag_attrs	[in]	tag attribute handle
 *
 * @return	This function returns MM_ERROR_NONE on success, or negative value with error code.
 *
 * @remark	None.
 *	@pre		Handle should be valid.
 *	@post	Handle is not valid any more.
 * @see 	mm_file_create_tag_attrs, mm_file_get_attrs
 * @par Example:
 * @code
#include <mm_file.h>

MMHandleType tag_attrs = NULL;
// get tag handle
mm_file_create_tag_attrs(&tag_attrs, filename);

// get attributes of tag
ret = mm_file_get_attrs( tag_attrs,
							&err_attr_name,
							MM_FILE_TAG_ARTIST, &ctag.artist.value.s_val, &ctag.artist.len,
							MM_FILE_TAG_ALBUM, &ctag.album.value.s_val, &ctag.album.len,
							MM_FILE_TAG_TITLE, &ctag.title.value.s_val, &ctag.title.len,
							MM_FILE_TAG_ALBUM_ARTIST, &ctag.album_artist.value.s_val, &ctag.album_artist.len,
							MM_FILE_TAG_GENRE, &ctag.genre.value.s_val, &ctag.genre.len,
							NULL);
if (ret != MM_ERROR_NONE)
{
	printf("failed to get %s attrs\n", *err_attr_name);
	return -1;
}

// Use tag information

// Release tag
mm_file_destroy_tag_attrs(tag_attrs);

 *	@endcode
 */
int mm_file_destroy_tag_attrs(MMHandleType tag_attrs);

 /**
  * This function is to create content attribute from media file.<BR>
  * Handle can be used to get actual content information by mm_file_get_attrs() after this function.<BR>
  * Handle should be destroyed using mm_file_destroy_content_attrs() after use.<BR>
  *
  * @param	 content_attrs		[out]	content attribute handle.
  * @param	 filename	[in]	file path.
  *
  * @return	This function returns MM_ERROR_NONE on success, or negative value with error code.
  *
  * @remark	Filename must be UTF-8 format.
  *
  * @pre	File should be exists.
  * @post	Handle is ready to use.
  * @see 	mm_file_destroy_content_attrs, mm_file_get_attrs
  * @par Example:
  * @code
#include <mm_file.h>

// get track info
mm_file_get_stream_info(filename, &audio_track_num, &video_track_num);
printf ("Testing mm_file_get_stream_info()....audio=%d, video=%d\n", audio_track_num, video_track_num);

// create content handle
mm_file_create_content_attrs(&content_attrs, filename);

// get duration information
ret = mm_file_get_attrs(content_attrs, &err_attr_name, MM_FILE_CONTENT_DURATION, &ccontent.duration, NULL);
printf("duration: %d\n", ccontent.duration);

if (ret != MM_ERROR_NONE)
{
	printf("failed to get %s\n", *err_attr_name);
	free(err_attr_name);
}

// if audio track exists, get audio related information
if (audio_track_num)
{
	mm_file_get_attrs(content_attrs,
					NULL,
					MM_FILE_CONTENT_AUDIO_CODEC, &ccontent.audio_codec,
					MM_FILE_CONTENT_AUDIO_SAMPLERATE, &ccontent.audio_samplerate,
					MM_FILE_CONTENT_AUDIO_BITRATE, &ccontent.audio_bitrate,
					MM_FILE_CONTENT_AUDIO_CHANNELS, &ccontent.audio_channel,
					MM_FILE_CONTENT_AUDIO_TRACK_INDEX, &ccontent.audio_track_id,
					MM_FILE_CONTENT_AUDIO_TRACK_COUNT, &ccontent.audio_track_num,
					NULL);

	// Use audio information
}

// if video track exists, get video related information
if (video_track_num)
{
	 void *thumbnail = NULL;

	mm_file_get_attrs(content_attrs,
					NULL,
					MM_FILE_CONTENT_VIDEO_CODEC, &ccontent.video_codec,
					MM_FILE_CONTENT_VIDEO_BITRATE, &ccontent.video_bitrate,
					MM_FILE_CONTENT_VIDEO_FPS, &ccontent.video_fps,
					MM_FILE_CONTENT_VIDEO_TRACK_INDEX, &ccontent.video_track_id,
					MM_FILE_CONTENT_VIDEO_WIDTH, &ccontent.video_w,
					MM_FILE_CONTENT_VIDEO_HEIGHT, &ccontent.video_h,
					MM_FILE_CONTENT_VIDEO_THUMBNAIL, &ccontent.thumbnail.value.p_val, &ccontent.thumbnail.len,
					NULL);

	// Use video information
}

// Destory content handle
mm_file_destroy_content_attrs(content_attrs);
  * @endcode
  */
int mm_file_create_content_attrs(MMHandleType *content_attrs, const char *filename);

/**
  * This function is to destroy content attribute handle.<BR>
  * Handle should be destroyed using mm_file_destroy_content_attrs() after use.<BR>
  *
  * @param	content_attrs	[in]	content attribute handle.
  *
  * @return	This function returns MM_ERROR_NONE on success, or negative value with error code.
  *
  * @remark	None.
  *
  * @pre		Handle should be valid.
  * @post	Handle is not valid anymore.
  * @see 	mm_file_create_content_attrs, mm_file_get_attrs
  * @par Example:
  * @code
#include <mm_file.h>

// get track info
mm_file_get_stream_info(filename, &audio_track_num, &video_track_num);
printf ("Testing mm_file_get_stream_info()....audio=%d, video=%d\n", audio_track_num, video_track_num);

// create content handle
mm_file_create_content_attrs(&content_attrs, filename);

// get duration information
ret = mm_file_get_attrs(content_attrs, &err_attr_name, MM_FILE_CONTENT_DURATION, &ccontent.duration, NULL);
printf("duration: %d\n", ccontent.duration);

if (ret != MM_ERROR_NONE)
{
	printf("failed to get %s\n", *err_attr_name);
	free(err_attr_name);
}

// if audio track exists, get audio related information
if (audio_track_num)
{
	mm_file_get_attrs(content_attrs,
					NULL,
					MM_FILE_CONTENT_AUDIO_CODEC, &ccontent.audio_codec,
					MM_FILE_CONTENT_AUDIO_SAMPLERATE, &ccontent.audio_samplerate,
					MM_FILE_CONTENT_AUDIO_BITRATE, &ccontent.audio_bitrate,
					MM_FILE_CONTENT_AUDIO_CHANNELS, &ccontent.audio_channel,
					MM_FILE_CONTENT_AUDIO_TRACK_INDEX, &ccontent.audio_track_id,
					MM_FILE_CONTENT_AUDIO_TRACK_COUNT, &ccontent.audio_track_num,
					NULL);

	// Use audio information
}

// if video track exists, get video related information
if (video_track_num)
{
	void *thumbnail = NULL;

	mm_file_get_attrs(content_attrs,
					NULL,
					MM_FILE_CONTENT_VIDEO_CODEC, &ccontent.video_codec,
					MM_FILE_CONTENT_VIDEO_BITRATE, &ccontent.video_bitrate,
					MM_FILE_CONTENT_VIDEO_FPS, &ccontent.video_fps,
					MM_FILE_CONTENT_VIDEO_TRACK_INDEX, &ccontent.video_track_id,
					MM_FILE_CONTENT_VIDEO_WIDTH, &ccontent.video_w,
					MM_FILE_CONTENT_VIDEO_HEIGHT, &ccontent.video_h,
					MM_FILE_CONTENT_VIDEO_THUMBNAIL, &ccontent.thumbnail.value.p_val, &ccontent.thumbnail.len,
					NULL);

	// Use video information
}

// Destory content handle
mm_file_destroy_content_attrs(content_attrs);
* @endcode
*/
int mm_file_destroy_content_attrs(MMHandleType content_attrs);

/**
  * This function is to get the attributes from media tag or content.<BR>
  * Handle should be valid and created by using create functions such as mm_file_create_content_attrs(), mm_file_create_tag_attrs().<BR>
  * Handle should be destroyed after use.
  *
  * @param	attrs		[in]	tag or content attribute handle.
  * @param   err_attr_name	     [out]  Name of attribute which is failed to get
  * @param   first_attribute_name [in] 	Name of the first attribute to get
  * @param   ...					 [in] 	Value for the first attribute, followed optionally by more name/value pairs, terminated by NULL.
  *									 	But, in the case of data or string type, it should be name/value/size.
  *
  * @return	This function returns MM_ERROR_NONE on success, or negative value with error code.
  *
  * @remark	This function must be terminated by NULL argument.<BR>
  *			And, if this function is failed, err_attr_name param must be free.
  * @pre		Handle should be valid.
  * @post	Every input parameters are set by each information attributes.
  * @see 	mm_file_create_content_attrs, mm_file_create_tag_attrs
  * @par Example:
  * @code
#include <mm_file.h>

// get track info
mm_file_get_stream_info(filename, &audio_track_num, &video_track_num);
printf ("Testing mm_file_get_stream_info()....audio=%d, video=%d\n", audio_track_num, video_track_num);

// create content handle
mm_file_create_content_attrs(&content_attrs, filename);

// get duration information
ret = mm_file_get_attrs(content_attrs, &err_attr_name, MM_FILE_CONTENT_DURATION, &ccontent.duration, NULL);
printf("duration: %d\n", ccontent.duration);

if (ret != MM_ERROR_NONE)
{
	printf("failed to get %s\n", *err_attr_name);
	free(err_attr_name);
}

// if audio track exists, get audio related information
if (audio_track_num)
{
	mm_file_get_attrs(content_attrs,
					NULL,
					MM_FILE_CONTENT_AUDIO_CODEC, &ccontent.audio_codec,
					MM_FILE_CONTENT_AUDIO_SAMPLERATE, &ccontent.audio_samplerate,
					MM_FILE_CONTENT_AUDIO_BITRATE, &ccontent.audio_bitrate,
					MM_FILE_CONTENT_AUDIO_CHANNELS, &ccontent.audio_channel,
					MM_FILE_CONTENT_AUDIO_TRACK_INDEX, &ccontent.audio_track_id,
					MM_FILE_CONTENT_AUDIO_TRACK_COUNT, &ccontent.audio_track_num,
					NULL);

	// Use audio information
}

// if video track exists, get video related information
if (video_track_num)
{
	 void *thumbnail = NULL;

	mm_file_get_attrs(content_attrs,
					NULL,
					MM_FILE_CONTENT_VIDEO_CODEC, &ccontent.video_codec,
					MM_FILE_CONTENT_VIDEO_BITRATE, &ccontent.video_bitrate,
					MM_FILE_CONTENT_VIDEO_FPS, &ccontent.video_fps,
					MM_FILE_CONTENT_VIDEO_TRACK_INDEX, &ccontent.video_track_id,
					MM_FILE_CONTENT_VIDEO_WIDTH, &ccontent.video_w,
					MM_FILE_CONTENT_VIDEO_HEIGHT, &ccontent.video_h,
					MM_FILE_CONTENT_VIDEO_THUMBNAIL, &ccontent.thumbnail.value.p_val, &ccontent.thumbnail.len,
					NULL);

	// Use video information
}

// Destory content handle
mm_file_destroy_content_attrs(content_attrs);
  * @endcode
  */

int mm_file_get_attrs(MMHandleType attrs, char **err_attr_name, const char *first_attribute_name, ...)G_GNUC_NULL_TERMINATED;

/**
  * This function is to get the tag attributes from media data on memory while mm_file_create_tag_attrs() extracts from file.<BR>
  * This function acts same functionality as mm_file_create_tag_attrs() except media source format (file/memory).
  *
  * @param	tag_attrs	[out]	tag attributes handle.
  * @param	data	[in]	memory pointer of media data.
  * @param	size	[in]	size of media data.
  * @param	format	[in]	file format of media data.
  *
  * @return	This function returns MM_ERROR_NONE on success, or negative value with error code.
  *
  * @remark	Must be free return value.
  *
  * @pre	Input data pointer and input size should be valid. format should be matched with actual data.
  * @post	Tag attribute handle is ready to use.
  * @see 	mm_file_destroy_tag_attrs
  * @code
#include <mm_file.h>

// create tag handle from memory
mm_file_create_tag_attrs_from_memory (&tag_attrs, data, size, MM_FILE_FORMAT_MP3);

// get audio artist & album tag
mm_file_get_attrs(tag_attrs,
				NULL,
				MM_FILE_TAG_ARTIST, &ctag.artist.value.s_val, &ctag.artist.len,
				MM_FILE_TAG_ALBUM, &ctag.album.value.s_val, &ctag.album.len,
				MM_FILE_TAG_ALBUM_ARTIST, &ctag.album_artist.value.s_val, &ctag.album_artist.len,
				NULL);

// Destory tag handle
mm_file_destroy_tag_attrs(tag_attrs);
  * @endcode
 */
int mm_file_create_tag_attrs_from_memory(MMHandleType *tag_attrs, const void *data, unsigned int size, int format);

/**
  * This function is to get the content attributes from media data on memory while mm_file_create_content_attrs() extracts from file.<BR>
  * This function acts same functionality as mm_file_create_content_attrs() except media source format (file/memory).
  *
  * @param	content_attrs	[out]	content attributes handle.
  * @param	data	[in]	memory pointer of media data.
  * @param	size	[in]	size of media data.
  * @param	format	[in]	file format of media data.
  *
  * @return	This function returns MM_ERROR_NONE on success, or negative value with error code.
  *
  * @remark	Must be free return value.
  *
  * @pre	Input data pointer and input size should be valid. format should be matched with actual data.
  * @post	Content attribute handle is ready to use.
  * @see 	mm_file_destroy_content_attrs
  * @par Example::
  * @code
#include <mm_file.h>

// create content handle from memory
mm_file_create_content_attrs_from_memory (&content_attrs, data, size, MM_FILE_FORMAT_MP3);

// get audio bit rate and sample rate
mm_file_get_attrs(content_attrs,
				NULL,
				MM_FILE_CONTENT_AUDIO_SAMPLERATE, &ccontent.audio_samplerate,
				MM_FILE_CONTENT_AUDIO_BITRATE, &ccontent.audio_bitrate,
				NULL);

// Destory content handle
mm_file_destroy_content_attrs(content_attrs);
   * @endcode
   */
int mm_file_create_content_attrs_from_memory(MMHandleType *content_attrs, const void *data, unsigned int size, int format);

/**
 * This function is to get the count of audio/video stream from media file.<BR>
 * This function is useful for determining whether content is audio or video file.<BR>
 * For example, If content contains video streams, then content can be treat as video contents.
 *
 * @param filename		[in]	file path
 * @param audio_stream_num	[out]	number of audio stream of media file
 * @param	 video_stream_num	[out]	number of video stream of media file
 *
 * @return	This function returns MM_ERROR_NONE on success, or negative value with error code.
 * @remark	None.
 * @pre		File path should be exists and input param should be valid.
 * @post	Audio/Video stream count will be set
 * @see None.
 * @par Example::
 * @code
#include <mm_file.h>

// get track info
mm_file_get_stream_info(filename, &audio_track_num, &video_track_num);
printf ("Testing mm_file_get_stream_info()....audio=%d, video=%d\n", audio_track_num, video_track_num);
 * @endcode
 */

int mm_file_get_stream_info(const char* filename, int *audio_stream_num, int *video_stream_num);

 /**
   * This function is to get the content attributes without thumbnail from media file.<BR>
   * This function is almost same as mm_file_create_content_attrs() except extracting thumbnail feature.<BR>
   * As this function is not extracting thumbnail, this is faster than mm_file_create_content_attrs().
   *
   * @param	 content_attrs		[out]	content attribute handle.
   * @param	 filename	[in]	file path.
   *
   * @return	This function returns MM_ERROR_NONE on success, or negative value with error code.
   *
   * @remark	Filename must be UTF-8 format.
   *
   * @pre	File should be exists.
   * @post	Handle is ready to use.
   * @see 	mm_file_destroy_content_attrs, mm_file_get_attrs
   * @par Example::
   * @code
#include <mm_file.h>

// create content handle
mm_file_create_content_attrs_simple(&content_attrs, filename);

// get width,height information
mm_file_get_attrs(content_attrs,
				NULL,
				MM_FILE_CONTENT_VIDEO_WIDTH, &ccontent.video_w,
				MM_FILE_CONTENT_VIDEO_HEIGHT, &ccontent.video_h,
				NULL);

// Destory content handle
mm_file_destroy_content_attrs(content_attrs);
   * @endcode
   */
int mm_file_create_content_attrs_simple(MMHandleType *content_attrs, const char *filename);

int mm_file_create_content_attrs_safe(MMHandleType *content_attrs, const char *filename);

int mm_file_get_synclyrics_info(MMHandleType tag_attrs, int index, unsigned long *time_info, char **lyrics);

/**
 * @brief Get a frame of video media. Not support for DRM Contents.
 *
 * @remarks @a frame must be released with @c free() by you
 *
 * @param [in] path The file path
 * @param [in] timestamp The timestamp in milliseconds
 * @param [in] is_accurate @a true, user can get an accurated frame for given the timestamp.\n
 * @a false, user can only get the nearest i-frame of video rapidly.
 * @param [out] frame raw frame data in RGB888
 * @param [out] size The frame data size
 * @param [out] width The frame width
 * @param [out] height The frame height
 * @return 0 on success, otherwise a negative error value
 * @retval #METADATA_EXTRACTOR_ERROR_NONE Successful
 * @retval #METADATA_EXTRACTOR_ERROR_INVALID_PARAMETER Invalid parameter
 * @retval #METADATA_EXTRACTOR_ERROR_OUT_OF_MEMORY Not enough memory is available
 * @retval #METADATA_EXTRACTOR_ERROR_OPERATION_FAILED Internal Operation Fail
 * @pre Set path to extract by calling metadata_extractor_set_path()
 * @see metadata_extractor_create(), metadata_extractor_destroy()
 */

int mm_file_get_video_frame(const char* path, double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height);

int mm_file_get_video_frame_from_memory(const void *data, unsigned int datasize, double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height);

/**
	@}
 */

#ifdef __cplusplus
}
#endif

#endif

