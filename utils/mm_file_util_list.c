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

#include <glib.h>
#include "mm_file_debug.h"
#include "mm_file_utils.h"

EXPORT_API
MMFileList mmfile_list_alloc()
{
	return g_list_alloc();
}

EXPORT_API
MMFileList mmfile_list_append(MMFileList list, void *data)
{
	return g_list_append(list, data);
}

EXPORT_API
MMFileList mmfile_list_prepend(MMFileList list, void *data)
{
	return g_list_prepend(list, data);
}

EXPORT_API
MMFileList mmfile_list_find(MMFileList list, void *data)
{
	return g_list_find(list, data);
}

EXPORT_API
MMFileList mmfile_list_first(MMFileList list)
{
	return g_list_first(list);
}

EXPORT_API
MMFileList mmfile_list_last(MMFileList list)
{
	return g_list_last(list);
}

EXPORT_API
MMFileList mmfile_list_nth(MMFileList list, unsigned int n)
{
	return g_list_nth(list, n);
}

EXPORT_API
MMFileList mmfile_list_next(MMFileList list)
{
	return g_list_next(list);
}

EXPORT_API
MMFileList mmfile_list_previous(MMFileList list)
{
	return g_list_previous(list);
}

EXPORT_API
unsigned int mmfile_list_length(MMFileList list)
{
	return g_list_length(list);
}

EXPORT_API
MMFileList mmfile_list_remove(MMFileList list, void *data)
{
	return g_list_remove(list, data);
}

EXPORT_API
MMFileList mmfile_list_remove_all(MMFileList list, void *data)
{
	return g_list_remove_all(list, data);
}


EXPORT_API
MMFileList mmfile_list_reverse(MMFileList list)
{
	return g_list_reverse(list);
}

EXPORT_API
void mmfile_list_free(MMFileList list)
{
	g_list_free(list);
}

