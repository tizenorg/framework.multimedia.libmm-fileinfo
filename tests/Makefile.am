bin_PROGRAMS = mm_file_test
mm_file_test_DEPENDENCIES = $(top_builddir)/libmmffile.la


mm_file_test_SOURCES = mm_file_test.c \
		   mm_file_traverser.c
	
mm_file_test_CFLAGS = -I$(top_builddir)/include \
		      $(MMCOMMON_CFLAGS) \
		      -I$(includedir) \
		      -D_LARGEFILE64_SOURCE \
                      -D_FILE_OFFSET_BITS=64 \
		      $(GLIB_CFLAGS) 

mm_file_test_LDADD =	$(MMCOMMON_LIBS) \
			$(top_builddir)/libmmffile.la \
			$(top_builddir)/utils/libmmfile_utils.la \
			 $(GLIB_LIBS) 


if USE_DYN
else
mm_file_test_LDADD += $(top_builddir)/codecs/ffmpeg/libmmfile_codecs.la  \
					  $(top_builddir)/formats/ffmpeg/libmmfile_formats.la 
endif


#bin_PROGRAMS += memtrace_reader

#memtrace_reader_SOURCES = mm_file_memtrace_reader.c

#memtrace_reader_CFLAGS = -I/$(srcdir)/include \
#		      -D_LARGEFILE64_SOURCE \
#                      -D_FILE_OFFSET_BITS=64 
		  		
#memtrace_reader_DEPENDENCIES = 
#memtrace_reader_LDADD = 

