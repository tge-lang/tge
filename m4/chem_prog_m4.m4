# -*- autoconf -*-
#
# Copyright (C) 2013, 2018, 2025 Barry Schwartz and Khaled Hosny
# 
# Copying and distribution of this file, with or without modification,
# are permitted in any medium without royalty provided the copyright
# notice and this notice are preserved.  This file is offered as-is,
# without any warranty.

# serial 1

# CHEM_PROG_M4
# ------------
#
# Set M4 to the path of the first m4 in the PATH, or to an empty
# string if m4 is not found. The result is cached in
# ac_cv_path_M4. The test may be overridden by setting M4 or the cache
# variable.
#
# No checking that this is really an m4 is done, because running m4
# from inside another run of m4 is problematic. For the same reason,
# no special attempt is made to find GNU m4 instead of some other
# implementation.
#
AC_DEFUN([CHEM_PROG_M4],[{ :
   CHEM_PATH_PROGS_CACHED_AND_PRECIOUS([M4],[m4 macro processor],
      [m4],[
         ac_cv_path_M4="${ac_path_M4}"
         ac_path_M4_found=:
      ])
}])

# CHEM_PROG_GNU_M4
# ----------------
#
# Set GNU_M4 to the path of the first GNU m4 in the PATH, or to an
# empty string if GNU m4 is not found. The result is cached in
# chem_cv_path_GNU_M4. The test may be overridden by setting GNU_M4 or
# the cache variable.
#
# Running m4 from inside another run of m4 is problematic, so the
# checking is done by running `m4 --version' and examining the
# message, rather than by running an m4 script.
#
AC_DEFUN([CHEM_PROG_GNU_M4],[{ :
   AC_REQUIRE([AC_PROG_GREP])
   CHEM_PATH_PROGS_CACHED_AND_PRECIOUS([GNU_M4],[GNU m4 macro processor],
      [m4 gm4 gnum4],[
         if LC_ALL=C LANG=C ${ac_path_GNU_M4} --version 2>&1 | \
               LC_ALL=C LANG=C ${GREP} -i '^m4 (GNU M4)' 2> /dev/null > /dev/null; then
            ac_cv_path_GNU_M4="${ac_path_GNU_M4}"
            ac_path_GNU_M4_found=:
         fi
      ])
}])
