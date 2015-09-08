#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([libmm-fileinfo],[0.0.1])
AC_CONFIG_SRCDIR([mm_file.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lavcodec':
#AC_CHECK_LIB([avcodec], [main])
# FIXME: Replace `main' with a function in `-lavformat':
#AC_CHECK_LIB([avformat], [main])
# FIXME: Replace `main' with a function in `-lavutil':
#AC_CHECK_LIB([avutil], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT64_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gettimeofday memset munmap strcasecmp strdup])

PKG_CHECK_MODULES(MMCOMMON,mm-common)
AC_SUBST(MMCOMMON_CFLAGS)
AC_SUBST(MMCOMMON_LIBS)

PKG_CHECK_MODULES(MMLOG,mm-log)
AC_SUBST(MMLOG_CFLAGS)
AC_SUBST(MMLOG_LIBS)

PKG_CHECK_MODULES(GLIB, glib-2.0)
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(GTK, gtk+-2.0, [HAVE_GTK=yes], [HAVE_GTK=no])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(AVCODEC, libavcodec)
AC_SUBST(AVCODEC_CFLAGS)
AC_SUBST(AVCODEC_LIBS)

PKG_CHECK_MODULES(AVUTIL, libavutil)
AC_SUBST(AVUTIL_CFLAGS)
AC_SUBST(AVUTIL_LIBS)

PKG_CHECK_MODULES(AVFORMAT, libavformat)
AC_SUBST(AVFORMAT_CFLAGS)
AC_SUBST(AVFORMAT_LIBS)

PKG_CHECK_MODULES(SWSCALE, libswscale)
AC_SUBST(SWSCALE_CFLAGS)
AC_SUBST(SWSACLE_LIBS)

PKG_CHECK_MODULES(ICU, icu-i18n)
AC_SUBST(ICU_CFLAGS)
AC_SUBST(ICU_LIBS)

PKG_CHECK_MODULES(VCONF, vconf)
AC_SUBST(VCONF_CFLAGS)
AC_SUBST(VCONF_LIBS)

dnl use dyn --------------------------------------------------------------------------
AC_ARG_ENABLE(dyn, AC_HELP_STRING([--enable-dyn], [using dyn]),
[
   case "${enableval}" in
      yes) USE_DYN=yes ;;
       no) USE_DYN=no ;;
        *) AC_MSG_ERROR(bad value ${enableval} for --enable-dyn) ;;
   esac
],[USE_DYN=yes])
AM_CONDITIONAL(USE_DYN, test "x$USE_DYN" = "xyes")
AC_MSG_RESULT(checking for USE_DYN ... $USE_DYN)

dnl use iommap --------------------------------------------------------------------------
AC_ARG_ENABLE(iommap, AC_HELP_STRING([--enable-iommap], [using iommap]),
[
   case "${enableval}" in
       yes) USE_IOMMAP=yes ;;
        no) USE_IOMMAP=no ;;
         *) AC_MSG_ERROR(bad value ${enableval} for --enable-iommap) ;;
   esac
],[USE_IOMMAP=no])
AM_CONDITIONAL(USE_IOMMAP, test "x$USE_IOMMAP" = "xyes")
AC_MSG_RESULT(checking for USE_IOMMAP ... $USE_IOMMAP)
	 
dnl use testmode  --------------------------------------------------------------------------
AC_ARG_ENABLE(testmode, AC_HELP_STRING([--enable-testmode], [using testmode]),
[
      case "${enableval}" in
              yes) USE_TESTMODE=yes ;;
              no)  USE_TESTMODE=no ;;
              *)   AC_MSG_ERROR(bad value ${enableval} for --enable-testmode) ;;
      esac
],[USE_TESTMODE=no])
AM_CONDITIONAL(USE_TESTMODE, test "x$USE_TESTMODE" = "xyes")
AC_MSG_RESULT(checking for USE_TESTMODE ... $USE_TESTMODE)

dnl use thumbnail dump  --------------------------------------------------------------------------
AC_ARG_ENABLE(dump, AC_HELP_STRING([--enable-dump], [using dump]),
[
      case "${enableval}" in
              yes) USE_DUMP=yes ;;
              no)  USE_DUMP=no ;;
              *)   AC_MSG_ERROR(bad value ${enableval} for --enable-dump) ;;
      esac
],[USE_DUMP=no])
AM_CONDITIONAL(USE_DUMP, test "x$USE_DUMP" = "xyes")
AC_MSG_RESULT(checking for USE_DUMP ... $USE_DUMP)


dnl use disable gtk  --------------------------------------------------------------------------
AC_ARG_ENABLE(gtk, AC_HELP_STRING([--enable-gtk], [using gtk]),
[
      case "${enableval}" in
              yes) USE_GTK=yes ;;
              no)  USE_GTK=no ;;
              *)   AC_MSG_ERROR(bad value ${enableval} for --enable-gtk) ;;
      esac
],[USE_GTK=yes])
AM_CONDITIONAL(USE_GTK, test "x$USE_GTK" = "xyes")
AC_MSG_RESULT(checking for USE_GTK ... $USE_GTK)


AC_CONFIG_FILES([Makefile
                 codecs/Makefile
                 codecs/ffmpeg/Makefile
                 formats/Makefile
                 formats/ffmpeg/Makefile
                 utils/Makefile
                 tests/Makefile
		 mm-fileinfo.pc
])
AC_OUTPUT
