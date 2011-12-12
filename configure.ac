
dnl --------------------------------
dnl Initialization macros.
dnl --------------------------------

AC_INIT(bogio/bogio.h)
AM_CONFIG_HEADER(config.h)

dnl -----------------------------------------------
dnl Package name and version number (user defined)
dnl -----------------------------------------------

GENERIC_LIBRARY_NAME=bogio

#release versioning
GENERIC_MAJOR_VERSION=0
GENERIC_MINOR_VERSION=0
GENERIC_MICRO_VERSION=1

#API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=0.1
AC_SUBST(GENERIC_API_VERSION)

#shared library versioning
GENERIC_LIBRARY_VERSION=0:1:0
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed
#                                  or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed


dnl --------------------------------
dnl Package name and version number
dnl --------------------------------

AC_SUBST(GENERIC_LIBRARY_VERSION)

PACKAGE=$GENERIC_LIBRARY_NAME
AC_SUBST(GENERIC_LIBRARY_NAME)

GENERIC_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION.$GENERIC_MICRO_VERSION
GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)

dnl VERSION=$GENERIC_VERSION
VERSION=$GENERIC_VERSION

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

dnl -----------------------------------------------
dnl Checks for programs.
dnl -----------------------------------------------

AC_PROG_CC
dnl AC_PROG_CXX
AM_PROG_LIBTOOL
AM_SANITY_CHECK

dnl AC_LANG_CPLUSPLUS
AC_LANG_C

dnl -----------------------------------------------
dnl Check for required libraries
dnl -----------------------------------------------
AC_HEADER_STDC
AC_CHECK_HEADERS([comedi.h],, AC_MSG_ERROR([
I can't find the headers for the comedi library
This library is required for compilation.
Bailing out.]))
AC_CHECK_LIB([comedi], [comedi_open],, [AC_MSG_ERROR([
libcomedi is required to build this package,
but it cannot be located. Try setting LDFLAGS
and configuring again, or installing the appropriate
package])], [-lm])


dnl -----------------------------------------------
dnl Generates Makefile's, configuration files and scripts
dnl -----------------------------------------------

AC_OUTPUT(Makefile \
          $PACKAGE.pc \
          bogio/Makefile
)

