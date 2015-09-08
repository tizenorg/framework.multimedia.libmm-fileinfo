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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>

#include "mm_file_traverse.h"

static GList *g_directories = NULL;

int mmfile_get_file_names (char *root_dir, MMFunc cbfunc, void* user_data)
{
	struct stat 	statbuf;
	struct dirent 	dirp;
	struct dirent *result = NULL;
	DIR				*dp;

	char pdirname[MMFILE_PATH_MAX+1];

	memset (pdirname, 0x00, MMFILE_PATH_MAX+1);
	
	if ( lstat (root_dir, &statbuf) < 0 )
	{
		printf ("lstat error\n");
		return MMFILE_FAIL;
	}

	if ( S_ISDIR (statbuf.st_mode) == 0 )
	{
		printf ("it is not directory\n");
		return MMFILE_FAIL;
	}

	g_directories = g_list_append(g_directories, strdup (root_dir));


	int i = 0;
	gpointer element_data = NULL;
	while ( (element_data = g_list_nth_data (g_directories, i)) != NULL )
	{
		if (strlen ((char*) element_data) > 0 && strlen ((char*) element_data) <= MMFILE_PATH_MAX)
		{
			strncpy (pdirname, (char*) element_data, strlen((char*) element_data));
				
			if ( (dp = opendir (pdirname)) != NULL )
			{
				while (!readdir_r(dp, &dirp, &result))
				{
					char cdirname[MMFILE_PATH_MAX+1];
					
					if ( strcmp (dirp.d_name, ".") == 0 ||
						 strcmp (dirp.d_name, "..") == 0 )
					{
						continue;
					}

					memset (cdirname, 0x00, MMFILE_PATH_MAX+1);
					strncpy (cdirname, pdirname, strlen(pdirname));
					strncat (cdirname, "/", 1);
					strncat (cdirname, dirp.d_name, strlen(dirp.d_name));
						
					if ( lstat (cdirname, &statbuf) < 0 )
					{
						printf ("lstat error\n");
						closedir (dp);
						return MMFILE_FAIL;
					}

					if ( S_ISDIR (statbuf.st_mode) )
					{
						printf ("directory: %s\n", cdirname);
						g_directories = g_list_append(g_directories, strdup (cdirname));
					}
					else
					{
						printf ("file: %s\n", cdirname);
						if ( cbfunc != NULL )
						{
							cbfunc (cdirname, user_data, true);
						}
					}
					
				}

				closedir (dp);
			}
		}

		i++;
	}

	g_list_free (g_directories);
	
	return MMFILE_SUCCESS;
	
	
}

