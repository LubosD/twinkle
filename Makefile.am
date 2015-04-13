# The Makefile for the Qt-part of the program is created by qmake.
# That Makefile builts the executable. 
# So here the executable is defined as a script such that it will
# only be installed and not built.

ACLOCAL_AMFLAGS = -I m4

pkglangdir = $(pkgdatadir)/lang

bin_SCRIPTS = src/gui/twinkle
noinst_SCRIPTS = twinkle.desktop

pkgdata_DATA = data/ringback.wav data/ringtone.wav \
	data/providers.csv \
	src/gui/images/twinkle16.png src/gui/images/twinkle32.png \
	src/gui/images/twinkle48.png

EXTRA_DIST = src/gui/*.ui src/gui/*.h src/gui/*.cpp src/gui/Makefile \
	src/gui/twinkle.pro src/gui/images/* \
	data/ringback.wav data/ringtone.wav data/providers.csv \
	m4/*.m4 \
	sip.protocol twinkle.desktop.in Doxyfile

SUBDIRS = src

edit = sed \
	-e 's,@datadir\@,$(pkgdatadir),g' \
	-e 's,@prefix\@,$(prefix),g'
	
twinkle.desktop: Makefile $(srcdir)/twinkle.desktop.in
	$(edit) $(srcdir)/twinkle.desktop.in >twinkle.desktop
