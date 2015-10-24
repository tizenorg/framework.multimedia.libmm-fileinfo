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

#ifndef _MMFILE_DEBUG_H_
#define _MMFILE_DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "MM_FILEINFO"

/*#define LOG_COLOR */

#ifdef LOG_COLOR
#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_BLUE     "\033[34m"
#define FONT_COLOR_PURPLE   "\033[35m"
#define FONT_COLOR_CYAN     "\033[36m"
#define FONT_COLOR_GRAY     "\033[37m"

#define debug_log(fmt, arg...) do { \
			LOGD(FONT_COLOR_RESET fmt, ##arg); \
		} while (0)

#define debug_msg(fmt, arg...) do { \
			LOGD(FONT_COLOR_RESET fmt, ##arg); \
		} while (0)

#define debug_warning(fmt, arg...) do { \
			LOGW(FONT_COLOR_GREEN fmt, ##arg); \
		} while (0)

#define debug_error(fmt, arg...) do { \
			LOGE(FONT_COLOR_RED fmt, ##arg); \
		} while (0)

#define debug_fenter() do { \
			LOGE(FONT_COLOR_RESET "<ENTER> \n");     \
		} while (0)

#define debug_fleave() do { \
			LOGE(FONT_COLOR_RESET "<LEAVE> \n");     \
		} while (0)

#else

#define debug_log(fmt, arg...) do { \
			LOGD(" "fmt"", ##arg);     \
		} while (0)

#define debug_msg(fmt, arg...) do { \
			LOGD(" "fmt"", ##arg);     \
		} while (0)

#define debug_warning(fmt, arg...) do { \
			LOGW(" "fmt"", ##arg);     \
		} while (0)

#define debug_error(fmt, arg...) do { \
			LOGE(" "fmt"", ##arg);     \
		} while (0)

#define debug_fenter() do { \
			LOGE("<ENTER> \n");     \
		} while (0)

#define debug_fleave() do { \
			LOGE("<LEAVE> \n");     \
		} while (0)
#endif


#ifdef __cplusplus
}
#endif

#endif /* _MMFILE_DEBUG_H_ */

