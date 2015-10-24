lib_LTLIBRARIES = libmmfile_codecs.la

libmmfile_codecs_la_DEPENDENCIES = $(top_builddir)/utils/libmmfile_utils.la 

noinst_HEADERS = include/mm_file_codec_dummy.h \
		  include/mm_file_codec_private.h 


libmmfile_codecs_la_SOURCES = mm_file_codecs.c \
			  mm_file_codec_dummy.c 


libmmfile_codecs_la_CFLAGS = -I$(srcdir)/include \
			 $(MMCOMMON_CFLAGS) \
			 -I$(srcdir)/../../include \
			 -I$(srcdir)/../../utils/include \
			 -D_LARGEFILE64_SOURCE \
			 -D_FILE_OFFSET_BITS=64

libmmfile_codecs_la_LIBADD = $(MMCOMMON_LIBS)	\
			     $(top_builddir)/utils/libmmfile_utils.la 

libmmfile_codecs_la_CFLAGS += $(DLOG_CFLAGS)
libmmfile_codecs_la_LIBADD += $(DLOG_LIBS)
