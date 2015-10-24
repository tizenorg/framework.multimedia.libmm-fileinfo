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

#ifndef __MMFILE_DYN_LOADING__
int mmfile_format_get_frame(const char *path, double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height);

int mmfile_format_get_frame_from_memory(const void *data, unsigned int datasize, double timestamp, bool is_accurate, unsigned char **frame, int *size, int *width, int *height);
#endif
