EXTRA_DIST =
CLEANFILES =
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DLIBEXECDIR=\""$(libexecdir)"\" \
	-I${top_srcdir}/src/cexcept \
	-I${top_srcdir}/src

AM_CFLAGS = ${my_CFLAGS} \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections

AM_LDFLAGS = \
	-Wl,--gc-sections \
	-Wl,--as-needed

SED_PROCESS = \
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(SED) \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@exec_prefix\@,$(exec_prefix),g' \
	-e 's,@libdir\@,$(libdir),g' \
	-e 's,@includedir\@,$(includedir),g' \
	< $< > $@ || rm $@

%.pc: %.pc.in Makefile
	$(SED_PROCESS)

LIBCEXCEPT_CURRENT=0
LIBCEXCEPT_REVISION=0
LIBCEXCEPT_AGE=0

pkginclude_HEADERS = \
	src/cexcept/cleanups.h \
	src/cexcept/exceptions.h \
	src/cexcept/libcexcept.h

lib_LTLIBRARIES = src/libcexcept.la

src_libcexcept_la_SOURCES =\
	src/libcexcept-private.h \
	src/cleanups.c \
	src/exceptions.c

EXTRA_DIST += src/libcexcept.sym

src_libcexcept_la_LDFLAGS = $(AM_LDFLAGS) \
	-version-info $(LIBCEXCEPT_CURRENT):$(LIBCEXCEPT_REVISION):$(LIBCEXCEPT_AGE) \
	-Wl,--version-script=$(top_srcdir)/src/libcexcept.sym
src_libcexcept_la_DEPENDENCIES = ${top_srcdir}/src/libcexcept.sym

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = src/libcexcept.pc
EXTRA_DIST += src/libcexcept.pc.in
CLEANFILES += src/libcexcept.pc

TESTS = src/test-libcexcept

check_PROGRAMS = src/test-libcexcept
src_test_libcexcept_SOURCES = src/test-libcexcept.c src/test-libcexcept.h
src_test_libcexcept_LDADD = src/libcexcept.la
