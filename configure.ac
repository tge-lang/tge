# Copyright © 2026 Barry Schwartz
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

AC_PREREQ([2.72])
AC_INIT([tge],[0.0.0])

AC_CONFIG_SRCDIR([tge_gc.c])

#AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_BUILD                 dnl On what platform are we compiling?
AC_CANONICAL_HOST                  dnl For what platform are we compiling?
AC_USE_SYSTEM_EXTENSIONS           dnl Define such macros as _GNU_SOURCE.

AM_INIT_AUTOMAKE([foreign tar-pax no-dist-gzip dist-xz])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE

LT_PREREQ([2.5.4])
LT_INIT([pic-only,dlopen])

AC_ARG_ENABLE([color],
  [AS_HELP_STRING([--disable-color],
    [disable color in test outputs])],
  [enable_color="${enableval}"],
  [enable_color=yes])
AC_SUBST([enable_color])

AC_PROG_CC
AC_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_AWK
AC_PROG_SED
AC_PROG_GREP
AC_PROG_FGREP
AC_PROG_EGREP

AC_SUBST([INDENT_FLAGS])

for i in 8 16 32 64; do
    INDENT_FLAGS="${INDENT_FLAGS}${INDENT_FLAGS+ }-Tint${i}_t -Tuint${i}_t"
done
for i in 8 16 32; do
    INDENT_FLAGS="${INDENT_FLAGS}${INDENT_FLAGS+ }-Tchar${i}_t"
done

CHEM_C23_FLAGS([c23_flags])
CFLAGS="${CFLAGS}${CFLAGS+ }${c23_flags}"
CHEM_GNU_C_FLAGS([hidden_flags],[-fPIC -fvisibility=hidden])
CFLAGS="${CFLAGS}${CFLAGS+ }${hidden_flags}"

CHEM_GNU_C_FLAGS([visible_flags],[-fPIC -fvisibility=default])
AC_SUBST([VISIBLE_CFLAGS],["${visible_flags}"])

m4_include([c_attributes.ac])

CHEM_PATH_PROGS_CACHED_AND_PRECIOUS([INDENT],
  [GNU indent],[indent],
  [
    if LC_ALL=C LANG=C ${ac_path_INDENT} --version 2>&1 | \
      LC_ALL=C LANG=C ${GREP} -e '^GNU indent' \
               2> /dev/null > /dev/null; then
      ac_cv_path_INDENT="${ac_path_INDENT}"
      ac_path_INDENT_found=:
    fi
  ])

# CHEM_PATH_PROGS_CACHED_AND_PRECIOUS([MERCURIAL],
#   [Mercurial Distributed SCM],[hg],
#   [
#     if LC_ALL=C LANG=C ${ac_path_MERCURIAL} --version 2>&1 | \
#       LC_ALL=C LANG=C ${GREP} -e '^Mercurial Distributed SCM' \
#                2> /dev/null > /dev/null; then
#       ac_cv_path_MERCURIAL="${ac_path_MERCURIAL}"
#       ac_path_MERCURIAL_found=:
#     fi
#   ])
# AM_CONDITIONAL([HAVE_MERCURIAL],
#         [test "${chem_cv_path_MERCURIAL_found}" = yes])

# CHEM_PROG_GNU_M4
# if test -z "${GNU_M4}"; then
#    AC_MSG_ERROR([GNU m4 not found. Perhaps set GNU_M4 to OpenBSD m4.])
# fi

# CHEM_UNIQUE_ID([UNIQUE_ID])

# host_is_x86=no
# if test x${host_cpu} = xi386; then
#   host_is_x86=yes
# elif test x${host_cpu} = xx86_64; then
#   host_is_x86=yes
# elif test x${host_cpu} = xx86_64-x32; then
#   host_is_x86=yes
# fi
# 
# if test x${host_is_x86} = xyes; then
#   AC_CACHE_CHECK([for __builtin_ia32_pause],
#                  [hiha_cv_func_have___builtin_ia32_pause],
#                  [hiha_cv_func_have___builtin_ia32_pause=no
#                   AC_COMPILE_IFELSE([
#                     AC_LANG_PROGRAM([[]],[[__builtin_ia32_pause();]])
#                   ],
#                   [hiha_cv_func_have___builtin_ia32_pause=yes])])
#   if test x"${hiha_cv_func_have___builtin_ia32_pause}" = xyes; then
#     AC_DEFINE([HAVE___BUILTIN_IA32_PAUSE],[1],
#               [Define if __builtin_ia32_pause exists.])
#   fi
#   AC_SUBST([have___builtin_ia32_pause],
#            ["${hiha_cv_func_have___builtin_ia32_pause}"])
# fi
# 
# if test x${host_is_x86} = xyes; then
#   AC_CACHE_CHECK([for _mm_pause],
#                  [hiha_cv_func_have__mm_pause],
#                  [hiha_cv_func_have__mm_pause=no
#                   AC_COMPILE_IFELSE([
#                     AC_LANG_PROGRAM([[#include <immintrin.h>]],
#                                     [[_mm_pause();]])
#                   ],
#                   [hiha_cv_func_have__mm_pause=yes])])
#   if test x"${hiha_cv_func_have__mm_pause}" = xyes; then
#     AC_DEFINE([HAVE__MM_PAUSE],[1],
#               [Define if _mm_pause exists.])
#   fi
#   AC_SUBST([have__mm_pause],
#            ["${hiha_cv_func_have__mm_pause}"])
# fi

# You can use ‘@GNU@’ to make GNU Make constructs unbothersome to
# Automake. (Often you can use ‘$(eval ...)’ instead, but @GNU@ is
# more straightforward.)
AC_SUBST([GNU],[])

CHEM_REQUIRE_GNU_MAKE_IN_PATH
CHEM_CONFIG_MAKEFILES
AC_CONFIG_FILES([atconfig])

AC_OUTPUT

# local variables:
# coding: utf-8
# end:
