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

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <unicode/ucsdet.h>
#include "mm_file_debug.h"
#include "mm_file_utils.h"

#ifdef __MMFILE_MEM_TRACE__
EXPORT_API
int  mmfile_util_wstrlen(unsigned short *wText)
{
	int n = 0;

	if (NULL == wText) {
		debug_error("wText is NULL\n");
		return MMFILE_UTIL_FAIL;
	}

	n = 0;

	while (*(wText + n) != 0) {
		n++;
	}

	return n;
}

short __WmLngSwapShort(short aShort)
{
	return ((aShort << 8) + (aShort >> 8));
}

EXPORT_API
short *mmfile_swap_2byte_string(short *mszOutput, short *mszInput, int length)
{
	int	i;

	for (i = 0; i < length; i++) {
		if (mszInput[i] == 0)
			break;

		mszOutput[i] = __WmLngSwapShort(mszInput[i]);
	}

	mszOutput[i] = 0;

	return mszOutput;
}


EXPORT_API
char *mmfile_string_convert_debug(const char *str, unsigned int len,
                                  const char *to_codeset, const char *from_codeset,
                                  int *bytes_read,
                                  int *bytes_written,
                                  const char *func,
                                  unsigned int line)
{
	char *tmp = g_convert(str, len, to_codeset, from_codeset, bytes_read, bytes_written, NULL);

	if (tmp) {
#ifdef __MMFILE_TEST_MODE__
		debug_msg("## DEBUG ## %p = g_convert (%p, %u, %p, %p, %p ,%p, %p, %u) by %s() %d\n",
		          tmp, str, len, to_codeset, from_codeset, bytes_read, bytes_written, func, line);
#endif
	}

	return tmp;

}

EXPORT_API
char **mmfile_strsplit(const char *string, const char *delimiter)
{
	return g_strsplit(string, delimiter, -1);
}

EXPORT_API
void mmfile_strfreev(char **str_array)
{
	g_strfreev(str_array);
}

EXPORT_API
char *mmfile_strdup_debug(const char *str, const char *func, unsigned int line)
{
	char *temp = NULL;

	if (!str)
		return NULL;

	temp = strdup(str);

	if (temp) {
#ifdef __MMFILE_TEST_MODE__
		debug_msg("## DEBUG ## %p = strdup (%p) by %s() %d\n", temp, str, func, line);
#endif
	}

	return temp;
}


#else   /* __MMFILE_MEM_TRACE__ */

EXPORT_API
int  mmfile_util_wstrlen(unsigned short *wText)
{
	int n = 0;

	if (NULL == wText) {
		debug_error("wText is NULL\n");
		return MMFILE_UTIL_FAIL;
	}

	n = 0;

	while (*(wText + n) != 0) {
		n++;
	}

	return n;
}

EXPORT_API
char *mmfile_get_charset(const char *str)
{
	UCharsetDetector *ucsd = NULL;
	const UCharsetMatch *ucm = NULL;
	UErrorCode status = U_ZERO_ERROR;

	const char *charset = NULL;
	char *ret_charset = NULL;

	ucsd = ucsdet_open(&status);
	if (U_FAILURE(status)) {
		debug_error("fail to ucsdet_open\n");
		return NULL;
	}

	ucsdet_enableInputFilter(ucsd, TRUE);

	ucsdet_setText(ucsd, str, strlen(str), &status);
	if (U_FAILURE(status)) {
		debug_error("fail to ucsdet_setText\n");
		goto done;
	}

	ucm = ucsdet_detect(ucsd, &status);
	if (U_FAILURE(status)) {
		debug_error("fail to ucsdet_detect\n");
		goto done;
	}

	if (ucm == NULL) {
		debug_error("fail to ucsdet_detect\n");
		goto done;
	}

	charset = ucsdet_getName(ucm, &status);
	if (U_FAILURE(status)) {
		debug_error("fail to ucsdet_getName\n");
		charset = NULL;
		goto done;
	}

done:

	if (charset != NULL)
		ret_charset = strdup(charset);

	ucsdet_close(ucsd);

	return ret_charset;
}

EXPORT_API
char *mmfile_string_convert(const char *str, unsigned int len,
                            const char *to_codeset, const char *from_codeset,
                            gsize *bytes_read,
                            unsigned int *bytes_written)
{
	char *result = NULL;
	GError *err = NULL;
	/*int i = 0;*/
	gsize written_len = 0;

	if (len != 0) {
		result = g_convert(str, len, to_codeset, from_codeset, bytes_read, &written_len, &err);

		/*if converting failed, return duplicated source string.*/
		if (result == NULL) {
#ifdef __MMFILE_TEST_MODE__
			debug_warning("text encoding failed.[%s][%d]\n", str, len);
			if (err != NULL) {
				debug_warning("Error msg [%s]", err->message);
				g_error_free(err);
			}
#endif
			written_len = 0;
		} else {
			/* check carrige return */
			unsigned int i = 0;
			for (i = 0; i < written_len; i++) {
				if (result[i] == 13) {
					if (result[i + 1] != 10)
						result[i] = 10;
				}
			}
		}
	} else {
		written_len = 0;
	}

	if (bytes_written != NULL) {
		*bytes_written = written_len;
	}

	return result;
}

EXPORT_API
char **mmfile_strsplit(const char *string, const char *delimiter)
{
	return g_strsplit(string, delimiter, -1);
}

EXPORT_API
void mmfile_strfreev(char **str_array)
{
	g_strfreev(str_array);
}

EXPORT_API
char *mmfile_strdup(const char *str)
{
	if (!str)
		return NULL;

	return strdup(str);
}

#endif  /*__MMFILE_MEM_TRACE__*/

