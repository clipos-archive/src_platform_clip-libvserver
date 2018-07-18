# SPDX-License-Identifier: GPL-2.0
# Copyright © 2007-2018 ANSSI. All Rights Reserved.
AC_PREREQ(2.59)

m4_define(clipvserver_version_major, 4)
m4_define(clipvserver_version_minor, 1)
m4_define(clipvserver_version_micro, 9)
m4_define(clipvserver_lib_minor, 15)

AC_INIT([libclipvserver],
      clipvserver_version_major.clipvserver_version_minor.clipvserver_version_micro,
      [clipos@ssi.gouv.fr])
AC_CONFIG_SRCDIR(src/clip-vserver.h)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.10])

VERSION_INFO=clipvserver_version_major:clipvserver_lib_minor:0
AC_SUBST(VERSION_INFO)

CLIP_VERSION_MAJOR=clipvserver_version_major()
CLIP_VERSION_MINOR=clipvserver_version_minor()
CLIP_VERSION_MICRO=clipvserver_version_micro()
AC_SUBST(CLIP_VERSION_MAJOR)
AC_SUBST(CLIP_VERSION_MINOR)
AC_SUBST(CLIP_VERSION_MICRO)

dnl AM_MAINTAINER_MODE

AC_PROG_CC
AM_PROG_LIBTOOL
AC_STDC_HEADERS
AC_PROG_INSTALL

PKG_PROG_PKG_CONFIG

dnl ===========================================================================
dnl === Local macros
dnl ===========================================================================

# _CHECK_FUNCS_WITH_FLAGS(FUNCTION..., CFLAGS, LIBS 
#                         [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
# Like AC_CHECK_FUNCS but with additional CFLAGS and LIBS
# --------------------------------------------------------------------
AC_DEFUN([_CHECK_FUNCS_WITH_FLAGS],
[ 
  save_cflags="$CFLAGS"
  save_libs="$LIBS"
  CFLAGS="$CFLAGS $2"
  LIBS="$LIBS $3"
  AC_CHECK_FUNCS($1, $4, $5)
  CFLAGS="$save_cflags"
  LIBS="$save_libs"
])

dnl ===========================================================================
AC_ARG_WITH(kernel-includes,
	AC_HELP_STRING([--with-kernel-includes=path],
		[Use path for kernel includes (default: /usr/src/linux/include/uapi)]),
		[KERNEL_INCLUDES=$withval],
		[KERNEL_INCLUDES=/usr/src/linux/include/uapi])
AC_SUBST(KERNEL_INCLUDES)

AC_ARG_ENABLE(html-doc, 
	AC_HELP_STRING([--enable-html-doc],[create and install html documentation (default=no)]),
		enable_html_doc=$enableval, enable_html_doc=no)
AC_MSG_CHECKING(if html documentation should be created)
if test "x$enable_html_doc" = xyes; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(HTML_DOC, test "x$enable_html_doc" = xyes)

AC_ARG_ENABLE(net-ns, 
	AC_HELP_STRING([--enable-net-ns], [build the library with support for NET namespaces (default=no)]),
		enable_net_ns=$enableval, enable_net_ns=no)
AC_MSG_CHECKING(if the library should be built with support for NET namespaces)
if test "x$enable_net_ns" = xyes; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
AM_CONDITIONAL(NET_NS, test "x$enable_net_ns" = xyes)

dnl ===========================================================================

AC_CHECK_HEADERS([stdint.h inttypes.h sys/int_types.h])
AC_CHECK_TYPES([uint64_t, uint128_t])

dnl Use lots of warning flags with GCC

WARN_CFLAGS=""

if test "x$GCC" = "xyes"; then
	WARN_CFLAGS="-Wall -Wextra -Werror \
	-Wstrict-prototypes -Wmissing-prototypes \
	-Wcast-qual -Wcast-align -Wpointer-arith \
	-Wwrite-strings \
	-Wnested-externs"
fi

CLIP_CFLAGS="$CLIP_CFLAGS $WARN_CFLAGS"

AC_SUBST(CLIP_CFLAGS)

dnl ===========================================================================

AC_OUTPUT([
clip-vserver.pc
Makefile
src/Makefile
docs/Makefile
])
