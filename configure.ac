#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# generate　configure :
# run 
#  aclocal ; autoreconf -i ; automake --add-missing 
#  or
#  autoreconf -i ; automake --add-missing

# AC_PREREQ([2.69])
AC_INIT(nysol_mcmdex, 3.0, info@nysol.jp)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([mbonsai/cmd/aiqueue.cpp])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([floor isascii memset mkdir mkfifo modf pow select setlocale sqrt strchr strerror strstr])


XML2HEAD=`xml2-config --cflags`
XML2LIB=`xml2-config --libs`
AC_SUBST(XML2HEAD)
AC_SUBST(XML2LIB)

# BOOST PATH　指定
AC_ARG_WITH([boost],
  AC_HELP_STRING(
    [--with-boost=PATH],[PATH is path to boost library]),
  [BOOSTPATH=$with_boost],[BOOSTPATH=/usr/local])

CXXFLAGS="-I $BOOSTPATH/include $CXXFLAGS"

AC_SUBST([BOOSTPATH], [$BOOSTPATH])

AC_CHECK_LIB([kgmod3], [main], [], [echo "Error. You need to have libkgmod3";exit -1])
AC_CHECK_LIB([boost_system], [main], [], [echo "Error. You need to have libboost";exit -1])
AC_CHECK_LIB([boost_filesystem], [main], [], [echo "Error. You need to have libboost";exit -1])


#OSチェック
AM_CONDITIONAL(DARWIN_CERT, false)
AC_CANONICAL_HOST
case $host_os in
  darwin* )
		#証明書チェック
		AS_IF([test "x$(security find-identity -p codesigning -v | grep nysol-cert | head -n 1)" != x],
			[AM_CONDITIONAL(DARWIN_CERT, true)]
		)
		;;
esac

# configure parameter
AC_ARG_ENABLE([jpnformat],
	[AS_HELP_STRING([--enable-jpnformat],
	    [jpnformat (default is yes)])],
	[],
	[enable_jpnformat=yes] 
)
AS_IF([test "x$enable_jpnformat" != xno],
    [AC_DEFINE([JPN_FORMAT], [1], [jpnformat])]
)

AC_CONFIG_FILES([Makefile
								 mbonsai/Makefile
                 mcm/Makefile
                 medist/Makefile
                 mroc/Makefile
                 mlcs/Makefile
                 mkmeans/Makefile
								 mbonsai/cmd/Makefile
                 mcm/cmd/Makefile
                 medist/cmd/Makefile
                 mroc/cmd/Makefile
                 mlcs/cmd/Makefile
                 mkmeans/cmd/Makefile])

AC_OUTPUT


