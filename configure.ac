AC_INIT(jload, 0.1, joewing@joewing.net)
AC_PREREQ(2.57)
AC_CONFIG_HEADER([config.h])
AC_LANG(C)

AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MKDIR_P

AC_PATH_X
if test ! "$no_x" = "yes" ; then
   if test ! x"$x_libraries" = x ; then
      LDFLAGS="$LDFLAGS -L$x_libraries"
   fi
   if test ! x"$x_includes" = x ; then
      CFLAGS="$CFLAGS -I$x_includes"
   fi
else
   AC_MSG_ERROR([Could not find X11])
fi
AC_CHECK_LIB([X11], XOpenDisplay,
   [ LDFLAGS="$LDFLAGS -lX11" ],
   [ AC_MSG_ERROR([libX11 not found]) ])

AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([sys/types.h sys/sysctl.h])
AC_CHECK_HEADERS([sys/mpctl.h])
AC_CHECK_HEADERS([sys/sysget.h])
AC_CHECK_FUNCS([sysconf sysctl mpctl sysget])

AC_CHECK_HEADERS([loadavg.h sys/loadavg.h])
AC_CHECK_FUNCS([getloadavg])

AC_SUBST(CFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(BINDIR)

AC_OUTPUT(Makefile)
AC_OUTPUT(src/Makefile)
