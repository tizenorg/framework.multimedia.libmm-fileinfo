ifeq ($(ARCH),target)
	PKG_CONFIG_PATH=/usr/lib/pkgconfig
	export PKG_CONFIG_PATH
	CC=arm-linux-gcc -Wall
	CXX=arm-linux-g++ -Wall
else
	PKG_CONFIG_PATH=/usr/lib/pkgconfig
	export PKG_CONFIG_PATH
	CC=gcc -Wall
    CXX=gcc -Wall
endif

TS1 = utc_mm_file_create_tag_attrs_func
TS2 = utc_mm_file_create_content_attrs_func
TS3 = utc_mm_file_destroy_content_attrs_func
TS4 = utc_mm_file_destroy_tag_attrs_func
TS5 = utc_mm_file_get_stream_info_func
TS6 = utc_mm_file_create_content_attrs_simple_func
TS7 = utc_mm_file_create_content_attrs_from_memory_func
TS8 = utc_mm_file_create_tag_attrs_from_memory_func
TS9 = utc_mm_file_get_attrs_func

LIBS = `pkg-config --libs mm-fileinfo glib-2.0`
LIBS +=-L/usr/lib/:/usr/lib/pkgconfig
LIBS +=$(TET_ROOT)/lib/tet3/tcm_s.o
LIBS +=-L$(TET_ROOT)/lib/tet3 -ltcm_s
LIBS +=-L$(TET_ROOT)/lib/tet3/ -lapi_s

INCS = -I. `pkg-config --cflags  mm-fileinfo glib-2.0`
INCS += -I$(TET_ROOT)/inc/tet3
INCS += -I/usr/include/mmf

CFLAGS = $(INCS) 
CC += $(CFLAGS)
LDFLAGS = $(LIBS)

all : $(TS1) $(TS2) $(TS3) $(TS4) $(TS5) $(TS6) $(TS7) $(TS8) $(TS9)

$(TS1): $(TS1).c
	$(CC) -o $(TS1) $(TS1).c $(LDFLAGS)

$(TS2): $(TS2).c
	$(CC) -o $(TS2) $(TS2).c $(LDFLAGS)
	
$(TS3): $(TS3).c
	$(CC) -o $(TS3) $(TS3).c $(LDFLAGS)

$(TS4): $(TS4).c
	$(CC) -o $(TS4) $(TS4).c $(LDFLAGS)

$(TS5): $(TS5).c
	$(CC) -o $(TS5) $(TS5).c $(LDFLAGS)
	
$(TS6): $(TS6).c
	$(CC) -o $(TS6) $(TS6).c $(LDFLAGS)

$(TS7): $(TS7).c
	$(CC) -o $(TS7) $(TS7).c $(LDFLAGS)

$(TS8): $(TS8).c
	$(CC) -o $(TS8) $(TS8).c $(LDFLAGS)

$(TS9): $(TS9).c
	$(CC) -o $(TS9) $(TS9).c $(LDFLAGS)

clean:
	rm -rf *~ *.o $(TS1) $(TS2) $(TS3) $(TS4) $(TS5) $(TS6) $(TS7) $(TS8) $(TS9)
	


