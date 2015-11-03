## Source directory

# SUBDIRS = sub

h_sources = bogio.h
c_sources = bogio.c

# library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
library_includedir=$(includedir)
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir)

lib_LTLIBRARIES= libbogio.la
libbogio_la_SOURCES= $(h_sources) $(c_sources)
# libbogio_0_0_la_LIBADD= sub/libsub.la
libbogio_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)

