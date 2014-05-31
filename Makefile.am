ACLOCAL_AMFLAGS=-I m4

SUBDIRS = utils \
	  codecs \
	  formats \
	  . \
	  tests 

lib_LTLIBRARIES = libmmffile.la

noinst_HEADERS = include/mm_file_codecs.h \
		 include/mm_file_formats.h \
		 include/mm_file_format_frame.h

libmmffile_la_SOURCES = mm_file.c 

includelibmmffiledir = $(includedir)/mmf
includelibmmffile_HEADERS = include/mm_file.h 
			
libmmffile_la_CFLAGS = -I$(srcdir)/include \
		       $(MMCOMMON_CFLAGS) \
		       -I$(srcdir)/utils/include \
		       -D_LARGEFILE64_SOURCE \
		       -D_FILE_OFFSET_BITS=64 

if USE_TESTMODE
libmmffile_la_CFLAGS += -D__MMFILE_TEST_MODE__
endif

libmmffile_la_DEPENDENCIES = $(top_builddir)/utils/libmmfile_utils.la 
		   
		  
libmmffile_la_LIBADD = $(top_builddir)/utils/libmmfile_utils.la \
			-ldl \
		       $(MMCOMMON_LIBS)

if USE_DYN
libmmffile_la_CFLAGS += -D__MMFILE_DYN_LOADING__
else
libmmffile_la_LIBADD += $(top_builddir)/codecs/ffmpeg/libmmfile_codecs.la \
						$(top_builddir)/formats/ffmpeg/libmmfile_formats.la
endif


libmmffile_la_CFLAGS += $(MMLOG_CFLAGS) -DMMF_LOG_OWNER=0x040 -DMMF_DEBUG_PREFIX=\"MMF-FILE\"
libmmffile_la_LIBADD += $(MMLOG_LIBS)


pcfiles = mm-fileinfo.pc
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(pcfiles)
EXTRA_DIST = $(pcfiles)
