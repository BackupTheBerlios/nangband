# Process this file with autoconf to produce a configure script.

# Initialize autoconf - look for the existance of a file to make sure
# the source distribution is installed correctly.
AC_INIT(src/main.c)

# write the resulting #defines into a header file
AM_CONFIG_HEADER(src/autoconf.h)

AM_INIT_AUTOMAKE(nangband, 1.0.0)

AC_ARG_WITH(setgid,
[  --with-setgid=NAME      install nangband as group NAME],
[case "${withval}" in
  no) GAMEGROUP="";;
  yes) AC_MSG_ERROR(missing argument for --with-setgid);;
  *) GAMEGROUP="$withval";;
esac])
AC_SUBST(GAMEGROUP)
AM_CONDITIONAL(SET_GID, test "$GAMEGROUP" != "")

AC_LANG_C

# generate the installation path for the ./lib/ folder
if test "$GAMEGROUP" != ""; then
  MY_EXPAND_DIR(game_datadir, "$datadir/games/$PACKAGE/lib/")
else
  MY_EXPAND_DIR(game_datadir, "./lib/")
  bindir=".."
fi

# overwrite the path with an user-specified value
AC_ARG_WITH(libpath,
[  --with-libpath=path     specify the path to the Angband lib folder],

[case "/$withval" in
  */) game_datadir="$withval"  ;;
  *)  game_datadir="$withval/" ;;
esac])

AC_DEFINE_UNQUOTED(DEFAULT_PATH, "$game_datadir")
DEFAULT_PATH="$game_datadir"
AC_SUBST(DEFAULT_PATH)

# Checks for programs.
AC_PROG_CC
AC_CHECK_TOOL(RANLIB, ranlib)

# -fno-strength-reduce prevents a bug in some versions of gcc
if test "$GCC" = yes; then
  CFLAGS="$CFLAGS -fno-strength-reduce"
fi

# Checks for the borg.
AC_ARG_ENABLE(borg, [  --enable-borg           enable the borg])
if test "X${enable_borg-no}" != Xno ; then
  CFLAGS="$CFLAGS -DALLOW_BORG"
fi

# Checks for libraries.

AC_CYGWIN
AC_MINGW32
AC_EXEEXT

if test "$CYGWIN" = "yes"; then
  AC_DEFINE(WINDOWS)
  LIBS="$LIBS -lwinmm"
  LDFLAGS="$LDFLAGS -s -mno-cygwin -mwindows -e _mainCRTStartup"
  CFLAGS="$CFLAGS -mno-cygwin"
  AM_CONDITIONAL(CYGWIN, test "$CYGWIN" = "yes")
  ANGBAND_OBJS="readdib.o angband.res"
  AC_SUBST(ANGBAND_OBJS)
else
  # SGI (IRIX) may need -lsun for the NIS version of getpwnam
  AC_CHECK_LIB(sun, getpwnam)

  # Check for ncurses, curses, or termcap
  # Try -lncurses, -lcurses, -lcurses -ltermcap, and finally plain -ltermcap
  use_ncurses=no
  AC_CHECK_LIB(ncurses, initscr,
    [AC_CHECK_HEADER(ncurses.h, use_ncurses=yes,
       [AC_CHECK_HEADER(/usr/include/ncurses/ncurses.h,
          [CFLAGS="-I/usr/include/ncurses $CFLAGS" use_ncurses=yes])])])
  if test "$use_ncurses" = yes; then
    AC_DEFINE(USE_GCU) AC_DEFINE(USE_NCURSES)
    LIBS="-lncurses $LIBS"
  else
    AC_CHECK_LIB(curses, initscr,
      [AC_DEFINE(USE_GCU) LIBS="-lcurses $LIBS"],
      [AC_CHECK_LIB(curses, endwin,
         [AC_DEFINE(USE_GCU) LIBS="-lcurses -ltermcap $LIBS"],
         [AC_CHECK_LIB(termcap, tgetent,
            [AC_DEFINE(USE_CAP) LIBS="-ltermcap $LIBS"])],
         -ltermcap)])
  fi

  # Check for GTK
  if test "X${enable_gtk-no}" != Xno ; then
    AC_DEFINE(USE_GTK)

    AC_CACHE_CHECK(gtk-config --libs, ac_cv_gtk_libs,
      [ac_cv_gtk_libs=`(  gtk-config --libs  ) 2>&5` || ac_cv_gtk_libs=""])
    AC_CACHE_CHECK(gtk-config --cflags, ac_cv_gtk_cflags,
      [ac_cv_gtk_cflags=`(gtk-config --cflags) 2>&5` || ac_cv_gtk_cflags="no"])
    if test "X$ac_cv_gtk_libs" = X || test "X$ac_cv_gtk_cflags" = Xno; then
      AC_MSG_ERROR([Could not find GTK.])
    fi
    CFLAGS="$ac_cv_gtk_cflags $CFLAGS"
    LIBS="$ac_cv_gtk_libs $LIBS"

    AC_CACHE_CHECK(for gtk_widget_show, ac_cv_lib_gtk_gtk_widget_show,
      [AC_TRY_LINK([
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <X11/Xlib.h>], [gtk_widget_show((GtkWidget *)0);],
		   [ac_cv_lib_gtk_gtk_widget_show=yes],
		   [AC_MSG_ERROR([Failed to link GTK.])])])
  fi

  if test "x${enable_gtk-no}" = xno ||
     test "x${with_x+set}" != x ||
     test "x$x_includes" != xNONE || test "x$x_libraries" != xNONE
  then
    # Checks for X window system.
    AC_PATH_XTRA
  fi

  AC_ARG_ENABLE(gtk,  [  --enable-gtk            enable the GTK X11 module])

  if test "x${enable_gtk-no}" != xno || test "$no_x" != "yes"; then
    ANGBAND_OBJS="maid-x11.o"
    AC_SUBST(ANGBAND_OBJS)
  fi

  if test "$no_x" != "yes"; then
    AC_DEFINE(USE_X11)
    AC_DEFINE(USE_XPJ)

    LIBS="-lX11 $X_EXTRA_LIBS $LIBS"
    CFLAGS="$CFLAGS $X_CFLAGS"
    LDFLAGS="$LDFLAGS $X_LIBS"
    # X_PRE_LIBS is prepended to LIBS below.

    # Check for the Athena widget set
    #
    # Does anyone know if it is OK to put '-lXmu -lXt' *before* X_PRE_LIBS?
    # If so this code can be simplified by inserting X_PRE_LIBS in LIBS above.
    # Today libraries come in the same sequence as in the old src/Makefile.std,
    # since that is presumably the way which has been tested most.
    x_more_libs="$X_PRE_LIBS -lXt"
    AC_CHECK_LIB(Xmu, main, x_more_libs="$X_PRE_LIBS -lXmu -lXt", , $x_more_libs)
    AC_CHECK_LIB(Xext, main, x_more_libs="-lXext $x_more_libs", , $x_more_libs)

    # Note: Both Xaw libraries and includes may be in nostandard places.
    ac_save_LIBS="$LIBS"
    LIBS="-lXaw $x_more_libs $LIBS"
    AC_CACHE_CHECK(for X11 Athena widget set, ac_cv_athena_widget,
                   [AC_TRY_LINK([
#include <X11/IntrinsicP.h>
#include <X11/Xaw/SimpleP.h>
#include <X11/Xaw/Simple.h>
#include <X11/Xaw/XawInit.h>],  [XawInitializeWidgetSet();],
                                [ac_cv_athena_widget=yes],
                                [ac_cv_athena_widget=no])])
    case "$ac_cv_athena_widget" in
    yes) AC_DEFINE(USE_XAW) ;;
    *)	  LIBS="$X_PRE_LIBS $ac_save_LIBS" ;;
    esac
  fi
fi

AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h limits.h strings.h sys/file.h sys/ioctl.h \
sys/time.h termio.h unistd.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(gethostname mkdir select strstr strtol usleep mkstemp setegid)

AC_OUTPUT(Makefile src/Makefile src/lua/Makefile lib/Makefile lib/apex/Makefile lib/bone/Makefile lib/data/Makefile lib/edit/Makefile lib/file/Makefile lib/help/Makefile lib/help/err/Makefile lib/help/coding/Makefile lib/info/Makefile lib/pref/Makefile lib/save/Makefile lib/script/Makefile lib/user/Makefile lib/xtra/Makefile)
