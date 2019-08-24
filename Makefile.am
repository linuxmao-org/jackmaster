## Process this file with automake to produce Makefile.in

SUBDIRS = src

EXTRA_DIST = \
	autogen.sh \
	config.h \
	jackmaster16x16.xpm \
	jackmaster.desktop

icondir = $(datadir)/icons
icon_DATA = jackmaster16x16.xpm

desktopdir = $(datadir)/applications
desktop_DATA = jackmaster.desktop

devdistdir: distdir
	-chmod -R a+r $(distdir)
	pushd .; cd $(distdir); \
	  rm -f aclocal.m4 libtool ltmain.sh install-sh depcomp missing mkinstalldirs; \
	  rm -f config.cache config.guess config.log config.status config.sub configure compile; \
	  rm -f Makefile.in src/Makefile.in; \
	  echo "#!/bin/sh" > autogen.sh; \
	  echo "aclocal && libtoolize --automake && automake -a && autoconf && \\" >> autogen.sh; \
	  echo "echo \"Great. Now type ' ./configure && make '.\"" >> autogen.sh; \
	  chmod a+x autogen.sh; popd

devdist_tgz: devdistdir
	$(AMTAR) chof - $(distdir) | GZIP=$(GZIP_ENV) gzip -c > dev_$(distdir).tar.gz
	-rm -rf $(distdir)

devdist_bz2: devdistdir
	find $(distdir) | cpio -o -H newc | bzip2 $(GZIP_ENV) -c > dev_$(distdir).cpio.bz2
	-rm -rf $(distdir)

devdist_7z: devdistdir
	find $(distdir) | cpio -o -H newc | 7z a -mx=7 -si dev_$(distdir).cpio.7z
	-rm -rf $(distdir)

dist_bz2: distdir
	find $(distdir) | cpio -o -H newc | bzip2 $(GZIP_ENV) -c > $(distdir).cpio.bz2
	-rm -rf $(distdir)

dist_7z: distdir
	find $(distdir) | cpio -o -H newc | 7z a -mx=7 -si $(distdir).cpio.7z
	-rm -rf $(distdir)

#devdist: devdist_tgz
#devdist: devdist_bz2
devdist: devdist_7z
