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
#include <vconf.h>

#ifdef GCONF_SUPPORT
#include <gconf/gconf-client.h>
#endif

#include "mm_debug.h"
#include "mm_file_utils.h"

/* This macro is the same with global-gconf.h */
#define MMFILE_LANGUAGETYPE_REPOSITORY      "/Apps/Settings/language_type"
typedef enum
{
	MMFILE_LANGUAGE_ENGLISH = 0x00,	/**<Language - English*/
	MMFILE_LANGUAGE_GERMAN,			/**<Language - German*/
	MMFILE_LANGUAGE_FRENCH,			/**<Language - French*/
	MMFILE_LANGUAGE_ITALIAN,			/**<Language - Italian*/
	MMFILE_LANGUAGE_DUTCH,			/**<Language - Dutch*/
	MMFILE_LANGUAGE_SPANISH,			/**<Language - Spanish*/
	MMFILE_LANGUAGE_GREEK,			/**<Language - Greek*/
	MMFILE_LANGUAGE_PORTUGUESE,		/**<Language - Portuguese*/
	MMFILE_LANGUAGE_TURKISH,			/**<Language - Turkish*/
	MMFILE_LANGUAGE_JAPAN_CP932,		/**<Language - Japanease for CP932*/
	MMFILE_LANGUAGE_SIM_CHINA,		/**<Language - Simplified Chinese*/
	MMFILE_LANGUAGE_TRA_CHINA,		/**<Language - Traditional Chinese*/
	MMFILE_LANGUAGE_JAPAN,			/**<Language - Japanease*/
#if 0        
	MMFILE_LANGUAGE_BULGARIAN,		/**<Language - Bulgarian*/
	MMFILE_LANGUAGE_ARABIC,			/**<Language - Arabic*/
#endif
    MMFILE_LANGUAGE_MAX
} eMMFileSettingPhoneLanguage;

const char *MMFILE_LOCALE_TABLE [MMFILE_LANGUAGE_MAX] =
{
	"EUC-KR",		/* Temporally - Language - English */
	"ISO8859-1",		/* Language - German */
	"ISO8859-1",		/* Language - French */
	"ISO8859-1",		/* Language - Italian */
	"ISO8859-1",		/* Temporally -  Language - Dutch */
	"ISO8859-1",		/* Language - Spanish */
	"ISO8859-7",		/* Language - Greek */
	"ISO8859-1",		/* Language - Portuguese */
	"ISO8859-3",		/* Language - Turkish */
	"CP932",			/* Language - Japan*/
	"GBK",			/* Language - Simplified Chinese */
	"BIG5",			/* Language - Traditional Chinese */
	"SHIFT_JIS"		/* Language - Japanease */
#if 0
					/* Language - Bulgarian */
					/* Language - Arabic */
#endif
};


static int _MMFileUtilGetLocaleindex()
{
	int index = MMFILE_LANGUAGE_ENGLISH;
	char *lang = NULL;
	char *con_iso = NULL;

	char *china_prefix = "zh";
	char *eng_prefix = "en";

	char *china_lang = "zh_CN";
	char *hongkong_lang = "zh_HK";
	char *taiwan_lang = "zh_TW";
	char *jpn_lang = "ja_JP";


	con_iso = vconf_get_str(VCONFKEY_CSC_COUNTRY_ISO);
	lang = vconf_get_str(VCONFKEY_LANGSET);

	if (lang != NULL) {
		if (strncmp(lang,china_prefix, strlen(china_prefix)) == 0) {
			/* This case is selected language is china */
			if (strncmp(lang, china_lang, strlen(china_lang)) == 0) {
				debug_msg("[%s]character set is simplified chinese", lang);
				index = MMFILE_LANGUAGE_SIM_CHINA;
			} else {
				debug_msg("[%s]character set is traditional chinese", lang);
				index = MMFILE_LANGUAGE_TRA_CHINA;
			}
		} else if (strncmp(lang, eng_prefix, strlen(eng_prefix)) == 0) {
			/* This case is selected language is engilish
			     In case of engilish, the character set is related with region of target binary */
			debug_msg("[%s]character set is engilish", lang);
			if (con_iso!=NULL) {
				if (strncmp(con_iso, "CN", strlen("CN")) == 0) {
					debug_msg("region of this target is China.");
					index = MMFILE_LANGUAGE_SIM_CHINA;
				} else if ((strncmp(con_iso, "TW", strlen("TW")) == 0) || (strncmp(con_iso, "HK", strlen("HK")) == 0)) {
					debug_msg("region of this target is Hong kong or Twian.");
					index = MMFILE_LANGUAGE_TRA_CHINA;
				} else {
					debug_msg("Use default character set.");
					index = MMFILE_LANGUAGE_ENGLISH;
				}
			} else {
				debug_error("country iso value is NULL");
				index = MMFILE_LANGUAGE_ENGLISH;
			}
		} else if (strncmp(lang, jpn_lang, strlen(jpn_lang)) == 0) {
			/* This case is selected language is japanease */
			debug_msg("[%s]character set is japanease", lang);
			index = MMFILE_LANGUAGE_JAPAN;
		} else {
			debug_error("use default character set");
			index = MMFILE_LANGUAGE_ENGLISH;
		}
	} else {
		debug_error("language value is NULL, use default character set");
		index = MMFILE_LANGUAGE_ENGLISH;
	}

	if(!con_iso) free(con_iso);
	if(!lang) free(lang);

	return index;
}

EXPORT_API
char *MMFileUtilGetLocale (int *error)
{
	int index = 0;
	int err = 0;
	index = _MMFileUtilGetLocaleindex();

	if (index < 0 || index >= MMFILE_LANGUAGE_MAX)
	{
		debug_error ("invalid index\n");
		err = MMFILE_UTIL_FAIL;
		return NULL;
	}

	err = MMFILE_UTIL_SUCCESS;
	if (error)
	{
		*error = err;
	}

	return (char *)MMFILE_LOCALE_TABLE[index];
}

