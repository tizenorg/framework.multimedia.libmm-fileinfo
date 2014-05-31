lib_LTLIBRARIES = libmmfile_formats.la


libmmfile_formats_la_DEPENDENCIES =     $(top_builddir)/utils/libmmfile_utils.la 

noinst_HEADERS = include/mm_file_format_dummy.h \
		include/mm_file_format_aac.h \
		include/mm_file_format_amr.h \
		include/mm_file_format_imelody.h \
		include/mm_file_format_midi.h \
		include/mm_file_format_ffmpeg_mem.h \
		include/mm_file_format_ffmpeg.h \
		include/mm_file_format_mmf.h \
		#include/mm_file_format_mp3.h \
		include/mm_file_format_wav.h \
		include/mm_file_format_private.h

libmmfile_formats_la_SOURCES = mm_file_formats.c \
			mm_file_format_dummy.c \
			mm_file_format_ffmpeg.c \
			mm_file_format_ffmpeg_mem.c \
			mm_file_format_mp3.c \
			mm_file_format_aac.c \
			mm_file_format_mmf.c \
			mm_file_format_amr.c \
			mm_file_format_midi.c \
			mm_file_format_imelody.c \
			mm_file_format_wav.c \
			mm_file_format_frame.c

libmmfile_formats_la_CFLAGS = -I$(srcdir)/include \
			      $(MMCOMMON_CFLAGS) \
			   -I$(srcdir)/../../include \
			   -I$(srcdir)/../../utils/include \
			   -D_LARGEFILE64_SOURCE \
			   -D_FILE_OFFSET_BITS=64 \
			  $(AVUTIL_CFLAGS) \
			   $(AVCODEC_CFLAGS) \
			   $(SWSCALE_CFLAGS) \
			  $(AVFORMAT_CFLAGS) 

if USE_TESTMODE
libmmfile_formats_la_CFLAGS += -D__MMFILE_TEST_MODE__
endif  

if USE_DUMP
libmmfile_formats_la_CFLAGS += -DMMFILE_FORMAT_DEBUG_DUMP
endif  

if USE_DRM
libmmfile_formats_la_CFLAGS += $(DRMCLIENT_CFLAGS) -DDRM_SUPPORT
endif

if USE_IOMMAP
libmmfile_formats_la_CFLAGS += -D__MMFILE_MMAP_MODE__
endif  


libmmfile_formats_la_LIBADD = $(MMCOMMON_LIBS) \
				$(AVUTIL_LIBS) \
				$(AVCODEC_LIBS) \
				$(AVFORMAT_LIBS) \
				$(SWSCALE_LIBS) \
      			  $(top_builddir)/utils/libmmfile_utils.la 

if USE_DRM
libmmfile_formats_la_LIBADD += $(DRMCLIENT_LIBS)
endif

libmmfile_formats_la_CFLAGS += $(MMLOG_CFLAGS) -DMMF_LOG_OWNER=0x040 -DMMF_DEBUG_PREFIX=\"MMF-FILE-FORMAT-FFMPEG\"
libmmfile_formats_la_LIBADD += $(MMLOG_LIBS)

libmmfile_formats_la_CFLAGS += -D__MMFILE_FFMPEG_V100__
libmmfile_formats_la_CFLAGS += -D__MMFILE_FFMPEG_V085__
