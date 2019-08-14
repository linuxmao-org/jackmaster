dnl Process this file with autoconf to produce a configure script.

AC_INIT([jackmaster], [0.0.2])
AC_CONFIG_SRCDIR([src/main.c])
dnl AC_CANONICAL_TARGET
AM_CONFIG_HEADER(ac_cfg.h)
AM_INIT_AUTOMAKE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_LIBTOOL


STD_OPT_CFLAGS=""

MAX_OPT_CFLAGS="-O3 -ffast-math -ftracer"

# for gcc3
#MAX_OPT_CFLAGS="$MAX_OPT_CFLAGS -fold-unroll-loops"

# This actually hurts with buffer-sizes 16-64
MAX_OPT_CFLAGS="$MAX_OPT_CFLAGS -fprefetch-loop-arrays"


PKG_CHECK_MODULES(GTK, gtk+-2.0 gthread-2.0)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)
AC_DEFINE(USE_GTK_2_0, 1, [Use GTK 2.x])


PKG_CHECK_MODULES(JACK, jack >= 0.80.0)
AC_SUBST(JACK_CFLAGS)
AC_SUBST(JACK_LIBS)


cfg_enable_alsa="yes"
AC_ARG_ENABLE(alsa,
[  --disable-alsa        disable ALSA support],[
  case "$enableval" in
    "yes") ;;
    "no")  cfg_enable_alsa="no" ;;
    *)     AC_MSG_ERROR([must use --enable-alsa(=yes/no) or --disable-alsa]) ;;
  esac
])
if test "$cfg_enable_alsa" = "yes"; then
  AM_PATH_ALSA(0.9.0,[
    AC_DEFINE(HAVE_ALSA, 1, [Whether or not we are supporting ALSA])
#    ALSA_OBJS=alsa_if.$OBJEXT
#    AC_SUBST(ALSA_OBJS)
  ])
else
  AC_MSG_WARN([ALSA support is disabled])
  unset ALSA_CFLAGS
  unset ALSA_LIBS
  AC_SUBST(ALSA_CFLAGS)
  AC_SUBST(ALSA_LIBS)
fi
AM_CONDITIONAL(ALSA, test "x$HAVE_ALSA" != "x")


cfg_enable_lash="yes"
AC_ARG_ENABLE(lash,
[  --disable-lash        disable LASH support],[
  case "$enableval" in
    "yes") ;;
    "no")  cfg_enable_lash="no" ;;
    *)     AC_MSG_ERROR([must use --enable-lash(=yes/no) or --disable-lash]) ;;
  esac
])
if test "$cfg_enable_lash" = "yes"; then
  PKG_CHECK_MODULES(LASH, lash-1.0 >= 0.5.0, LASH_FOUND="yes", LASH_FOUND="no")
  if test "$LASH_FOUND" = "yes"; then
    AC_DEFINE(HAVE_LASH, 1, [whether or not we are supporting lash])
    LASH_VERSION=$( pkg-config --modversion lash-1.0 )
    AC_DEFINE_UNQUOTED(LASH_VERSION, "$LASH_VERSION", [The version of lash we're compiling against])
    AC_SUBST(LASH_CFLAGS)
    AC_SUBST(LASH_LIBS)
  fi
else
  AC_MSG_WARN([LASH support is disabled])
fi


AC_MSG_CHECKING([whether 0.0f is 'all bits 0'])
ok=0
AC_RUN_IFELSE(
    [AC_LANG_PROGRAM([], [
float f= 0.0; char * fp= &f; int x;
for (x=0; x<sizeof(float); x++) if (*fp++!=0) exit(1);
exit(0);
    ])],
    [AC_MSG_RESULT([yes]); ok=1],
    [AC_MSG_RESULT([no])],
    [AC_MSG_RESULT([cross compiling - assuming no.])]
)
if test "x${ok}" = "x1"; then
AC_DEFINE(FAST_FLOAT_0, 1, [whether 0.0f is 'all bits 0'
(if you have very small jack buffers, it might actually be slower to use
 this (depending on the compiler, optimization and libc). On my Athlon XP
 with gcc 3.4, -O3 and GNU libc 2.3.2 however, the break-even-point would
 have been around 8 frames per period)])
fi


dnl Set DATA_ROOT_DIR in config.h.
if test "x${prefix}" = "xNONE"; then
  data_root_dir="/usr/local/share"
else
  data_root_dir="${prefix}/share"
fi
AC_DEFINE_UNQUOTED(DATA_ROOT_DIR, "$data_root_dir", [Directory for non-binary data])


dnl Set PACKAGE_SOURCE_DIR in config.h.
packagesrcdir=`cd $srcdir && pwd`
AC_DEFINE_UNQUOTED(PACKAGE_SOURCE_DIR, "$packagesrcdir", [Source directory])


dnl Use -Wall if we have gcc.
changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
    *[\ \	]-Wall[\ \	]*) ;;
    *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl

CFLAGS="$CFLAGS $STD_OPT_CFLAGS"
AC_SUBST(MAX_OPT_CFLAGS)

AC_OUTPUT([
Makefile
src/Makefile
])
