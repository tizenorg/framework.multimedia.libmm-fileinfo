lib_LTLIBRARIES = libmmfile_utils.la

noinst_HEADERS =  include/mm_file_utils.h

libmmfile_utils_la_SOURCES = mm_file_util_memory.c \
			   mm_file_util_io.c \
			   mm_file_util_io_file.c \
			   mm_file_util_io_mem.c \
			   mm_file_util_io_mmap.c \
			   mm_file_util_print.c \
			   mm_file_util_mime.c \
			   mm_file_util_string.c \
			   mm_file_util_list.c \
			   mm_file_util_locale.c \
			   mm_file_util_validity.c \
			   mm_file_util_tag.c
			
libmmfile_utils_la_CFLAGS = -I$(srcdir)/include \
		       -I$(srcdir)/../include \
		       $(MMCOMMON_CFLAGS) \
		       -D_LARGEFILE64_SOURCE \
		       -D_FILE_OFFSET_BITS=64 \
		       $(ICU_CFLAGS) \
		       $(VCONF_CFLAGS)
if USE_TESTMODE
libmmfile_utils_la_CFLAGS += -D__MMFILE_TEST_MODE__
endif  

libmmfile_utils_la_LIBADD = $(MMCOMMON_LIBS) \
			    $(ICU_LIBS) \
			    $(VCONF_LIBS)

libmmfile_utils_la_CFLAGS += $(MMLOG_CFLAGS) -DMMF_LOG_OWNER=0x040 -DMMF_DEBUG_PREFIX=\"MMF-FILE-UTILS\"
libmmfile_utils_la_LIBADD += $(MMLOG_LIBS)
