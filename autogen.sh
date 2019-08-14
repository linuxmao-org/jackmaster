#!/bin/sh
aclocal -I /usr/local/share/aclocal && libtoolize --automake && automake -a && autoconf && \
echo "Great. Now type ' ./configure && make '."
