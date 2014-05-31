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

#ifndef _MM_FILE_TRAVERSE_H_
#define _MM_FILE_TRAVERSE_H_

#define MMFILE_PATH_MAX 256

typedef enum
{
	MMFILE_FAIL = 0,
	MMFILE_SUCCESS		
} MMFILE_RETURN;

typedef int (*MMFunc) (void *data, void* user_data, bool file_test);

int mmfile_get_file_names (char *root_dir, MMFunc cbfunc, void* user_data);

#endif /* _MM_FILE_TRAVERSE_H_ */
