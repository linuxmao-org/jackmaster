## Process this file with automake to produce Makefile.in

EXTRA_DIST = jm_if_t.h jack_process.c jack_process.h  testloop.c

AM_CPPFLAGS = $(GTK_CFLAGS) $(JACK_CFLAGS) $(LASH_CFLAGS) $(ALSA_CFLAGS)

jackmaster-jack_engine.o: jack_engine.c
	if $(COMPILE) $(MAX_OPT_CFLAGS) -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
	then mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi
##	$(CC) $(DEFS) $(DEFAULT_INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) $(GTK_CFLAGS) $(JACK_CFLAGS) $(MAX_OPT_CFLAGS) -c $<

bin_PROGRAMS = jackmaster
jackmaster_SOURCES = \
	main.c main.h \
	configfile.c configfile.h \
	jack_if.c jack_if.h jack_engine.c jack_engine.h \
	alsa_if.c alsa_if.h \
	gui.c gui.h \
	gtkhelper.c gtkhelper.h \
	gtkmeter.c gtkmeter.h
##	gtkmeterscale.c gtkmeterscale.h
#if ALSA
#jackmaster_SOURCES += alsa_if.c alsa_if.h
#endif
jackmaster_DEPENDENCIES = jm_if_t.h jack_process.c jack_process.h
jackmaster_LDADD = $(GTK_LIBS) $(JACK_LIBS) $(LASH_LIBS) $(ALSA_LIBS)
jackmaster_CFLAGS = $(AM_CFLAGS)


jackmaster_la-jack_engine.lo: jack_engine.c
	if ../libtool --mode=compile $(COMPILE) $(MAX_OPT_CFLAGS) -MT $@ -MD -MP -MF "$(DEPDIR)/$*.Tpo" \
	  -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<; \
	then mv -f "$(DEPDIR)/$*.Tpo" "$(DEPDIR)/$*.Po"; \
	else rm -f "$(DEPDIR)/$*.Tpo"; exit 1; \
	fi

jacklibdir = $(DESTDIR)$(prefix)/lib/jack
jacklib_LTLIBRARIES = jackmaster.la
jackmaster_la_SOURCES = jack_inserver.c jack_engine.c jack_engine.h
#if ALSA
#jackmaster_la_SOURCES += alsa_if.c alsa_if.h
#endif
jackmaster_la_DEPENDENCIES = jm_if_t.h jack_process.c jack_process.h
jackmaster_la_LDFLAGS = -module -avoid-version
jackmaster_la_LIBADD = $(JACK_LIBS)
jackmaster_la_CFLAGS = $(AM_CFLAGS)

