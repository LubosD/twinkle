pkglangdir = $(pkgdatadir)/lang

pkglang_DATA = \
	twinkle_nl.qm \
	twinkle_de.qm \
	twinkle_cs.qm \
	twinkle_fr.qm \
	twinkle_ru.qm \
	twinkle_sv.qm

CLEANFILES = $(pkglang_DATA)

EXTRA_DIST = \
	twinkle_nl.ts \
	twinkle_de.ts \
	twinkle_cs.ts \
	twinkle_fr.ts \
	twinkle_ru.ts \
	twinkle_sv.ts \
	twinkle_xx.ts

twinkle_nl.qm: $(top_builddir)/src/gui/twinkle $(top_srcdir)/src/gui/twinkle.pro
	lrelease -compress $(srcdir)/twinkle_nl.ts -qm $@

twinkle_de.qm: $(top_builddir)/src/gui/twinkle $(top_srcdir)/src/gui/twinkle.pro
	lrelease -compress $(srcdir)/twinkle_de.ts -qm $@

twinkle_cs.qm: $(top_builddir)/src/gui/twinkle $(top_srcdir)/src/gui/twinkle.pro
	lrelease -compress $(srcdir)/twinkle_cs.ts -qm $@

twinkle_fr.qm: $(top_builddir)/src/gui/twinkle $(top_srcdir)/src/gui/twinkle.pro
	lrelease -compress $(srcdir)/twinkle_fr.ts -qm $@

twinkle_ru.qm: $(top_builddir)/src/gui/twinkle $(top_srcdir)/src/gui/twinkle.pro
	lrelease -compress $(srcdir)/twinkle_ru.ts -qm $@

twinkle_sv.qm: $(top_builddir)/src/gui/twinkle $(top_srcdir)/src/gui/twinkle.pro
	lrelease -compress $(srcdir)/twinkle_sv.ts -qm $@
