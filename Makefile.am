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

.DELETE_ON_ERROR:

ACLOCAL_AMFLAGS = -I m4 --install

AUTOMAKE_OPTIONS =
AUTOMAKE_OPTIONS += -Wno-portability

@GNU@ include $(top_srcdir)/silent-rules.mk

bin_PROGRAMS =
lib_LTLIBRARIES =

CLEANFILES =
DISTCLEANFILES =
EXTRA_DIST =
EXTRA_PROGRAMS =
EXTRA_SCRIPTS =
BUILT_SOURCES =
AM_CPPFLAGS =
AM_CFLAGS =
AM_LIBRS =

EXTRA_DIST += silent-rules.mk
#EXTRA_DIST += COPYING
EXTRA_DIST += README.adoc
#EXTRA_DIST += INSTALL
#EXTRA_DIST += AUTHORS

DISTCLEANFILES += Makefile
DISTCLEANFILES += GNUmakefile

#---------------------------------------------------------------------

lib_LTLIBRARIES += libtge.la
libtge_la_SOURCES =
libtge_la_SOURCES += xalloc.h
libtge_la_SOURCES += spookyhash.c spookyhash.h
libtge_la_SOURCES += persistent_hash_map.h
libtge_la_SOURCES += string_t.c string_t.h

#---------------------------------------------------------------------

.PHONY: indent
indent:
	$(INDENT) -l72 --no-tabs \
		-Tint8_t \
		-Tint16_t \
		-Tint32_t \
		-Tint64_t \
		-Tuint8_t \
		-Tuint16_t \
		-Tuint32_t \
		-Tuint64_t \
		-Tchar32_t \
		-TFILE \
		$(srcdir)/*.{c,h}

#---------------------------------------------------------------------

# dist: $(DISTITEMS)
# 
# DISTITEMS =
# 
# $(srcdir)/README: $(srcdir)/README.adoc
# 	asciidoctor $(<) -o - \
# 		| elinks -dump 1 -no-references -no-numbering > $(@)
# DISTITEMS += $(srcdir)/README
# 
# if HAVE_MERCURIAL
# #
# # The changelogging here is for a Mercurial repository. At the time of
# # this writing I have not yet made public a Mercurial repository.
# # However, the log will include the Git hashes of the changesets.
# #
# # Those who fork the public Git repository may prefer to design their
# # own ChangeLog.
# #
# CHANGELOG_TEMPLATE = "{ifeq(bookmarks,'','','{bookmarks}\n')}{localdate(date, 'UTC')|date|isodate}\n{author}\ngit:       {gitnode}\nmercurial: {node}\nrevision:  {rev}\nparents:   {ifeq(p1.rev,'-1','','{p1.rev}')}{ifeq(p2.rev,'-1','',' {p2.rev}')}\n\n{indent('{desc|fill68}','  ')}\n\n\n"
# $(srcdir)/ChangeLog: $(srcdir)/.hg
# 	$(MERCURIAL) log -G -T $(CHANGELOG_TEMPLATE) -C $(srcdir) \
# 	  > $(@).tmp && mv $(@){.tmp,}
# DISTITEMS += $(srcdir)/ChangeLog
# endif HAVE_MERCURIAL

#---------------------------------------------------------------------
# local variables:
# coding: utf-8
# tab-width: 4
# end:
