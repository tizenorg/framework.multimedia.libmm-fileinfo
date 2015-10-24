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
#include <stdlib.h>
#include "mm_file_debug.h"
#include "mm_file_format_tag_id3.h"

#define MMFILE_ID3V1TAG_SIZE 128
#define MMFILE_ID3V2TAG_HEADER_SIZE 10

static unsigned int GetSynchsafeInteger(unsigned int value);

EXPORT_API
int MMFileID3V1TagFind(MMFileIOHandle *fp, unsigned char bAppended, unsigned int startOffset, unsigned int endOffset, tMMFileTags *out)
{
	int ret = 0;

	if (!fp || !out) {
		debug_error("Invalid input\n");
		return MMFILE_ID3TAG_FAIL;
	}

	if (bAppended) {
		unsigned char tagSymbol[3] = {0, };

		unsigned int offset = endOffset - MMFILE_ID3V1TAG_SIZE;

		mmfile_seek(fp, offset, MMFILE_SEEK_SET);

		ret = mmfile_read(fp, tagSymbol, 3);
		if (MMFILE_UTIL_FAIL == ret) {
			debug_error("read error\n");
			return MMFILE_ID3TAG_FAIL;
		}

		if (memcmp("TAG", tagSymbol, 3) == 0) {
			unsigned char versionBuf[2] = {0, };

			out->typeOfTag = MMFILE_TAG_ID3V1;
			out->startOffset = offset;
			out->endOffset = endOffset;
			out->bAppendedTag = 1;
			out->tagSize = MMFILE_ID3V1TAG_SIZE;

			offset += 125; /* byte delimiter offset: ID3V1.1 spec. */
			mmfile_seek(fp, offset, MMFILE_SEEK_SET);

			ret = mmfile_read(fp, versionBuf, 2);
			if (MMFILE_UTIL_FAIL == ret) {
				debug_error("read error\n");
				return MMFILE_ID3TAG_FAIL;
			}

			if (versionBuf[0] == '\0' && versionBuf[1] != '\0') {
				out->version = MMFILE_ID3TAG_V1_1;
			} else {
				out->version = MMFILE_ID3TAG_V1_0;
			}

#ifdef __MMFILE_TEST_MODE__
			debug_msg("typeOfTag = %d\n", out->typeOfTag);
			debug_msg("startOffset = %d\n", out->startOffset);
			debug_msg("endOffset = %d\n", out->endOffset);
			debug_msg("bAppendedTag = %d\n", out->bAppendedTag);
			debug_msg("tagSize = %d\n", out->tagSize);
			debug_msg("version = %d\n", out->version);
#endif

			return MMFILE_ID3TAG_SUCCESS;
		}
	}

	return MMFILE_ID3TAG_FAIL;
}


EXPORT_API
int MMFileID3V2TagFind(MMFileIOHandle *fp, unsigned char bAppended, unsigned int startOffset, unsigned int endOffset, tMMFileTags *out)
{
	unsigned char *id3v2ID = NULL;
	unsigned char header[MMFILE_ID3V2TAG_HEADER_SIZE] = {0, };
	unsigned int  offset = 0;
	unsigned int  index = 0;
	int ret = 0;

	if (!fp || !out) {
		debug_error("Invalid input\n");
		return MMFILE_ID3TAG_FAIL;
	}

	if (bAppended) {
		id3v2ID = "3DI";
		offset = endOffset - MMFILE_ID3V2TAG_HEADER_SIZE;
	} else {
		id3v2ID = "ID3";
		offset = startOffset;
	}

	mmfile_seek(fp, offset, MMFILE_SEEK_SET);

	ret = mmfile_read(fp, header, MMFILE_ID3V2TAG_HEADER_SIZE);
	if (MMFILE_UTIL_FAIL == ret) {
		debug_error("read error\n");
		return MMFILE_ID3TAG_FAIL;
	}

	if (memcmp(id3v2ID, header, 3) == 0) {
		index += 3;
		out->typeOfTag = MMFILE_TAG_ID3V2;
		out->bAppendedTag = bAppended;
		out->startOffset = offset;

		/*version check */
		switch (header[index]) {
			case 0:
				out->version = MMFILE_ID3TAG_V2_0;
				break;
			case 2:
				out->version = MMFILE_ID3TAG_V2_2;
				break;
			case 3:
				out->version = MMFILE_ID3TAG_V2_3;
				break;
			case 4:
				out->version = MMFILE_ID3TAG_V2_4;
				break;
			default:
				debug_warning("unknown version of id3v2\n");
				break;
		}

		index += 2;
		/*check footer */
		unsigned char footer = (header[index] & 0x10) ? 1 : 0;

		index += 1;
		/*out->tagSize = GetSynchsafeInteger() + (footer ? 20 : 10) */

		if (bAppended) {
/*            out->endOffset */
		}
		out->endOffset = endOffset;
	}


	return MMFILE_ID3TAG_FAIL;
}


static unsigned int GetSynchsafeInteger(unsigned int value)
{
	int i = 0;
	const unsigned int mask = 0x7F000000;
	unsigned int ret = 0;

	for (i = 0; i < sizeof(unsigned int); i++) {
		ret = (ret << 7) | ((value & mask) >> 24);
		value <<= 8;
	}
	return ret;
}

