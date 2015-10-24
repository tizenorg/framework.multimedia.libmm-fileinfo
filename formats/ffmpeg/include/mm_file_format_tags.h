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

#ifndef _MMFILE_TAGS_H_
#define _MMFILE_TAGS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void *MMFileTagsHandle;

#define MMFILE_TAGS_SUCCESS   1
#define MMFILE_TAGS_FAIL      0

typedef enum mmfiletagstype {
	MMFILE_TAG_ID3V1 = 0,
	MMFILE_TAG_ID3V2,
	MMFILE_TAG_MUSICAL_MATCH,
	MMFILE_TAG_LYRICS3,
	MMFILE_TAG_APE,
	MMFILE_TAG_MAX,
} eMMFileTagsType;

typedef struct mmfileTags {
	eMMFileTagsType typeOfTag;
	unsigned char   bAppendedTag;
	unsigned int    version;
	unsigned int    startOffset;
	unsigned int    tagSize;
	unsigned int    endOffset;
} tMMFileTags;

int MMFileOpenTags(MMFileTagsHandle *tagsHandle, const char *uriName);
int MMFileGetFirstTag(MMFileTagsHandle  tagsHandle, tMMFileTags *out);
int MMFileGetNextTag(MMFileTagsHandle  tagsHandle, tMMFileTags *out);
int MMFileTagsClose(MMFileTagsHandle  tagsHandle);

#ifdef __cplusplus
}
#endif


#endif /* _MMFILE_TAGS_H_ */

