AC_PREREQ([2.69])
AC_INIT([clk], [1.0])
AC_CONFIG_SRCDIR([src/main.cpp])

# The makefile selects -O2 or DEBUG's -g -O0, so suppress Autoconf's
# default -g -O2 while retaining any flags explicitly supplied by the user.
AS_IF([test "${CFLAGS+set}" != set], [CFLAGS=])
AS_IF([test "${CXXFLAGS+set}" != set], [CXXFLAGS=])
AC_PROG_CC
AC_PROG_CXX

AC_LANG_PUSH([C++])
clk_saved_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS -std=c++20"
AC_MSG_CHECKING([whether $CXX supports C++20])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if __cplusplus < 202002L
#error C++20 is required
#endif
]], [[]])], [AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no])
   AC_MSG_ERROR([a C++20 compiler is required])])
CXXFLAGS=$clk_saved_CXXFLAGS
AC_LANG_POP([C++])

AC_CHECK_HEADERS([gcrypt.h png.h], [],
  [AC_MSG_ERROR([libgcrypt and libpng development headers are required; set CPPFLAGS for nonstandard include directories])])
AC_CHECK_LIB([gcrypt], [gcry_check_version], [:],
  [AC_MSG_ERROR([libgcrypt is required; set LDFLAGS for nonstandard library directories])])
AC_CHECK_LIB([png], [png_create_read_struct], [:],
  [AC_MSG_ERROR([libpng is required; set LDFLAGS for nonstandard library directories])])

AC_ARG_VAR([PANDOC], [Markdown-to-man-page converter])
AC_CHECK_PROG([PANDOC], [pandoc], [pandoc], [pandoc])
AC_ARG_VAR([VBUILD], [version source generator (needed only for make version)])
AC_CHECK_PROG([VBUILD], [vbuild], [vbuild], [vbuild])

AC_CONFIG_FILES([makefile])
AC_OUTPUT
