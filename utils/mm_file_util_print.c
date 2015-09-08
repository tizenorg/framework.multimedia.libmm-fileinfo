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

#include "mm_debug.h"
#include "mm_file_utils.h"

EXPORT_API
void mmfile_format_print_contents (MMFileFormatContext *in)
{
	if (in) {
		debug_msg ("formatType = %d\n", in->formatType);
		debug_msg ("commandType = %d\n", in->commandType);
		debug_msg ("duration = %d\n", in->duration);
		debug_msg ("videoTotalTrackNum = %d\n", in->videoTotalTrackNum);
		debug_msg ("audioTotalTrackNum = %d\n", in->audioTotalTrackNum);
		debug_msg ("nbStreams = %d\n", in->nbStreams);
		debug_msg ("audioStreamId = %d\n", in->audioStreamId);
		debug_msg ("videoStreamId = %d\n", in->videoStreamId);

		if (in->videoTotalTrackNum > 0 && in->streams[MMFILE_VIDEO_STREAM]) {
			debug_msg ("VstreamType = %d\n", in->streams[MMFILE_VIDEO_STREAM]->streamType);
			debug_msg ("VcodecId = %d\n", in->streams[MMFILE_VIDEO_STREAM]->codecId);
			debug_msg ("VbitRate = %d\n", in->streams[MMFILE_VIDEO_STREAM]->bitRate);
			debug_msg ("VframePerSec = %d\n", in->streams[MMFILE_VIDEO_STREAM]->framePerSec);
			debug_msg ("Vwidth = %d\n", in->streams[MMFILE_VIDEO_STREAM]->width);
			debug_msg ("Vheight = %d\n", in->streams[MMFILE_VIDEO_STREAM]->height);
			debug_msg ("VnbChannel = %d\n", in->streams[MMFILE_VIDEO_STREAM]->nbChannel);
			debug_msg ("VsamplePerSec = %d\n", in->streams[MMFILE_VIDEO_STREAM]->samplePerSec);
		}

		if (in->audioTotalTrackNum > 0 && in->streams[MMFILE_AUDIO_STREAM]) {
			debug_msg ("AstreamType = %d\n", in->streams[MMFILE_AUDIO_STREAM]->streamType);
			debug_msg ("AcodecId = %d\n", in->streams[MMFILE_AUDIO_STREAM]->codecId);
			debug_msg ("AbitRate = %d\n", in->streams[MMFILE_AUDIO_STREAM]->bitRate);
			debug_msg ("AframePerSec = %d\n", in->streams[MMFILE_AUDIO_STREAM]->framePerSec);
			debug_msg ("Awidth = %d\n", in->streams[MMFILE_AUDIO_STREAM]->width);
			debug_msg ("Aheight = %d\n", in->streams[MMFILE_AUDIO_STREAM]->height);
			debug_msg ("AnbChannel = %d\n", in->streams[MMFILE_AUDIO_STREAM]->nbChannel);
			debug_msg ("AsamplePerSec = %d\n", in->streams[MMFILE_AUDIO_STREAM]->samplePerSec);
		}
	}
}

EXPORT_API
void mmfile_format_print_tags (MMFileFormatContext *in)
{
	if (in) {
		if (in->title)				debug_msg ("title = %s\n", in->title);
		if (in->artist)				debug_msg ("artist = %s\n", in->artist);
		if (in->author)			debug_msg ("author = %s\n", in->author);
		if (in->composer)			debug_msg ("composer = %s\n", in->composer);
		if (in->album)			debug_msg ("album = %s\n", in->album);
		if (in->album_artist)			debug_msg ("album_artist = %s\n", in->album_artist);
		if (in->copyright)			debug_msg ("copyright = %s\n", in->copyright);
		if (in->comment)			debug_msg ("comment = %s\n", in->comment);
		if (in->genre)			debug_msg ("genre = %s\n", in->genre);
		if (in->year)				debug_msg ("year = %s\n", in->year);
		if (in->recDate)			debug_msg ("recDate = %s\n", in->recDate);
		if (in->tagTrackNum)		debug_msg ("tagTrackNum = %s\n", in->tagTrackNum);
		if (in->artworkMime)		debug_msg ("artworkMime = %s\n", in->artworkMime);
								debug_msg ("artworksize = %d\n", in->artworkSize);
		if (in->artwork)			debug_msg ("artwork = %p\n", in->artwork);
		if (in->classification)		debug_msg ("classification = %s\n", in->classification);
	}
}

EXPORT_API
void mmfile_format_print_frame (MMFileFormatFrame *in)
{
	if (in) {
		debug_msg ("in->bCompressed = %d\n", in->bCompressed);
		debug_msg ("in->frameData = %p\n", in->frameData);
		debug_msg ("in->frameHeight = %d\n", in->frameHeight);
		debug_msg ("in->frameWidth = %d\n", in->frameWidth);
		debug_msg ("in->frameSize = %d\n", in->frameSize);
		debug_msg ("in->configLenth = %d\n", in->configLenth);
		debug_msg ("in->configData = %p\n", in->configData);
	}
}

EXPORT_API
void mmfile_codec_print (MMFileCodecContext *in)
{
	if (in) {
		debug_msg ("codecType = %d\n", in->codecType);
		debug_msg ("codec id = %d\n", in->codecId);
	}
}

